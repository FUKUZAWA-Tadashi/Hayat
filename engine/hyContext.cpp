/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


#include "hyContext.h"
#include "hyMemPool.h"
#include "hyEndian.h"
#include "hyVM.h"
#include "hyVarTable.h"
#include "hyBytecode.h"
#include "hyException.h"
#include "hyStringBuffer.h"
#include "hyFiber.h"
#include "hyCodeManager.h"
#include "hySymbolTable.h"
#include "hyGC.h"
#include "hyDebug.h"


using namespace Hayat::Common;
using namespace Hayat::Engine;

bool Hayat::Engine::isJumpControlLabel(SymbolID_t label)
{
    return (label >= HSymR_jumpControlStart && label <= HSymR_finally);
}

void* Context::operator new(size_t size)
{
    if(size != sizeof(Context))
        HMD_PRINTF("size %d, %d\n", size, sizeof(Context));

    HMD_DEBUG_ASSERT(size == sizeof(Context));
    Object* obj = Object::create(HC_Context, sizeof(Context) + sizeof(hyu32));
    Context* self = obj->cppObj<Context>(HSym_Context);
    return (void*)self;
}

void Context::operator delete(void* ptr)
{
    Object::fromCppObj(ptr)->destroy();
}


Context::Context(void)
    : stack(),
      codePtr(NULL),
      frameStack(),
      curFrame(NULL),
      thread(NULL),
      fiber(NULL)
{
    canSubstConst = false;
}

Context::~Context()
{
    finalize();
}

Context* Context::create(void)
{
    Object* obj = Object::create(HC_Context, sizeof(Context) + sizeof(hyu32));
    Context* self = obj->cppObj<Context>(HSym_Context);
    self->initialize();
    return self;
}

void Context::destroy(void)
{
    finalize();
    getObj()->destroy();
}


Context* Context::initialize(hyu32 stackSize, hyu32 frameStackSize)
{
    stack.initialize(stackSize);
    frameStack.initialize(frameStackSize);
    curFrame = NULL;
    codePtr.initialize(NULL);
    canSubstConst = false;
    thread = NULL;
    fiber = NULL;
    return this;
}

void Context::finalize(void)
{
    //HMD_PRINTF("Context %x finalize\n",this);
    stack.finalize();
    while (frameStack.size() > 0) {
        Frame_st& fs = frameStack.getTop();
        if (! fs.bFfi)
            JumpControlList::deleteAll(fs.r.jumpControls);
        frameStack.drop(1);
    }
    frameStack.finalize();
    curFrame = NULL;
    codePtr.finalize();
    if (thread != NULL)
        thread->detachContext(this);
    thread = NULL;
    canSubstConst = false;
}

void Context::cleanup(void)
{
    stack.clean();
    frameStack.clean();
    curFrame = NULL;
    codePtr.jump(NULL);
    canSubstConst = false;  // 定数代入不可
}


hyu8 Context::getCodeHYU8(void)
{
    hyu8 i = *codePtr;
    codePtr.advance(1);
    return i;
}

hys8 Context::getCodeHYS8(void)
{
    hys8 i = (hys8) *codePtr;
    codePtr.advance(1);
    return i;
}

hyu16 Context::getCodeHYU16(void)
{
    hyu16 i = Endian::unpack<hyu16>(codePtr.addr());
    codePtr.advance(2);
    return i;
}

hys16 Context::getCodeHYS16(void)
{
    hys16 i = Endian::unpack<hys16>(codePtr.addr());
    codePtr.advance(2);
    return i;
}

hyu32 Context::getCodeHYU32(void)
{
    hyu32 i = Endian::unpack<hyu32>(codePtr.addr());
    codePtr.advance(4);
    return i;
}

hys32 Context::getCodeHYS32(void)
{
    hyu32 i = Endian::unpack<hys32>(codePtr.addr());
    codePtr.advance(4);
    return i;
}

hyf32 Context::getCodeHYF32(void)
{
    hyf32 i = Endian::unpack<hyf32>(codePtr.addr());
    codePtr.advance(4);
    return i;
}

SymbolID_t Context::getCodeSymbolID(void)
{
#if defined(SYMBOL_ID_IS_HYU16)
    return (SymbolID_t)getCodeHYU16();
#elif defined(SYMBOL_ID_IS_HYU32)
    return (SymbolID_t)getCodeHYU32();
#else
#error symbol id size not defined
#endif  
}






void Context::m_throwMethodNotFoundException(const Value& self, SymbolID_t methodSym)
{
    if (self.getType()->symCheck(HSym_Class)) {
        throwException(HSym_method_not_found,
                       M_M("class method not found or signature mismatch: %s::%s"),
                       ((HClass*)self.ptrData)->name(),
                       gSymbolTable.id2str(methodSym));
    } else {
        throwException(HSym_method_not_found,
                       M_M("instance method not found or signature mismatch: %s::%s"),
                       self.getType()->name(),
                       gSymbolTable.id2str(methodSym));
    }
}


// インスタンスメソッド呼び出し
// スタックtopがインスタンス
// 失敗すると例外を投げる
// [instance argn .. arg1] -> [retval]
void Context::methodCall(SymbolID_t methodSymbol, Signature* pSig)
{
    if (m_methodCall(methodSymbol, pSig))
        return;

    Value& instance = stack.getTop();
    m_throwMethodNotFoundException(instance, methodSymbol);
}
bool Context::methodCall_ifExist(SymbolID_t methodSymbol, int numArgs)
{
    Signature sig((hyu8)numArgs, this);
    sig.setNumOnStack(numArgs + 1);
    return m_methodCall(methodSymbol, &sig);
}

// インスタンスメソッド呼び出し
// スタックtopが引数の最後の値
// 失敗すると例外を投げる
// [argn .. arg1 instance] -> [retval]
void Context::methodCallR(SymbolID_t methodSymbol, Signature* pSig)
{
    if (m_methodCallR(methodSymbol, pSig)) {
        return;
    }

    Value& instance = stack.getNth(pSig->getNumOnStack());
    m_throwMethodNotFoundException(instance, methodSymbol);
}

bool Context::methodCallR_ifExist(SymbolID_t methodSymbol, int numArgs)
{
    Signature sig((hyu8)numArgs, this);
    sig.setNumOnStack(numArgs);
    return m_methodCallR(methodSymbol, &sig);
}

// インスタンスメソッド呼び出し
// スタックtopが引数の最後の値
// 失敗すると例外を投げる
// [argn .. arg1] -> [retval]
void Context::methodCall(const Value& instance, SymbolID_t methodSymbol, Signature* pSig)
{
    if (m_methodCall(instance, methodSymbol, pSig)) {
        return;
    }

    m_throwMethodNotFoundException(instance, methodSymbol);
}

