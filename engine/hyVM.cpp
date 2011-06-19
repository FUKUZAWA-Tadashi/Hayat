/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyVM.h"
#include "hyContext.h"
#include "hyValue.h"
#include "opcode_def.h"
#include "hyBytecode.h"
#include "hyThread.h"
#include "hyHash.h"
#include "hyClosure.h"
#include "hyFiber.h"
#include "hyException.h"
#include "hyMethod.h"
#include "hyEndian.h"
#include "hySymbolTable.h"
#include "hyDebug.h"
#include "htPrintOpcode.h"
#include "hyStringBuffer.h"
#include <stdio.h>


using namespace Hayat::Common;
using namespace Hayat::Engine;


VM VM::m_vm;


VM::OpcodeFunc_t VM::m_opcodeFuncs[NUM_OPCODES] = {
#include "opcode_func_table.inc"
};


VM::VM(void)
    : m_context(NULL), m_multiSubstSignature(NULL)
{
    m_clearOpcodeHistory();
}

VM::~VM()
{
    if (m_multiSubstSignature != NULL)
        delete m_multiSubstSignature;
}

bool VM::m_haveCode(void)
{
    if (m_context == NULL)
        return false;
    if (m_context->isInFfi())
        return true;
    return m_context->codePtr.haveCode();
}

void VM::m_exec1step(void)
{
    if (m_context->isInFfi()) {
        //HMD__PRINTF_FK("VM: after ffi\n");
        m_context->afterFfi();
        return;
    }

    HMD_DEBUG_ASSERT(m_haveCode());

#ifdef HY_ENABLE_BYTECODE_HISTORY
    if (!m_context->isInFfi())
        m_addOpcodeHistory(m_context->codePtr, m_context->ownerClass());
#endif
    if (Debug::isPrintStack())
        m_context->debugPrintStack();
    if (Debug::isPrintOpcode() && (!m_context->isInFfi()))
        m_printOpcode(m_context->codePtr, m_context->ownerClass());

    hyu8 opcode = m_context->getCodeHYU8();

    if (opcode < NUM_OPCODES) {
        (m_vm.*m_opcodeFuncs[opcode])();
    } else {
        HMD_FATAL_ERROR("unknown opcode 0x%x", opcode);
        if (Debug::isPrintOpcode())
            m_printOpcode(m_context->codePtr, m_context->ownerClass());
    }            

    if (gMemPool->isAlertAbsorbLevel()) {
        HMD__PRINTF_GC("GC by alert\n");
        GC::full();
        if (m_alertGCCallback != NULL)
            m_alertGCCallback();
    }
}

void VM::m_waitTicks(hys32 ticks)
{
    if (m_context->thread == NULL)
        HMD_FATAL_ERROR("cannot wait without thread");
    m_context->thread->waitTicks(ticks);
}


#ifdef HMD_DEBUG
void VM::debugStackTrace(void)
{
    static bool m_avoidLoop = false;
    if (m_avoidLoop) {
        HMD_PRINTF("INTERNAL BUG: VM::debugStackTrace() called recursively\n");
        m_avoidLoop = false;  // ����Ăяo���p�ɏ�����
        HMD_HALT();
    }
    m_avoidLoop = true;
    printOpcodeHistory();
    if (m_vm.m_context == NULL)
        HMD_PRINTF("VM::debugStackTrace() : VM has no context\n");
    else 
        m_vm.m_context->debugStackTrace();
    m_avoidLoop = false;
}
#endif


#ifdef HY_ENABLE_BYTECODE_HISTORY
void VM::printOpcodeHistory(void)
{
    m_vm.m_printOpcodeHistory();
}
void VM::clearOpcodeHistory(void)
{
    m_vm.m_clearOpcodeHistory();
}
void VM::m_clearOpcodeHistory(void)
{
    m_opcodeHistory_start = 0;
    m_opcodeHistory_end = 0;
}
#endif

//========================================
//
//          opcode
//
//========================================

// nop
void VM::m_opcode_NOP(void)
{
}

// top���R�s�[ [x] -> [x,x]
void VM::m_opcode_COPY_TOP(void)
{
    m_context->stack.copyNth(1);
}

// 2nd���R�s�[ [x,y] -> [y,x,y]
void VM::m_opcode_COPY_2ND(void)
{
    m_context->stack.copyNth(2);
}

// 3rd���R�s�[ [x,y,z] -> [z,x,y,z]
void VM::m_opcode_COPY_3RD(void)
{
    m_context->stack.copyNth(3);
}

// top��2nd���R�s�[ [x,y] -> [x,y,x,y]
void VM::m_opcode_COPY_TOP_2ND(void)
{
    m_context->stack.copyNth(2);
    m_context->stack.copyNth(2);
}

// top��2nd������ [x,y] -> [y,x]
void VM::m_opcode_SWAP(void)
{
    m_context->stack.rot(2);
}

// top��3rd�̉��Ɉړ� [x,y,z] -> [y,z,x]
void VM::m_opcode_ROTATE_R(void)
{
    m_context->stack.rotr(3);
}

// nil ���v�b�V�� []->[nil]
void VM::m_opcode_PUSH_NIL(void)
{
    m_context->pushNil();
}

// self ���v�b�V�� []->[self]
void VM::m_opcode_PUSH_SELF(void)
{
    m_context->push(m_context->curFrame->self);
}

// ���� 0 ���v�b�V�� []->[0]
void VM::m_opcode_PUSH_I0(void)
{
    m_context->push(INT_0_VALUE);
}

// ���� 1 ���v�b�V�� []->[1]
void VM::m_opcode_PUSH_I1(void)
{
    m_context->push(INT_1_VALUE);
}
    
// ���� -1 ���v�b�V�� []->[-1]
void VM::m_opcode_PUSH_IM1(void)
{
    m_context->push(INT_M1_VALUE);
}
    
// �����t��8bit�������v�b�V��  :INT8 []->[i]
void VM::m_opcode_PUSH_I8(void)
{
    m_context->pushInt(m_context->getCodeHYS8());
}
    
// �������v�b�V�� :INT []->[i]
void VM::m_opcode_PUSH_I(void)
{
    m_context->pushInt(m_context->getCodeHYS32());
}
    
// ���� 0.0 ���v�b�V�� []->[0.0]
void VM::m_opcode_PUSH_F0(void)
{
    m_context->pushFloat(0.0f);
}
    
// ���� 1.0 ���v�b�V�� []->[1.0]
void VM::m_opcode_PUSH_F1(void)
{
    m_context->pushFloat(1.0f);
}

// ���� -1.0 ���v�b�V�� []->[-1.0]
void VM::m_opcode_PUSH_FM1(void)
{
    m_context->pushFloat(-1.0f);
}
    
// �������v�b�V�� :FLOAT []->[r]
void VM::m_opcode_PUSH_F(void)
{
    m_context->pushFloat(m_context->getCodeHYF32());
}

// ������萔���v�b�V�� :STRCONST []->[s]
void VM::m_opcode_PUSH_STRCONST(void)
{
    const hyu8* str = m_context->ownerClass()->bytecode()->getString(m_context->getCodeHYU32());
    Value d(HC_String, (void*)str);
    m_context->push(d);
}

// �V���{�����v�b�V�� :SYMBOL []->[sym]
void VM::m_opcode_PUSH_SYMBOL(void)
{
    m_context->pushSymbol(m_context->getCodeSymbolID());
}

// true���v�b�V�� [] -> [true]
void VM::m_opcode_PUSH_TRUE(void)
{
    m_context->push(TRUE_VALUE);
}

// false���v�b�V�� [] -> [false]
void VM::m_opcode_PUSH_FALSE(void)
{
    m_context->push(FALSE_VALUE);
}
    
// Array���e�����ɂ����鏉���l��� :UINT8 [xn, .. ,x2,x1,array]->[array]
void VM::m_opcode_SUBSTARRAY(void)
{
    // [xn,...,x2,x1,array] -> [array]
    hys32 numSubst = (hys32)m_context->getCodeHYU8(); // �����
    hys32 sp = m_context->stack.size();
    Value* pArray = m_context->stack.addrAt(sp - numSubst - 1);
    Object* arrObj = pArray->toObj(HSym_Array);
    ValueArray* arr = arrObj->cppObj<ValueArray>();
    hys32 idx = arr->m_size;
    HMD_ASSERT((hyu32)(idx + numSubst) <= arr->m_capacity);
    for (hys32 i = numSubst ; i > 0; --i) {
        Value v = m_context->stack.getNth(i);
        GC::writeBarrier(v);
        arr->m_contents[idx++] = v;
    }
    arr->m_size = idx;
    m_context->stack.drop(numSubst);
}

// Hash���e�����ɂ����鏉���l��� :UINT8 [vn,kn, .. ,v2,k2,v1,k1,hash]->[hash]
void VM::m_opcode_SUBSTHASH(void)
{
    // [vn,kn, .. ,v2,k2,v1,k1,hash]->[hash]
    hys32 numSubst = (hys32)m_context->getCodeHYU8(); // ����y�A��
    hys32 sp = m_context->stack.size();
    Value* pHash = m_context->stack.addrAt(sp - numSubst * 2 - 1);
    Object* hashObj = pHash->toObj(HSym_Hash);
    Hash* hash = hashObj->cppObj<Hash>();
    for (hys32 i = numSubst; i > 0; --i) {
        Value& key = m_context->stack.getNth(i*2);
        Value& val = m_context->stack.getNth(i*2-1);
        GC::writeBarrier(key);
        GC::writeBarrier(val);
        hash->put(m_context, key, val);
    }
    m_context->stack.drop(numSubst * 2);
}
    