bool Context::methodCall_ifExist(const Value& instance, SymbolID_t methodSymbol, int numArgs)
{
    Signature sig((hyu8)numArgs, this);
    sig.setNumOnStack(numArgs);
    return m_methodCall(instance, methodSymbol, &sig);
}


    
// 暗黙self、もしくはトップレベルメソッド呼び出し
// [argn .. arg1] -> [retval]
void Context::sMethodCall(SymbolID_t methodSymbol, Signature* pSig)
{
    // selfに対するメソッドがあれば呼び出す
    if (m_methodCall(curFrame->self, methodSymbol, pSig))
        return;

    // トップレベルメソッドがあれば呼び出す
    if (m_topMethodCall(methodSymbol, pSig))
        return;

    m_throwMethodNotFoundException(curFrame->self, methodSymbol);
}
    
// 指定クラスのメソッド呼び出し
// 暗黙self、もしくはクラスメソッド、もしくは組み込み関数を呼び出す
// [argn .. arg1] -> [retval]
void Context::scopeMethodCall(SymbolID_t methodSymbol, const HClass* pClass, Signature* pSig)
{
    if (m_scopeMethodCall(methodSymbol, pClass, pSig))
        return;

    throwException(HSym_method_not_found,
                   M_M("no method %s found in scope %s.  self.type=%s"),
                   gSymbolTable.id2str(methodSymbol),
                   pClass->name(),
                   curFrame->self.getTypeName());
}
    
void Context::callBytecodeTop(const Bytecode* pBytecode)
{
    const HClass* klass = pBytecode->mainClass();
    Value self(HC_Class, (void*)klass);
    MethodPtr mptr(self,
                   klass->classInitializer(),
                   0);
    gCodeManager.setWorkingBytecode(pBytecode);
    routineCall(mptr, 0);
}

bool Context::m_doExec(void)
{
    gMemPool->disableAbsorbAlert();
    throwFlag = false;
    hyu32 frameSP = frameStack.size();
    while ((frameStack.size() >= frameSP) && VM::haveCode()) {
        VM::exec1step();
        HMD_ASSERT((thread == NULL) || (thread->state() == Thread::RUNNING));
        if (throwFlag) {
            throwFlag = false;
            if (frameStack.size() < frameSP) {
                // exec中にcatchされずに呼び出し元まで行ってしまった
                gMemPool->enableAbsorbAlert();
                return false;
            }
        }
    }
    gMemPool->enableAbsorbAlert();
    return true;
}

bool Context::execBytecodeTop(const Bytecode* pBytecode)
{
    canSubstConst = true;
    VM::setContext(this);
    callBytecodeTop(pBytecode);
    bool result = m_doExec();
    canSubstConst = false;
    return result;
}

// インスタンスメソッド実行
// スタックtopがインスタンス
// 失敗すると例外を投げる
// [instance argn .. arg1] -> [retval]
bool Context::execMethod(SymbolID_t methodSymbol, Signature* pSig)
{
    VM::setContext(this);
    if (m_methodCall(methodSymbol, pSig)) {
        return m_doExec();
    }

    Value& instance = stack.getTop();
    m_throwMethodNotFoundException(instance, methodSymbol);
    return false;
}
// インスタンスメソッド実行
// スタックtopが引数の最後の値
// 失敗すると例外を投げる
// [argn .. arg1 instance] -> [retval]
bool Context::execMethodR(SymbolID_t methodSymbol, Signature* pSig)
{
    Value& instance = stack.getNth(pSig->getNumOnStack());
    VM::setContext(this);
    if (m_methodCall(instance, methodSymbol, pSig)) {
        if (m_doExec()) {
            stack.removeNth(2); // instanceを削除
            return true;
        }
        return false;
    }

    m_throwMethodNotFoundException(instance, methodSymbol);
    return false;
}
bool Context::execMethod(const Value& instance, SymbolID_t methodSymbol, Signature* pSig)
{
    VM::setContext(this);
    if (m_methodCall(instance, methodSymbol, pSig)) {
        return m_doExec();
    }

    m_throwMethodNotFoundException(instance, methodSymbol);
    return false;
}

int Context::execMethod_ifExist(const Value& instance, SymbolID_t methodSymbol, Signature* pSig)
{
    VM::setContext(this);
    if (! m_methodCall(instance, methodSymbol, pSig)) {
        return 0;
    }
    return m_doExec() ? 1 : -1;
}



// メソッド呼び出し
// [instance argn .. arg1] -> [retval]
// instanceがクラスならばClassクラスのインスタンスメソッド呼び出し、メソッドが
// 見つからなければinstanceのクラスのクラスメソッド呼び出し。
// instanceがクラスでなければinstanceのクラスのインスタンスメソッド呼び出し。
// 呼び出しに成功したらtrueを返す
// 失敗したらスタックはそのまま
bool Context::m_methodCall(SymbolID_t methodSymbol, Signature* pSig)
{
    Value& ins = stack.getTop(); // ここではpopしない
    MethodPtr methodPtr(ins);
    if (! m_buildMethodPtr(methodPtr, methodSymbol, pSig))
        return false;
    pop();                      // insをpop
    m_doMethodCall(methodPtr, methodSymbol, pSig);
    return true;
}

// メソッド呼び出し2
// [argn .. arg1 instance] -> [retval]
// instanceがクラスならばClassクラスのインスタンスメソッド呼び出し、メソッドが
// 見つからなければinstanceのクラスのクラスメソッド呼び出し。
// instanceがクラスでなければinstanceのクラスのインスタンスメソッド呼び出し。
// 呼び出しに成功したらtrueを返す
// 失敗したらスタックはそのまま
bool Context::m_methodCallR(SymbolID_t methodSymbol, Signature* pSig)
{
    Value& ins = stack.getNth(pSig->getNumOnStack()+1); // ここではinsは残しておく
    MethodPtr methodPtr(ins);
    if (! m_buildMethodPtr(methodPtr, methodSymbol, pSig))
        return false;
    stack.removeNth(pSig->getNumOnStack()+1); // insを削除
    pSig->adjustStack(-1);
    m_doMethodCall(methodPtr, methodSymbol, pSig);
    return true;
}


// メソッド呼び出し
// [argn .. arg1] -> [retval]
// selfがクラスならばClassクラスのインスタンスメソッド呼び出し、メソッドが
// 見つからなければselfのクラスのクラスメソッド呼び出し。
// selfがクラスでなければselfのクラスのインスタンスメソッド呼び出し。
// 呼び出しに成功したらtrueを返す
// 失敗したらスタックはそのまま
bool Context::m_methodCall(const Value& self, SymbolID_t methodSymbol, Signature* pSig)
{
    MethodPtr methodPtr(self);
    if (! m_buildMethodPtr(methodPtr, methodSymbol, pSig))
        return false;
    m_doMethodCall(methodPtr, methodSymbol, pSig);
    return true;
}