// �󃊃X�g���v�b�V�� [] -> ['()]
void VM::m_opcode_PUSHEMPTYLIST(void)
{
    m_context->push(EMPTY_LIST_VALUE);
}

// cons�Z������� [x,list] -> [list]
void VM::m_opcode_CONS(void)
{
    Object* newObj = HC_List->createInstance_noInit(sizeof(ValueList));
    ValueList* newCell = newObj->cppObj<ValueList>();
    Value hd = m_context->pop();
    GC::writeBarrier(hd);
    ValueList* list = m_context->popList();
    newCell->setContent(hd);
    newCell->link(list);
    m_context->pushList(newCell);
}

// �|�b�v [x]->[]
void VM::m_opcode_POP(void)
{
    m_context->pop();
}
    
// 2nd���̂Ă� [x,y,z] -> [x,z]
void VM::m_opcode_DROP_2ND(void)
{
    ValueStack& stk = m_context->stack;
    stk.removeAt(stk.size() - 2);
}
    
// ����0�̃C���X�^���X���\�b�h�R�[�� :METHOD [self]->[retval]
void VM::m_opcode_INSMETHOD_0(void)
{
    m_context->methodCall(m_context->getCodeSymbolID(), 0);
}

// ����1�̃C���X�^���X���\�b�h�R�[�� :METHOD [self,x]->[retval]
void VM::m_opcode_INSMETHOD_1(void)
{
    m_context->methodCall(m_context->getCodeSymbolID(), 1);
}
    
// ����2�̃C���X�^���X���\�b�h�R�[�� :METHOD [self,x2,x1]->[retval]
void VM::m_opcode_INSMETHOD_2(void)
{
    m_context->methodCall(m_context->getCodeSymbolID(), 2);
}
    
// ����n�̃C���X�^���X���\�b�h�R�[�� :SIGNATURE,:METHOD [self,xn,..,x1]->[retval]
void VM::m_opcode_INSMETHOD(void)
{
    hyu16 signatureID = m_context->getCodeHYU16();
    const Bytecode* pBytecode = m_context->lexicalScope()->bytecode();
    Signature sig(pBytecode->getSignatureArity(signatureID) + 1,
                  pBytecode->getSignatureBytes(signatureID),
                  pBytecode->getSignatureBytesLen(signatureID),
                  m_context);
    m_context->methodCall(m_context->getCodeSymbolID(), &sig);
}

// ����1�̃C���X�^���X���\�b�h�R�[��2 :METHOD [x,self]->[retval]
void VM::m_opcode_INSMETHODR_1(void)
{
    m_context->methodCallR(m_context->getCodeSymbolID(), 1);
}
    
// ����2�̃C���X�^���X���\�b�h�R�[��2 :METHOD [x2,x1,self]->[retval]
void VM::m_opcode_INSMETHODR_2(void)
{
    m_context->methodCallR(m_context->getCodeSymbolID(), 2);
}
    
// ����n�̃C���X�^���X���\�b�h�R�[��2 :SIGNATURE,:METHOD [xn,..,x1,self]->[retval]
void VM::m_opcode_INSMETHODR(void)
{
    hyu16 signatureID = m_context->getCodeHYU16();
    const Bytecode* pBytecode = m_context->lexicalScope()->bytecode();
    Signature sig(pBytecode->getSignatureArity(signatureID) + 1,
                  pBytecode->getSignatureBytes(signatureID),
                  pBytecode->getSignatureBytesLen(signatureID),
                  m_context);
    m_context->methodCallR(m_context->getCodeSymbolID(), &sig);
}

// ����0�̃��\�b�h�R�[�� :METHOD []->[retval]
void VM::m_opcode_METHOD_0(void)
{
    m_context->sMethodCall(m_context->getCodeSymbolID(), 0);
}
    
// ����1�̃��\�b�h�R�[�� :METHOD [x]->[retval]
void VM::m_opcode_METHOD_1(void)
{
    m_context->sMethodCall(m_context->getCodeSymbolID(), 1);
}
    
// ����2�̃��\�b�h�R�[�� :METHOD [x2,x1]->[retval]
void VM::m_opcode_METHOD_2(void)
{
    m_context->sMethodCall(m_context->getCodeSymbolID(), 2);
}
    
// ����n�̃��\�b�h�R�[�� :SIGNATURE,:METHOD [xn,...,x1]->[retval]
void VM::m_opcode_METHOD(void)
{
    hyu16 signatureID = m_context->getCodeHYU16();
    const Bytecode* pBytecode = m_context->lexicalScope()->bytecode();
    Signature sig(pBytecode->getSignatureArity(signatureID),
                  pBytecode->getSignatureBytes(signatureID),
                  pBytecode->getSignatureBytesLen(signatureID),
                  m_context);
    m_context->sMethodCall(m_context->getCodeSymbolID(), &sig);
}