bool Context::m_buildMethodPtr(MethodPtr& methodPtr, SymbolID_t methodSymbol, Signature* pSig)
{
    const Value& self = methodPtr.getSelf();
    const HClass* selfType = self.getType();
    if (selfType->symCheck(HSym_Class)) {
        if (! HC_Class->searchMethod(&methodPtr, methodSymbol, pSig)) {
            const HClass* pClass = (const HClass*) self.ptrData;
            if (! pClass->searchMethod(&methodPtr, methodSymbol, pSig))
                return false;
            //else HMD__PRINTF_FK("class method %s:%s\n", gSymbolTable.id2str(pClass->getSymbol()), gSymbolTable.id2str(methodSymbol));
        }
        //else HMD__PRINTF_FK("instance method Class:%s\n", gSymbolTable.id2str(methodSymbol));
    } else {
        if (! selfType->searchMethod(&methodPtr, methodSymbol, pSig))
            return false;
        //else HMD__PRINTF_FK("instance method %s:%s\n", gSymbolTable.id2str(selfType->getSymbol()), gSymbolTable.id2str(methodSymbol));
    }
    return true;
}


void Context::m_doMethodCall(MethodPtr& methodPtr, SymbolID_t methodSymbol, Signature* pSig)
{
    hyu8 numArgs = pSig->getNumOnStack();
    if (methodPtr.isFfi()) {
        FfiCall(methodPtr, numArgs, methodSymbol);
    } else {
        routineCall(methodPtr, numArgs);
    }
}
    
bool Context::m_topMethodCall(SymbolID_t methodSymbol, Signature* pSig)
{
    // バイトコードトップレベルメソッドがあれば呼び出す
    MethodPtr methodPtr;
    const Bytecode* curBytecode = gCodeManager.getWorkingBytecode();
    if (curBytecode == NULL)
        return false;
    if (! curBytecode->searchMethod(&methodPtr, methodSymbol, pSig))
        return false;

    /*
    HClass* mainClass = codePtr.mainClass();
    Value d(HC_Class, mainClass);
    MethodPtr methodPtr(d);
    if (! mainClass->searchMethod(&methodPtr, methodSymbol))
        return false;
    */

    m_doMethodCall(methodPtr, methodSymbol, pSig);
    return true;
}

// 指定クラスのメソッド呼び出し
bool Context::m_scopeMethodCall(SymbolID_t methodSymbol, const HClass* pClass, Signature* pSig)
{
    Value& self = curFrame->self;
    MethodPtr methodPtr(self);
    if (! ((HClass*)pClass)->searchMethod(&methodPtr, methodSymbol, pSig))
        return false;

    m_doMethodCall(methodPtr, methodSymbol, pSig);
    return true;
}

    
// 共有も考慮したローカル変数アドレスを取得
Value* Context::m_getLocalVarAddr(hyu32 pos)
{
    Value* varAddr = stack.addrAt(pos);
    if (varAddr->type == HC_INDIRECT_REF) {
        // 共有されている
        Object* ind = varAddr->objPtr;
        HMD_DEBUG_ASSERT(ind->type() == HC_INDIRECT_ENT);
        varAddr = (Value*) ind->field();
    }
    return varAddr;
}


Value Context::getLocalVar(int varIdx)
{
    //HMD__PRINTF_FK("getLocalVar: framepos=%d, varIdx=%d, numArgs=%d, numLocals=%d\n",curFrame->framePos, varIdx, curFrame->numArgs, curFrame->numLocals);
    HMD_ASSERT(! curFrame->bFfi);
    HMD_ASSERT(varIdx >= 0);
    HMD_ASSERT(varIdx < curFrame->numArgs + curFrame->r.numLocals);
    Value* varAddr = m_getLocalVarAddr(curFrame->framePos + varIdx);
    return *varAddr;
}
    
void Context::setLocalVar(int varIdx, const Value& val)
{
    //HMD__PRINTF_FK("setLocalVar framepos=%d, varIdx=%d, numArgs=%d, numLocals=%d\n",curFrame->framePos,varIdx, curFrame->numArgs, curFrame->numLocals);
    HMD_ASSERT(! curFrame->bFfi);
    HMD_ASSERT(varIdx >= 0);
    HMD_ASSERT(varIdx < curFrame->numArgs + curFrame->r.numLocals);
    Value* varAddr = m_getLocalVarAddr(curFrame->framePos + varIdx);
    *varAddr = val;
    GC::writeBarrier(val);
}
    
Object* Context::shareLocalVar(int outVarIdx)
{
    //HMD__PRINTF_FK("shareLocalVar outVarIdx=%d\n",outVarIdx);
    Context::Frame_st& fs = frameStack.getTop();
    HMD_ASSERT(! fs.bFfi);
    Value* outVar = stack.addrAt(fs.framePos + outVarIdx);
    Object* ind;
    if (outVar->type == HC_INDIRECT_REF) {
        // 既に共有されている
        return outVar->objPtr;
    }
    ind = Object::create(HC_INDIRECT_ENT, sizeof(Value) + sizeof(hyu32));
    *((Value*)ind->field()) = *outVar;
    GC::copyMarkFlag(ind, *outVar);
    outVar->type = HC_INDIRECT_REF;
    outVar->objPtr = ind;
    return ind;
}

Value Context::getMembVar(SymbolID_t varSym)
{
    if (curFrame->self.type != HC_REF) {
        throwException(HSym_use_member_var_out_of_instance,
                       M_M("using member variable '%s' out of instance"),
                       gSymbolTable.id2str(varSym));
    }
    Object* obj = curFrame->self.objPtr;
    //HMD__PRINTF_FK("getInsVar: class=%s, var=%s, ",obj->type()->name(),gSymbolTable.id2str(varSym));
    const HClass* pClass = obj->type();
    Value* addr = pClass->getMembVarAddr(obj, varSym);
    HMD_ASSERTMSG(addr != NULL, M_M("unknown member var %s (class %s)"), gSymbolTable.id2str(varSym), gSymbolTable.id2str(pClass->getSymbol()));
    return *addr;
}
    
void Context::setMembVar(SymbolID_t varSym, const Value& val)
{
    if (curFrame->self.type != HC_REF) {
        throwException(HSym_use_member_var_out_of_instance,
                       M_M("using member variable '%s' out of instance"),
                       gSymbolTable.id2str(varSym));
    }
    Object* obj = curFrame->self.objPtr;
    //HMD__PRINTF_FK("setInsVar: class=%s, var=%s\n",obj->type()->name(),gSymbolTable.id2str(varSym));
    const HClass* pClass = obj->type();
    Value* addr = pClass->getMembVarAddr(obj, varSym);
    HMD_ASSERTMSG(addr != NULL, M_M("unknown member var %s (class %s)"), gSymbolTable.id2str(varSym), gSymbolTable.id2str(pClass->getSymbol()));
    *addr = val;
    GC::writeBarrier(val);
}
    
Value Context::getClassVar(SymbolID_t varSym)
{
    const HClass* pClass = m_getSelfClass();
    //HMD__PRINTF_FK("getClassVar class=%s, var=%s\n",gSymbolTable.id2str(pClass->getSymbol()),gSymbolTable.id2str(varSym));
    Value* addr = pClass->getClassVarAddr(varSym);
    //HMD__PRINTF_FK("addr=%x\n",addr);
    HMD_ASSERT(addr != NULL);
    return *addr;
}
    
void Context::setClassVar(SymbolID_t varSym, const Value& val)
{
    const HClass* pClass = m_getSelfClass();
    //HMD__PRINTF_FK("setClassVar class=%s, var=%s\n",gSymbolTable.id2str(pClass->getSymbol()),gSymbolTable.id2str(varSym));
    Value* addr = pClass->getClassVarAddr(varSym);
    //HMD__PRINTF_FK("addr=%x\n",addr);
    HMD_ASSERT(addr != NULL);
    *addr = val;
    GC::writeBarrier(val);
}
    
Value Context::getGlobalVar(SymbolID_t varSym)
{
    //HMD__PRINTF_FK("getGlobalVar var=%s\n",gSymbolTable.id2str(varSym));
    Value* addr = gGlobalVar.getVarAddr(varSym);
    //HMD__PRINTF_FK("addr=%x\n",addr);
    HMD_ASSERT(addr != NULL);
    return *addr;
}
    
void Context::setGlobalVar(SymbolID_t varSym, const Value& val)
{
    //HMD__PRINTF_FK("setGlobalVar var=%s\n",gSymbolTable.id2str(varSym));
    Value* addr = gGlobalVar.getVarAddr(varSym);
    //HMD__PRINTF_FK("addr=%x\n",addr);
    HMD_ASSERT(addr != NULL);
    *addr = val;
    GC::writeBarrier(val);
}
    
Value Context::getConstVar(SymbolID_t varSym)
{
    const HClass* pClass = lexicalScope();
    Value* addr = pClass->getConstVarAddr(varSym);
    if (addr != NULL) return *addr;
    throwException(HSym_cannot_access_constant,
                   M_M("cannot access constant '%s' from class '%s'"),
                   gSymbolTable.id2str(varSym), pClass->name());
    return NIL_VALUE;
}
    
void Context::setConstVar(SymbolID_t varSym, const Value& val)
{
    if (!canSubstConst) {
        throwError(M_M("substitution to Constant '%s' not allowed"),
                   gSymbolTable.id2str(varSym));
    }
    const HClass* pClass = lexicalScope();
    //HMD__PRINTF_FK("setConstVar class=%s, var=%s\n",gSymbolTable.id2str(pClass->getSymbol()),gSymbolTable.id2str(varSym));
    Value* addr = pClass->getConstVarAddr(varSym);
    //HMD__PRINTF_FK("addr=%x\n",addr);
    if (addr == NULL) {
        throwException(HSym_cannot_access_constant,
                       M_M("cannot access constant '%s' from class '%s'"),
                       gSymbolTable.id2str(varSym), pClass->name());
        return;
    }
    *addr = val;
    GC::writeBarrier(val);
    //HMD__PRINTF_FK(" <- %d\n",val.data);
}



// 例外throw
void Context::throwException(SymbolID_t type, Value exVal)
{
    Object* exObj;
    Exception* e;
    if (exVal.getType()->symCheck(HSym_Exception)) {
        HMD_DEBUG_ASSERT(exVal.type == HC_REF);// Objectにくるまれていなければならない
        exObj = exVal.objPtr;
        e = exObj->cppObj<Exception>();
        e->setType(type);
    } else {
        // Exceptionにはメンバ変数もスーパークラスも無いので簡易createInstanceを使う
        exObj = (Object*) HC_Exception->createInstance_noInit(sizeof(Exception));
        e = exObj->cppObj<Exception>();
        e->initialize(type, exVal);
    }
    throwException(e);
}

void Context::throwJumpCtlException(SymbolID_t label, Value val)
{
    Object* exObj = (Object*) HC_Exception->createInstance_noInit(sizeof(Exception));
    Exception* e = exObj->cppObj<Exception>();
    if (isJumpControlLabel(label))
        e->initialize(HSymR_jump_control, val);
    else
        e->initialize(HSym_goto, val);
    e->setLabel(label);
    throwException(e);
}

void Context::throwGotoException(SymbolID_t label, Value val)
{
    HMD_DEBUG_ASSERT(! isJumpControlLabel(label));
    Object* exObj = (Object*) HC_Exception->createInstance_noInit(sizeof(Exception));
    Exception* e = exObj->cppObj<Exception>();
    e->initialize(HSym_goto, val);
    e->setLabel(label);
    throwException(e);
}