// ����n�̎w��N���X�̃��\�b�h�R�[�� :SIGNATURE,:METHOD [class,xn,...,x1]->[retval]
void VM::m_opcode_SCOPEMETHOD(void)
{
    hyu16 signatureID = m_context->getCodeHYU16();
    const Bytecode* pBytecode = m_context->lexicalScope()->bytecode();
    Signature sig(pBytecode->getSignatureArity(signatureID) + 1,
                  pBytecode->getSignatureBytes(signatureID),
                  pBytecode->getSignatureBytesLen(signatureID),
                  m_context);
    SymbolID_t methodSym = m_context->getCodeSymbolID();
    const HClass* pClass = m_context->popClass();
    m_context->scopeMethodCall(methodSym, pClass, &sig);
}

// ����n��new���Z�q :CLASS,:SIGNATURE [xn,..,x1]->[newObj]
void VM::m_opcode_NEW(void)
{
    SymbolID_t classSym = m_context->getCodeSymbolID();
    const HClass* klass = m_context->lexicalScope()->getScopeClass(classSym);
    HMD_ASSERTMSG(klass != NULL, M_M("unknown class '%s'"), gSymbolTable.id2str(classSym));
    hyu16 signatureID = m_context->getCodeHYU16();
    const Bytecode* pBytecode = m_context->lexicalScope()->bytecode();
    Signature sig(pBytecode->getSignatureArity(signatureID),
                  pBytecode->getSignatureBytes(signatureID),
                  pBytecode->getSignatureBytesLen(signatureID),
                  m_context);

    if (! klass->isPrimitive()) {
        Object* obj = klass->createInstance(m_context, &sig, true);
        // initialize()�ŗ�O�����ȂǂŎ��s����� obj==NULL �ƂȂ�
        // ��O��Ԃ�push����Ɠ��R�X�^�b�N�������̂�push���Ȃ�
        if (obj != NULL)
            m_context->pushObj(obj);
        return;
    }

    // �����o�ϐ���C++�I�u�W�F�N�g�������Ȃ��N���X
#ifdef HMD_DEBUG
    hyu32 frameSP = m_context->frameStack.size();
    hyu32 sp = m_context->stack.size() - sig.getNumOnStack();
#endif
    Value v(klass, (hyu32)0);
    int er = m_context->execMethod_ifExist(v, HSym_initialize, &sig);
    if (er == 1) {
        // ���s����
        if (m_context->stack.getTop().type != klass) {
            m_context->throwException(HSym_bad_constructor,
                                      M_M("initialize() must return an object of class '%s'"),
                                      klass->name());
            return;
        }
    } else if (er == -1) {
        // ���s���s�Fexception���������Ă���
        return;
    } else {
        // initialize()����������
        sig.clearRight();
        m_context->push(v);
    }
    HMD_DEBUG_ASSERTMSG(frameSP == m_context->frameStack.size(),
                        "frameSP=%d,frameStack.sp=%d,er=%d",
                        frameSP,m_context->frameStack.size(),er);
    HMD_DEBUG_ASSERTMSG(m_context->stack.size() == sp + 1,
                        "stack size expected %d but actually %d",
                        sp+1, m_context->stack.size());
    return;
}

// ���[�J���ϐ����v�b�V�� :LOCALVAR []->[x]
void VM::m_opcode_GETLOCAL(void)
{
    m_context->push(m_context->getLocalVar(m_context->getCodeHYS16()));
}
    
// ���[�J���ϐ����Z�b�g :LOCALVAR [x]->[x]
void VM::m_opcode_SETLOCAL(void)
{
    m_context->setLocalVar(m_context->getCodeHYS16(), m_context->stack.getTop());
}
    
// �O���̃X�R�[�v�̃��[�J���ϐ������L���� :UINT16,:UINT16 [closure]->[closure]
void VM::m_opcode_SHARELOCAL(void)
{
    hys16 localVarIdx = m_context->getCodeHYS16(); // �N���[�W�����[�J���ϐ�index
    HMD_DEBUG_ASSERT(localVarIdx >= 0 && localVarIdx < 256);
    hyu16 outVarIdx = m_context->getCodeHYU16();   // �O�����[�J���ϐ�index
    Object* shared = m_context->shareLocalVar(outVarIdx);
    Value& c = m_context->stack.getTop();
    Closure* closure = c.toObj(HSym_Closure)->cppObj<Closure>(HSym_Closure);
    closure->setSharedLocal((hyu8)localVarIdx, shared);
}
    
// �N���X�ϐ����v�b�V�� :CLASSVAR []->[x]
void VM::m_opcode_GETCLASSVAR(void)
{
    m_context->push(m_context->getClassVar(m_context->getCodeHYS16()));
}
    
// �N���X�ϐ����Z�b�g :CLASSVAR [x]->[x]
void VM::m_opcode_SETCLASSVAR(void)
{
    m_context->setClassVar(m_context->getCodeHYS16(), m_context->stack.getTop());
}
    
// �����o�ϐ����v�b�V�� :MEMBVAR []->[x]
void VM::m_opcode_GETMEMBVAR(void)
{
    m_context->push(m_context->getMembVar(m_context->getCodeHYS16()));
}
    
// �����o�ϐ����Z�b�g :MEMBVAR [x]->[x]
void VM::m_opcode_SETMEMBVAR(void)
{
    m_context->setMembVar(m_context->getCodeHYS16(), m_context->stack.getTop());
}
    
// �O���[�o���ϐ����v�b�V�� :GLOBALVAR []->[x]
void VM::m_opcode_GETGLOBAL(void)
{
    m_context->push(m_context->getGlobalVar(m_context->getCodeHYS16()));
}
    
// �O���[�o���ϐ����Z�b�g :GLOBALVAR [x]->[x]
void VM::m_opcode_SETGLOBAL(void)
{
    m_context->setGlobalVar(m_context->getCodeHYS16(), m_context->stack.getTop());
}
    
// �N���X���v�b�V�� :CLASS []->[x]
void VM::m_opcode_GETCLASS(void)
{
    SymbolID_t csym = m_context->getCodeSymbolID();
    const HClass* pClass = m_context->lexicalScope()->getScopeClass(csym);
    HMD_ASSERTMSG(pClass != NULL, M_M("unknown class '%s'"),gSymbolTable.id2str(csym));
    m_context->pushClass(pClass);
}
    
// �萔���v�b�V�� :CONSTVAR []->[x]
void VM::m_opcode_GETCONSTVAR(void)
{
    m_context->push(m_context->getConstVar(m_context->getCodeHYS16()));
}

// �萔���Z�b�g :CONSTVAR [x]->[x]
void VM::m_opcode_SETCONSTVAR(void)
{
    m_context->setConstVar(m_context->getCodeHYS16(), m_context->stack.getTop());
}

// getter [obj]->[x]
void VM::m_opcode_GETMEMBER(void)
{
    SymbolID_t sym = m_context->getCodeSymbolID();

    // getter���\�b�h������ΌĂяo��
    if (m_context->methodCall_ifExist(sym, 0))
        return;

    Value val = m_context->pop();
    if (val.type == HC_REF) {
        // �C���X�^���X
        Object* obj = val.objPtr;
        const HClass* pClass = obj->type();
        Value* addr = pClass->getMembVarAddr(obj, sym);
        if (addr != NULL) {
            // �����o�ϐ����������̂ł����Ԃ�
            m_context->push(*addr);
            return;
        }

        m_context->throwException(HSym_cannot_access_member,
                                  M_M("cannot get member '%s' of insance of class '%s'"),
                                  gSymbolTable.id2str(sym),
                                  pClass->name());

    } else if (val.type == HC_Class) {
        // �N���X
        const HClass* pClass = val.toClass();
        Value* addr = pClass->getClassVarAddr(sym);
        if (addr != NULL) {
            // �N���X�ϐ����������̂ł����Ԃ�
            m_context->push(*addr);
            return;
        }

        m_context->throwException(HSym_cannot_access_member,
                                  M_M("cannot get class variable '%s' in class '%s'"),
                                  gSymbolTable.id2str(sym),
                                  pClass->name());

    } else {
        m_context->throwException(HSym_cannot_access_member,
                                  M_M("class '%s' has no member '%s'"),
                                  val.type->name(),
                                  gSymbolTable.id2str(sym));
    }
}

// setter [obj,x]->[x]
void VM::m_opcode_SETMEMBER(void)
{
    SymbolID_t varSym = m_context->getCodeSymbolID();
    SymbolID_t methodSym = m_context->getCodeSymbolID();

    // setter���\�b�h������ΌĂяo��
    if (m_context->methodCall_ifExist(methodSym, 1))
        return;

    Value val = m_context->pop();
    if (val.type == HC_REF) {
        // �C���X�^���X
        Object* obj = val.objPtr;
        const HClass* pClass = obj->type();
        Value* addr = pClass->getMembVarAddr(obj, varSym);
        if (addr != NULL) {
            // �����o�ϐ����������̂ŃZ�b�g����
            *addr = m_context->stack.getTop();
            return;
        }
        m_context->throwException(HSym_cannot_access_member,
                                  M_M("cannot set member '%s' of instance of class '%s'"),
                                  gSymbolTable.id2str(varSym),
                                  pClass->name());

    } else if (val.type == HC_Class) {
        // �N���X
        const HClass* pClass = val.toClass();
        Value* addr = pClass->getClassVarAddr(varSym);
        if (addr != NULL) {
            // �N���X�ϐ����������̂ŃZ�b�g����
            *addr = m_context->stack.getTop();
            return;
        }

        m_context->throwException(HSym_cannot_access_member,
                                  M_M("cannot set class variable '%s' in class '%s'"),
                                  gSymbolTable.id2str(varSym),
                                  pClass->name());

    } else {
        m_context->throwException(HSym_cannot_access_member,
                                  M_M("class '%s' has no member '%s'"),
                                  val.type->name(),
                                  gSymbolTable.id2str(varSym));
    }
}