// e は Objectにくるまれたものでなければならない
void Context::throwException(Exception* e)
{
#if 0
    HMD_PRINTF("throwException(%x) ",e);
    e->debugPrint(this);
    HMD_PRINTF(" at ");
    debugPrintCurPos();
#endif
    throwFlag = true;
    {
        Signature* sig = VM::getMultiSubstSignature();
        if (sig != NULL) {
            delete sig;
            VM::setMultiSubstSignature(NULL);
        }
    }
    for (;;) {
        //HMD_PRINTF("thrown: context=%x, sp=%d, framepos=%d, numargs=%d\n",this,stack.size(),curFrame->framePos,curFrame->numArgs);

        if (! curFrame->bFfi) {
            for ( ; curFrame->r.jumpControls != NULL; JumpControlList::drop(&(curFrame->r.jumpControls))) {

                JumpControlInfo_st& inf = curFrame->r.jumpControls->head();
                if (inf.tableNo == EXCEPTION_TO_FIBER) {
#if 0
                    HMD_PRINTF("Fiber::stakeCall at ");
                    debugPrintCurPos();
                    HMD_PRINTF("\n");
#endif

                    // Fiber::stakeCall()の実行位置

                    e->addBackTrace(codePtr, ownerClass()); // バックトレース追加
                    codePtr = curFrame->returnCodePtr;
                    Fiber* iFiber = inf.fiber;

                    // closureの実行を終了して、
                    JumpControlList::drop(&(curFrame->r.jumpControls));
                    frameStack.pop();
                    curFrame = frameStack.topAddr();
                    if (curFrame->bFfi) {
                        stack.dropTo(curFrame->framePos);
                    } else {
                        stack.dropTo(curFrame->framePos + curFrame->r.numLocals + curFrame->numArgs);
                    }

                    // 例外をfiberに投げる
                    iFiber->thrown(this, e);
                    return;
                }


                if (e->type() == HSymR_jump_control || e->type() == HSym_goto) {
                    // break, next, goto
                    SymbolID_t label = e->getLabel();
                    const hyu8* labelAddr = getJumpAddr(label);
                    if (labelAddr != NULL) {
                        codePtr.jump(labelAddr);
#if 0
                        {
                            char buf[256];

                            ownerClass()->bytecode()->debugGetInfo(buf, 256, labelAddr);
                            HMD__PRINTF_FK("jumpControl: label=%s jump to: %x:%s\n",
                                           gSymbolTable.id2str(label), labelAddr, buf);
                        }
#endif
                        push(e->val());
                        delete e; // jump_control例外オブジェクトは共有されないので、ここでdelete
                        //HMD__PRINTF_FK("jumpControl end: context=%x, sp=%d, framepos=%d, numargs=%d, numlocal=%d\n",this,stack.size(),curFrame->framePos,curFrame->numArgs,curFrame->numLocals);
                        return;
                    }

#if 0
                    {
                        char buf[256];
                        ownerClass()->bytecode()->debugGetInfo(buf, 256, inf.startAddr);
                        HMD_PRINTF("go out jump control: %s\n  start = %s\n  catchVar=%d, tableNo=%d,stackLv=%d\n",gSymbolTable.id2str(label), buf,inf.catchVar, inf.tableNo, inf.stackLv);
                    }
#endif
                    if (e->type() == HSymR_jump_control)
                        goto finally; // break,next はcatchされない
                }

#if 0
                {
                    char buf[256];
                    ownerClass()->bytecode()->debugGetInfo(buf, 256, codePtr.addr());
                    HMD_PRINTF("add backtrace %s\n", buf);
                }
#endif
                // catch 処理
                if (! e->isError()) {
                    const hyu8* catchAddr = getJumpAddr(HSymR_catch);
                    // catch,finally節での例外をここでcatchすると無限ループ
                    // try節より後にあるという前提でアドレスで判定
                    if (catchAddr != NULL && catchAddr >= codePtr.addr()) {
                        // catch
                        e->addBackTrace(codePtr, ownerClass()); // バックトレース追加
                        if (inf.catchVar != NO_CATCH_VAR) {
                            Object* exObj = Object::fromCppObj(e);
                            setLocalVar(inf.catchVar, Value::fromObj(exObj));
                        }
                        HMD_ASSERT(stack.size() >= inf.stackLv);
                        stack.dropTo(inf.stackLv);
                        codePtr.jump(catchAddr);
#if 0
                        HMD__PRINTF_FK("catched: sp=%d, framepos=%d, numargs=%d, numlocal=%d\nat ",stack.size(),curFrame->framePos,curFrame->numArgs,curFrame->r.numLocals);
                        debugPrintCurPos();
                        debugPrintStack();
#endif
                        return;
                    }
                }

            finally:
                // finally処理
                if (! isInFinally(inf)) {
                    const hyu8* finallyAddr = getJumpAddr(HSymR_finally, inf);
                    if (finallyAddr != NULL) {
                        e->addBackTrace(codePtr, ownerClass()); // バックトレース追加
                        Object* exObj = Object::fromCppObj(e);
                        setLocalVar(inf.finallyValVar, Value::fromObj(exObj));
                        inf.finallyExitType = FINALLY_EXIT_TYPE_EXCEPTION;
                        stack.dropTo(inf.stackLv);
                        codePtr.jump(finallyAddr);
                        return;
                    }
                }

            }

        }
        else {
            // in FFI
            if (curFrame->f.exceptionCatcher != NULL) {
                bool b = curFrame->f.exceptionCatcher(this, e);
                setFfiExceptionCatcher(NULL);

                if (b) return;
            }
        }



        // catch,finallyの外なのでコールスタックを遡る
        e->addBackTrace(codePtr, ownerClass()); // バックトレース追加
        codePtr = curFrame->returnCodePtr;
#if 0
        HMD_PRINTF("exception: stack back ");
        debugPrintCurPos();
        HMD_PRINTF(" fiber = %x\n", fiber);
#endif
        if (frameStack.size() > 1) {
            Frame_st& fs = frameStack.getTop();
            if (! fs.bFfi)
                JumpControlList::deleteAll(fs.r.jumpControls);
            frameStack.pop();
            curFrame = frameStack.topAddr();
#if 0
            //e->debugPrint(this);
            debugPrintStack();
#endif
            if (curFrame->bFfi) {
                stack.dropTo(curFrame->framePos);
            } else {
                stack.dropTo(curFrame->framePos + curFrame->r.numLocals + curFrame->numArgs);
            }
        } else {
            // トップまで戻った
            HMD_ASSERT(! codePtr.haveCode());
            if (fiber != NULL) {
                // Fiberだったら、Fiberを作ったcontextに例外を投げる
                //HMD__PRINTF_FK("fiberThrow from context %x\n", this);
                fiber->fiberThrow(e);
                return;
            } else {
#ifdef HMD_DEBUG
#ifndef HY_WORK_WITHOUT_VM
                Hayat::Engine::VM::debugStackTrace();
#endif
                e->debugPrint(this);
                HMD_HALT();
#else
#ifdef HY_ENABLE_BYTECODE_RELOAD
                Hayat::Engine::VM::printOpcodeHistory();
#endif
                HMD_FATAL_ERROR(M_M("uncaught exception"));
#endif
            }
        }
    }
}


void Context::jumpControlStart(void)
{
    HMD_ASSERT(! curFrame->bFfi);
    JumpControlList* newControl = new JumpControlList();
    ((MemCell*)newControl)->setMemID("JCLS");
    JumpControlInfo_st& inf = newControl->content();
    inf.catchVar = getCodeHYS16(); // codePtrが動くので順番に注意
    inf.finallyValVar = getCodeHYS16(); // codePtrが動くので順番に注意
    inf.finallyExitType = FINALLY_EXIT_TYPE_NONE;
    inf.tableNo = getCodeHYU8();   // codePtrが動くので順番に注意
    inf.stackLv = stack.size();
    inf.startAddr = codePtr.addr();
    newControl->link(curFrame->r.jumpControls);
    curFrame->r.jumpControls = newControl;
}