// �f�t�H���g�l���Z�b�g :UINT16 [x]->[]
void VM::m_opcode_SETDEFAULTVAL(void)
{
    Value v = m_context->pop();
    m_context->setDefaultVal(m_context->getCodeHYU16(), v);
    GC::writeBarrier(v);
}

// �Ō�̒l��߂�l�Ƃ��āA���^�[��
void VM::m_opcode_RETURN(void)
{
    m_context->routineReturn();
}

// callAway�ŌĂ΂ꂽclosure��������A�Ō�̒l��߂�l�Ƃ��ăN���[�W���𐶐��������[�`�����烊�^�[��
// ���̑��̏ꍇ��return�Ɠ���
void VM::m_opcode_RETURNAWAY(void)
{
    HMD_DEBUG_ASSERT(! m_context->curFrame->bFfi);
    m_context->routineReturn(m_context->curFrame->r.creatorMethodID);
}

// �������W�����v :RELATIVE
void VM::m_opcode_JUMP(void)
{
    m_context->jumpRelative(m_context->getCodeHYS32());
}
    
// false�̎��W�����v :RELATIVE
void VM::m_opcode_JUMP_IFFALSE(void)
{
    hys32 relative = m_context->getCodeHYS32();
    Value v = m_context->pop();
    if ((v.type->symCheck(HSym_NilClass)) || ((v.type->symCheck(HSym_Bool)) && (v.data == 0)))
        m_context->jumpRelative(relative);
}
    
// true�̎��W�����v :RELATIVE
void VM::m_opcode_JUMP_IFTRUE(void)
{
    hys32 relative = m_context->getCodeHYS32();
    Value v = m_context->pop();
    if ((! v.type->symCheck(HSym_NilClass)) && ((! v.type->symCheck(HSym_Bool)) || (v.data != 0)))
        m_context->jumpRelative(relative);
}
    
// �O�����x���ւ̃W�����v :JUMPSYMBOL
void VM::m_opcode_JUMPCONTROL(void)
{
    SymbolID_t label = m_context->getCodeSymbolID();
#ifdef SYMBOL_ID_IS_HYU16
    m_context->codePtr.advance(2); // skip 2 byte
#endif
    const hyu8* addr = m_context->getJumpAddr(label);
    if (addr != NULL) {
        m_context->codePtr.jump(addr);
        return;
    }
    Value v = m_context->pop();
    m_context->throwJumpCtlException(label, v);
}

// �X�^�b�N�g�b�v�ɐς܂ꂽ���x���ւ̃W�����v [symbol] -> []
void VM::m_opcode_GOTO(void)
{
    SymbolID_t label = m_context->popSymbol();
    const hyu8* addr = m_context->getJumpAddr(label);
    if (addr != NULL) {
        m_context->codePtr.jump(addr);
        return;
    }
    Value v = m_context->pop();
    m_context->throwGotoException(label, v);
}

// �N���X�C�j�V�����C�U���Ă�(���Ɏ��s����Ă���Ύ��s���Ȃ�) :CLASS
void VM::m_opcode_CLASSINIT(void)
{
    SymbolID_t classSym = m_context->getCodeSymbolID();
    const HClass* pClass = m_context->m_getSelfClass();
    pClass = pClass->getInnerClass(classSym);
    if (pClass == NULL)
        pClass = m_context->lexicalScope()->getScopeClass(classSym);
    HMD_DEBUG_ASSERT(pClass != NULL);
    if (! pClass->isInitialized()) {
        ((HClass*)pClass)->initialized();
        const hyu8* code = pClass->classInitializer();
        if (code != NULL) {
            Value self(HC_Class, (hyu32)pClass);
            MethodPtr mptr(self, code, 0);
            m_context->routineCall(mptr, 0);
        } else {
            m_context->pushNil();
        }
    } else {
        m_context->pushNil();
    }
}

// 1 tick �� wait []->[1]
void VM::m_opcode_WAITTICK_1(void)
{
    m_waitTicks(1);
    m_context->push(INT_1_VALUE);
}

// �w��tick������wait [n] -> [n] (�X�^�b�N�͕ω����Ȃ�)
void VM::m_opcode_WAITTICKS(void)
{
    int n = m_context->stack.getNth(1).toInt();
    m_waitTicks(n);
}

// �X�R�[�v�ɑΉ�����N���X���擾 :SCOPE []->[class]
void VM::m_opcode_GETSCOPECLASS(void)
{
    hyu8 numSymbols = m_context->getCodeHYU8();
    //HMD_ASSERT(numSymbols > 0 && numSymbols < 16);
    SymbolID_t sym = m_context->getCodeSymbolID();
    const HClass* pClass;
    if (sym == HSym_nil) {
        pClass = m_context->lexicalScope()->bytecode()->mainClass();
        HMD_ASSERT(pClass != NULL);
    } else {
        pClass = m_context->lexicalScope();
        HMD_DEBUG_ASSERT(pClass != NULL);
        pClass = pClass->getScopeClass(sym);
        HMD_ASSERTMSG(pClass != NULL,
                      M_M("getScopeClass: class not found: %s"),
                      gSymbolTable.id2str(sym));
    }
    for (hyu8 i = 1; i < numSymbols; i++) {
        sym = m_context->getCodeSymbolID();
        pClass = pClass->getInnerClass(sym);
        HMD_ASSERTMSG(pClass != NULL,
                      M_M("getScopeClass: inner class not found: %s"),
                      gSymbolTable.id2str(sym));
    }
    m_context->pushClass(pClass);
}

// �w��N���X�̒萔������	:CONSTVAR [class]->[x]
void VM::m_opcode_GETCLASSCONST(void)
{
    SymbolID_t constSym = m_context->getCodeSymbolID();
    const HClass* pClass = m_context->popClass();
    Value* addr = pClass->getConstVarAddr(constSym);
    if (addr != NULL) {
        m_context->push(*addr);
        return;
    }

    m_context->throwException(HSym_cannot_access_constant,
                              M_M("cannot access constant '%s::%s'"),
                              pClass->name(),
                              gSymbolTable.id2str(constSym));
}
    
// �w��N���X�̒萔�܂��̓C���i�[�N���X������	:SYMBOL [class]->[x]
void VM::m_opcode_GETSCOPEDVAL(void)
{
    SymbolID_t sym = m_context->getCodeSymbolID();
    const HClass* pClass = m_context->popClass();
    Value* addr = pClass->getConstVarAddr(sym);
    if (addr != NULL) {
        m_context->push(*addr);
        return;
    }

    const HClass* inner = pClass->getInnerClass(sym);
    if (inner != NULL) {
        m_context->pushClass(inner);
        return;
    }

    m_context->throwException(HSym_cannot_access_scoped_val,
                              M_M("cannot access scoped value '%s::%s'"),
                              pClass->name(),
                              gSymbolTable.id2str(sym));
}
    
// jumpControl�X�^�[�g :MAYLOCALVAR,:UINT8
void VM::m_opcode_JUMPCONTROLSTART(void)
{
    m_context->jumpControlStart();
}

// jumpControl�G���h
void VM::m_opcode_JUMPCONTROLEND(void)
{
    m_context->jumpControlEnd();
}
    
// ��O�𓊂���
void VM::m_opcode_THROW(void)
{
    Value e = m_context->pop();
    SymbolID_t sym = m_context->popSymbol();
    m_context->throwException(sym, e);
}

// finally�߂ɃW�����v����   [x]->[]  x��finally�I�����try�u���b�N�̒l�ƂȂ�
void VM::m_opcode_JUMP_FINALLY(void)
{
    Value x = m_context->pop();
    m_context->valueAfterFinally(x);
    const hyu8* finallyAddr = m_context->getJumpAddr(HSymR_finally);
    HMD_DEBUG_ASSERT(finallyAddr != NULL);
    m_context->codePtr.jump(finallyAddr);
}

// finally�߂̏I���
void VM::m_opcode_END_FINALLY(void)
{
    m_context->endFinally();
}
    
// �N���[�W���𐶐����� :UINT16,:UINT8
void VM::m_opcode_CLOSURE(void)
{
    hyu16 closureIdx = m_context->getCodeHYU16();
    hyu8 numSharedLocal = m_context->getCodeHYU8();
    Closure* closure = Closure::create(numSharedLocal);
    const HClass* lexicalScope = m_context->lexicalScope();
    MethodPtr mptr(m_context->curFrame->self);
    lexicalScope->setClosureMethodPtr(closureIdx, &mptr);
    closure->initialize(mptr, numSharedLocal, m_context->getMethodID());
    m_context->pushObj(closure->getObj());
}

// yield
void VM::m_opcode_YIELD(void)
{
    if (m_context->fiber == NULL) {
        m_context->throwError(M_M("yield out of fiber"));
    } else {
        m_context->fiber->yielding();
    }
}

// ���݂�context��fiber�ɂ���
void VM::m_opcode_BEFIBER(void)
{
    Fiber* fiber = new Fiber(m_context);
    m_context->pushObj(Object::fromCppObj(fiber));
}

// ���ꂩ��X�^�b�N�ɐςޑ��d����E�ӂ̃V�O�l�`�����w��  []->[]
// ���d������[�L���O�ϐ����Z�b�g
void VM::m_opcode_MS_RIGHT(void)
{
    HMD_DEBUG_ASSERT(m_multiSubstSignature == NULL);
    hyu16 signo = m_context->getCodeHYU16();
    const Bytecode* bytecode = m_context->lexicalScope()->bytecode();
    m_multiSubstSignature = new Signature(0,
                                         bytecode->getSignatureBytes(signo),
                                         bytecode->getSignatureBytesLen(signo),
                                         m_context);
}