void Context::jumpControlEnd(void)
{
    // 現在のJumpControl情報を削除
    Context::JumpControlList::drop(&(curFrame->r.jumpControls));
}

void Context::valueAfterFinally(Value& x)
{
    JumpControlInfo_st& inf = curFrame->r.jumpControls->head();
    setLocalVar(inf.finallyValVar, x);
    inf.finallyExitType = FINALLY_EXIT_TYPE_NORMAL;
}

void Context::endFinally(void)
{
    const JumpControlInfo_st& inf = curFrame->r.jumpControls->head();
    Value x = getLocalVar(inf.finallyValVar);
    switch (inf.finallyExitType) {
    case FINALLY_EXIT_TYPE_NORMAL:
        {
            const hyu8* addr = getJumpAddr(HSymR_jumpControlEnd);
            stack.dropTo(inf.stackLv);
            push(x);
            jumpControlEnd();
            codePtr.jump(addr);
        }
        break;
    case FINALLY_EXIT_TYPE_EXCEPTION:
        {
            Exception* e = x.toCppObj<Exception>(HSym_Exception);
            jumpControlEnd();
            throwException(e);
        }
        break;
    case FINALLY_EXIT_TYPE_RETURN:
        {
            hyu16 callAwayMethodID = inf.finallyCallAwayMethodID;
            stack.dropTo(inf.stackLv);
            push(x);
            jumpControlEnd();
            routineReturn(callAwayMethodID);
        }
        break;
    default:
        HMD_FATAL_ERROR(M_M("internal error: incorrect finally process"));
    }
}

const hyu8* Context::getJumpAddr(SymbolID_t jumpLabel)
{
    if (curFrame->bFfi)
        return NULL;
    if (curFrame->r.jumpControls == NULL)
        return NULL;
    const JumpControlInfo_st& inf = curFrame->r.jumpControls->head();
    return getJumpAddr(jumpLabel, inf);
}

const hyu8* Context::getJumpAddr(SymbolID_t jumpLabel, const JumpControlInfo_st& inf)
{
    if (inf.tableNo == EXCEPTION_TO_FIBER)
        return NULL;
    const hyu8* table = curFrame->r.jumpControlTableTop;
    HMD_ASSERT(inf.tableNo == 0 || inf.tableNo < *(hyu8*)(table + 2));
    const hyu8* ctl = table + *(hyu16*)(table + 4 + sizeof(hyu16) * inf.tableNo);
    int numLabel = (int) ctl[0];
    SymbolID_t* labels = (SymbolID_t*)(ctl + 4);
    int idx;
    if (binarySearch<SymbolID_t>(labels, numLabel, jumpLabel, &idx)) {
        int padding = (numLabel % 2 == 1) ? 2 : 0;
        hys32* addrs = (hys32*)(ctl + 4 + sizeof(SymbolID_t) * numLabel + padding);
        return inf.startAddr + addrs[idx];
    }
    return NULL;
}

// printf形式でメッセージを作り、throwする
void Context::throwException(SymbolID_t type, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    StringBuffer* sb = new StringBuffer();
    sb->vsprintf(msg, ap);
    va_end(ap);
    throwException(type, Value::fromObj(sb->getObj()));
}
// tryでcatchできない例外をthrow
void Context::throwError(Value exVal)
{
    Object* exobj = Object::create(HC_Exception, sizeof(Exception) + sizeof(hyu32));
    Exception* e = exobj->cppObj<Exception>();
    e->initialize(HSym_fatal_error, exVal);
    throwException(e);
}
// printf形式でメッセージを作り、エラーとしてthrowする
void Context::throwError(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    StringBuffer* sb = new StringBuffer();
    sb->vsprintf(msg, ap);
    va_end(ap);
    throwError(Value::fromObj(sb->getObj()));
}





Context* Context::fork(int stackAdjust)
{
    HMD_ASSERT(frameStack.size() > 1);
    Context* newContext = new Context();

    // 新Contextにstackを移動
    if (curFrame->bFfi) {
        stack.cutPaste(newContext->stack, stackAdjust + curFrame->numArgs);
    } else {
        stack.cutPaste(newContext->stack, stackAdjust + curFrame->numArgs + curFrame->r.numLocals);
    }

    // 新ContextにframeStackを移動
    newContext->codePtr = codePtr;
    codePtr = curFrame->returnCodePtr;
    frameStack.cutPaste(newContext->frameStack, 1);
    newContext->curFrame = newContext->frameStack.topAddr();
    newContext->curFrame->framePos = 0;
    newContext->curFrame->returnCodePtr.initialize(NULL);
    if (! newContext->curFrame->bFfi) {
        hyu32 d = stack.size();
        for (JumpControlList* p = newContext->curFrame->r.jumpControls; p != NULL ; p = p->tail()) {
            p->head().stackLv -= d;
        }
    }

    curFrame = frameStack.topAddr();

    //HMD__PRINTF_FK("fork %x -> %x\n",this,newContext);
    return newContext;
}




void Context::FfiCall(const MethodPtr& methodPtr, hyu8 numArgs, SymbolID_t methodSymbol)
{
    const Value& self = methodPtr.getSelf();
    push(self);

    frameStack.alloc_a(1);
    curFrame = frameStack.topAddr();
    curFrame->bFfi = true;
    curFrame->self = self;
    curFrame->framePos = stack.size() - numArgs - 1;
    curFrame->returnCodePtr = codePtr;
    curFrame->numArgs = numArgs;
    curFrame->f.methodSymbol = methodSymbol;
    curFrame->f.afterFfiFunc = NULL;
    curFrame->f.afterFfiParam = NULL;
    curFrame->f.exceptionCatcher = NULL;
    curFrame->f.ffiContext = this;
    HMD_ASSERT(VM::getContext() == this);

#if 0
    HMD_PRINTF("ffi call %s selfClass=%s\n",gSymbolTable.id2str(methodSymbol),
               gSymbolTable.id2str(self.getType()->getSymbol()));
    HMD_PRINTF("stack.size=%d, ffiSp=%d, ffiNumArgs=%d\n",
               stack.size(),curFrame->framePos,curFrame->numArgs);
#endif
    // FFI呼出し
    Ffi_t ffi = methodPtr.getFfi();
    ffi(this, numArgs);
}

void Context::setAfterFfiFunc(Aff_t func)
{
    HMD_ASSERT(curFrame->bFfi);
    curFrame->f.afterFfiFunc = func;
}

void Context::setAfterFfiParam(void* param)
{
    HMD_ASSERT(curFrame->bFfi);
    curFrame->f.afterFfiParam = param;
}

void* Context::getAfterFfiParam(void)
{
    HMD_ASSERT(curFrame->bFfi);
    return curFrame->f.afterFfiParam;
}


void Context::setFfiExceptionCatcher(FfiExceptionCatcher_t catcher)
{
    HMD_ASSERT(curFrame->bFfi);
    curFrame->f.exceptionCatcher = catcher;
}

bool Context::isInFfi(void)
{
    if (curFrame == NULL)
        return false;
    return curFrame->bFfi;
}

void Context::afterFfi(void)
{
    HMD_ASSERT(curFrame->bFfi);
#if 0
    HMD_PRINTF("ffi return %s\n",gSymbolTable.id2str(curFrame->f.methodSymbol));
    HMD_PRINTF("stack.size=%d, ffiSp=%d, ffiNumArgs=%d\n",
               stack.size(),curFrame->framePos,curFrame->numArgs);
    //debugPrintStack();
#endif
    if (curFrame->f.afterFfiFunc != NULL) {
        curFrame->f.afterFfiFunc(this);
    } else {
        ffiFinish();
    }
}


void Context::ffiFinish(void)
{
    // FFI呼び出し完了時の処理
    static const int numRetVal = 1;

    if (! curFrame->bFfi) {
        // FFI内から別バイトコードが呼び出された
    } else if (curFrame->f.ffiContext != VM::getContext()) {
        // コンテキストが切り替えられて継続
    } else if (thread != NULL &&
               ((thread->state() == Thread::TERMINATED) || (thread->state() == Thread::BEFORE_INIT))) {
        // FFIによりスレッド終了させられた
    } else {
        SymbolID_t klassSym;
        if (curFrame->self.getType()->symCheck(HSym_Class))
            klassSym = ((HClass*) curFrame->self.ptrData)->getSymbol();
        else
            klassSym = curFrame->self.getType()->getSymbol();
        HMD_ASSERTMSG((stack.size() - numRetVal == curFrame->framePos),
                      M_M("Bad FFI call %s::%s\nsp=%d,numRetVal=%d,newsp=%d"),
                      gSymbolTable.id2str(klassSym),
                      gSymbolTable.id2str(curFrame->f.methodSymbol),
                      curFrame->framePos,numRetVal,stack.size());
    }


    frameStack.pop();
    curFrame = frameStack.topAddr();
}

void Context::ffiKill(void)
{
    HMD__PRINTF_FK("ffi kill\n");
    HMD_DEBUG_ASSERT(curFrame->bFfi);
    HMD_DEBUG_ASSERT(curFrame->f.ffiContext == VM::getContext());
    frameStack.pop();
    curFrame = frameStack.topAddr();
}

void Context::routineCall(const MethodPtr& methodPtr, hyu8 numArgs)
{
    frameStack.alloc_a(1);
    curFrame = frameStack.topAddr();
    curFrame->bFfi = false;
    curFrame->self = methodPtr.getSelf();
    curFrame->ownerClass = curFrame->lexicalScope = methodPtr.lexicalScope();
    curFrame->framePos = stack.size() - numArgs;
    curFrame->returnCodePtr = codePtr;
    curFrame->numArgs = numArgs;
    curFrame->r.methodID = methodPtr.getMethodID();
    curFrame->r.creatorMethodID = (hyu16)-1;
    const hyu8* routine = methodPtr.addr();
    curFrame->r.jumpControlTableTop = routine;
    routine += Endian::unpack<hyu16>(routine); // jumpControlTableをスキップ
    //hyu16 sigID = Endian::unpack<hyu16>(routine);
    hyu8 numLocal = routine[4];
    routine += 8;
    curFrame->r.numLocals = numLocal;
    stack.alloc_a(numLocal);
    int top = stack.size() - 1;
    for (int i = 0; i < numLocal; i++) {
        stack.setAt(top-i, NIL_VALUE);
    }
    curFrame->r.jumpControls = new JumpControlList();
    ((MemCell*)curFrame->r.jumpControls)->setMemID("JCLR");
    JumpControlInfo_st& inf = curFrame->r.jumpControls->content();
    inf.catchVar = NO_CATCH_VAR;
    inf.tableNo = 0;
    inf.stackLv = stack.size();
    inf.startAddr = routine;
    codePtr.initialize(routine);
}

void Context::routineReturn(hyu16 callAwayMethodID)
{
    HMD_ASSERT(! curFrame->bFfi);
    HMD_ASSERTMSG(stack.size() == curFrame->framePos + curFrame->numArgs + curFrame->r.numLocals + 1,
                  M_M("return assertion failed:\n  context=%x, sp=%d, framePos=%d, numArgs=%d, nulLocals=%d"),
                  this, stack.size(), curFrame->framePos, curFrame->numArgs, curFrame->r.numLocals
                  );
    
    Value retval = pop();
    const HClass* orgscope = lexicalScope();
    Exception* ex = NULL;
    if (callAwayMethodID != (hyu16)-1) {
        ex = new Exception(HSym_fatal_error, Value::fromSymbol(HSym_closure_return));
    }

    do {
        //HMD__PRINTF_FK("return: sp=%d, framepos=%d, numargs=%d, numlocal=%d\n",stack.size(),curFrame->r.framePos,curFrame->numArgs,curFrame->r.numLocals);
        if (Debug::isPrintStack())
            debugPrintStack();


        if (! curFrame->bFfi) {
            // finally処理
            for ( ; curFrame->r.jumpControls != NULL; JumpControlList::drop(&(curFrame->r.jumpControls))) {
                JumpControlInfo_st& inf = curFrame->r.jumpControls->head();
                if (! isInFinally(inf)) {
                    const hyu8* finallyAddr = getJumpAddr(HSymR_finally, inf);
                    if (finallyAddr != NULL) {
                        setLocalVar(inf.finallyValVar, retval);
                        inf.finallyExitType = FINALLY_EXIT_TYPE_RETURN;
                        inf.finallyCallAwayMethodID = callAwayMethodID;
                        stack.dropTo(inf.stackLv);
                        codePtr.jump(finallyAddr);
                        return;
                    }
                }
            }

            stack.dropTo(curFrame->framePos);

            if (callAwayMethodID != (hyu16)-1) {
                ex->addBackTrace(codePtr, ownerClass());
                if (curFrame->r.methodID == callAwayMethodID && orgscope == lexicalScope()) {
                    callAwayMethodID = (hyu16)-1;
                }
            }
        }
        codePtr = curFrame->returnCodePtr;
        frameStack.pop();
        curFrame = frameStack.topAddr();
        if (curFrame == NULL)
            break;
    } while (callAwayMethodID != (hyu16)-1);

    if (callAwayMethodID != (hyu16)-1) {
#ifdef HMD_DEBUG
        ex->setError();
        ex->debugPrint(this);
        HMD_HALT();
#else
        HMD_FATAL_ERROR("stray return");
#endif
    }

    push(retval);

    //if (curFrame == NULL)HMD__PRINTF_FK("routineReturn: frame sp=%d, curFrame=NULL\n",frameStack.size());else HMD__PRINTF_FK("routineReturn: frame sp=%d, ret code ptr=%x, bytecode=%x\n",frameStack.size(),curFrame->returnCodePtr.addr(),curFrame->returnCodePtr.bytecode());
    if (curFrame == NULL) {
        if (fiber != NULL) {
            //HMD__PRINTF_FK("context = %x, thread = %x\n",this,thread);
            fiber->fiberReturn();
        } else {
            HMD_ASSERT(! codePtr.haveCode());
        }
    }
    //if (curFrame == NULL)HMD__PRINTF_FK("        -> : frame sp=%d, curFrame=NULL\n",frameStack.size());else HMD__PRINTF_FK("routineReturn: frame sp=%d, ret code ptr=%x, bytecode=%x\n",frameStack.size(),curFrame->returnCodePtr.addr(),curFrame->returnCodePtr.bytecode());
}