// ���d������ӂɉE�ӂ�����\�����ׂāA���s�Ȃ�W�����v :UINT16,:RELATIVE
// [xn,...,x1]->[xm,...,x1]
void VM::m_opcode_MS_TESTLEFT(void)
{
    HMD_DEBUG_ASSERT(m_multiSubstSignature != NULL);
    hyu16 signo = m_context->getCodeHYU16();
    hys32 relative = m_context->getCodeHYS32();
    const Bytecode* bytecode = m_context->lexicalScope()->bytecode();
#if 0
    HMD_PRINTF("test right ");
    m_multiSubstSignature->printSigs();
    HMD_PRINTF("  with left ");
    const Signature::Sig_t* p = bytecode->getSignatureBytes(signo);
    Signature::printSig(p);
    HMD_PRINTF("\n");
#endif
    if (m_multiSubstSignature->canSubstTo(bytecode->getSignatureBytes(signo), NULL, 0)) {
        // ����
        delete m_multiSubstSignature;
        m_multiSubstSignature = NULL;
    } else {
        // ���s
        m_context->jumpRelative(relative);
    }
}

// �X�^�b�N�ɐς܂�Ă��鑽�d����E�ӂƑ��d������[�L���O�ϐ����N���A [xn,...,x1]->[]
void VM::m_opcode_MS_END(void)
{
    HMD_DEBUG_ASSERT(m_multiSubstSignature != NULL);
    m_multiSubstSignature->clearRight();
    delete m_multiSubstSignature;
    m_multiSubstSignature = NULL;
}

// �R���p�C�����ɂ͉������Ȃ������V���{���ɑΉ�����l�𓮓I�Ɏ擾
void VM::m_opcode_GETSOMETHING(void)
{
    SymbolID_t sym = m_context->getCodeSymbolID();
    const HClass* lexicalScope = m_context->lexicalScope();
    const HClass* klass = lexicalScope->getScopeClass(sym, true);
    if (klass != NULL) {
        m_context->pushClass(klass);
        return;
    }
    Value* vp = lexicalScope->getConstVarAddr(sym);
    if (vp != NULL) {
        m_context->push(*vp);
        return;
    }
    if (m_context->methodCall_ifExist(m_context->curFrame->self, sym, 0))
        return;
    Signature sig(0, m_context);
    if (m_context->m_topMethodCall(sym, &sig))
        return;

    m_context->throwException(HSym_unknown_identifier,
                              M_M("unknown identifier '%s'"),
                              gSymbolTable.id2str(sym));
}

// sprintf(fmt, x)  [x] -> [stringBuffer]
void VM::m_opcode_SPRINTF(void)
{
    const char* fmt = (const char*) m_context->ownerClass()->bytecode()->getString(m_context->getCodeHYU32());
    hyu8 n = m_context->getCodeHYU8();
    StringBuffer* sb = new StringBuffer();
    if (n == 0) {
        hys32 i = m_context->popInt();
        sb->sprintf(fmt, i);
    } else if (n == 1) {
        hyf32 f = m_context->popFloat();
        sb->sprintf(fmt, f);
    } else if (n == 2) {
        const char* s = m_context->popString();
        sb->sprintf(fmt, s);
    } else {
        HMD_FATAL_ERROR("illegal operand of SPRINTF");
    }
    m_context->pushObj(sb->getObj());
}





void VM::m_printOpcode(const CodePtr& codePtr, const HClass* scope)
{
    Hayat::Tool::printOpcode(codePtr.addr(), scope->bytecode());
}

#ifdef HY_ENABLE_BYTECODE_HISTORY

void VM::m_addOpcodeHistory(const CodePtr& codePtr, const HClass* scope)
{
    m_opcodeHistory[m_opcodeHistory_end].codePtr = codePtr;
    m_opcodeHistory[m_opcodeHistory_end].scope = scope;
    if (++m_opcodeHistory_end >= m_MAX_OPCODE_HISTORY)
        m_opcodeHistory_end = 0;
    if (m_opcodeHistory_end == m_opcodeHistory_start) {
        if (++m_opcodeHistory_start >= m_MAX_OPCODE_HISTORY)
            m_opcodeHistory_start = 0;
    }
}
    
void VM::m_printOpcodeHistory(void)
{
    HMD_PRINTF("----- last opcode -----\n");
    for (int i = m_opcodeHistory_start; i != m_opcodeHistory_end; ) {
        m_printOpcode(m_opcodeHistory[i].codePtr, m_opcodeHistory[i].scope);
        if (++i >= m_MAX_OPCODE_HISTORY)
            i = 0;
    }
    HMD_PRINTF("-----\n");
}


#endif /* HY_ENABLE_BYTECODE_HISTORY */