void Context::exceptionToFiber(Fiber* exfiber)
{
    HMD_ASSERT(! curFrame->bFfi);
    JumpControlInfo_st inf;
    inf.tableNo = EXCEPTION_TO_FIBER;
    inf.fiber = exfiber;
    curFrame->r.jumpControls->add(inf);
}

// 相対ジャンプ
void Context::jumpRelative(hys32 relative)
{
    codePtr += relative;
}

// numArgs個の引数を、modeに従ってconcatし、それぞれの間に m を挟み
// デバッグ表示して、引数をスタックから削除
void Context::debugPrintValues(int numArgs, int mode, const char* m)
{
    StringBuffer* sb = new StringBuffer();
    int n = numArgs;
    while (--n >= 0) {
        Value v = stack.getNth(n+1);
        sb->concat(this, v, mode);
        if (n > 0)
            sb->concat(m);
    }
    HMD_PRINTF(sb->top());
    delete sb;
    stack.drop(numArgs);
}

//======================================================================
//          for Garbage Collection
//======================================================================

void Context::m_GC_mark(void)
{
    //HMD__PRINTF_FK("mark context %x\n",this);
    Debug::incMarkTreeLevel();
    gCodeManager.usingCodeAt(codePtr.addr());

    // 呼び出しスタック
    for (hyu32 i = 0; i < frameStack.size(); i++) {
        Frame_st* fp = frameStack.addrAt(i);
        GC::markValue(fp->self);      // self
        if (! fp->bFfi) {
            for (JumpControlList::Iterator itr(&fp->r.jumpControls) ; !itr.empty(); itr.next()) {
                JumpControlInfo_st& inf = itr.get()->content();
                if (inf.tableNo == EXCEPTION_TO_FIBER) {
                    GC::markObj(Object::fromCppObj(inf.fiber));
                }
            }
            gCodeManager.usingCodeAt(fp->returnCodePtr.addr());
#ifdef HY_ENABLE_BYTECODE_RELOAD
            gCodeManager.usingCodeAt((hyu8*) fp->ownerClass->bytecode());
#endif
        }
#ifdef HY_ENABLE_BYTECODE_RELOAD
        if (GC::pGenerationMap != NULL)
            GC::genClass(&(fp->lexicalScope));
#endif
    }

    if (fiber != NULL) {
        GC::markObj(Object::fromCppObj(fiber));
    }

    m_GC_mark_stack();
    Debug::decMarkTreeLevel();
}

// スタック上のObject
void Context::m_GC_mark_stack(void)
{
    HMD__PRINTF_GC("GC mark stack of context %x\n", this);

    // 値スタック上のObject
    for (hyu32 i = 0; i < stack.size(); i++) {
        Value* pv = stack.addrAt(i);
#if 1
        {
            Value* p = pv;
            HMD__PRINTF_GC("mark value on stack ");
            if (p->type == HC_INDIRECT_REF) {
                p = (Value*)p->objPtr->field();
                HMD__PRINTF_GC("(*)");
            }
            HMD__PRINTF_GC("%s:%x\n",p->getType()->name(),p->data);
        }
#endif
        GC::markValue(*pv);
    }
}

//======================================================================
//          for DEBUG
//======================================================================

#ifdef HMD_DEBUG
void Context::debugStackTrace(void)
{
    debugPrintStack();
    m_debugStackTrace();
}

void Context::debugPrintStack(void)
{
    HMD_PRINTF("[");
    int j = frameStack.size() - 1;
    if (j >= 0) {
        hyu32 fp = stack.size() - frameStack.getAt(j).framePos;
        for (hyu32 i = 1; i <= stack.size(); i++) {
            while (i > fp) {
                if (j-- < 1)
                    break;
                HMD_PRINTF("][");
                fp = stack.size() - frameStack.getAt(j).framePos;
            }
            Value d = stack.getNth(i);
            Object* obj = d.objPtr;
            HMD_PRINTF("<");
            if (d.type == HC_REF) {
                HMD_PRINTF("*ref:");
                d.type = obj->type();
                d.ptrData = obj->field();
            }
            if (d.type != NULL)
                HMD_PRINTF("%s,%x>,", d.type->name(), obj);
            else
                HMD_PRINTF("(NULL),%x>",obj);
        }
    }
    HMD_PRINTF("]\n");
}

void Context::m_debugStackTrace(void)
{
    CodePtr ptr = codePtr;
    for (hyu32 i = 0; i < frameStack.size(); i++) {
        Frame_st& fr = frameStack.getNth(i+1);
        m_debugStackTrace1(i, ptr, fr);
        HMD_PRINTF("\n");
        ptr = fr.returnCodePtr;
    }
}
    
void Context::m_debugStackTrace1(int level, const CodePtr codePtr, Frame_st& fr)
{
    char buf[256];
    const HClass* ownerClass = fr.ownerClass;
    const Bytecode* bc = ownerClass->bytecode();
    if (level >= 0)
        HMD_PRINTF("%2d: ", level);
    if (fr.bFfi) {
        HMD_PRINTF("FFI[%s::%s]", ownerClass->name(), gSymbolTable.id2str(fr.f.methodSymbol));
    } else if (bc != NULL) {
        bc->debugGetInfo(buf, 256, codePtr.addr());
        HMD_PRINTF("%s", buf);
    } else {
        HMD_PRINTF("<no bytecode>");
    }
}
    
#endif
