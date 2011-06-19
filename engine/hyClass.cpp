/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyClass.h"
#include "hyEndian.h"
#include "hyObject.h"
#include "hyClass.h"
#include "hySymbolTable.h"
#include "hyMemPool.h"
#include "hyThread.h"
#include "hyThreadManager.h"
#include "hyBytecode.h"
#include "hyBinarySearch.h"
#include "hyDebug.h"
#include "hyVM.h"


using namespace Hayat::Common;
using namespace Hayat::Engine;

TArray<const HClass::ClassFfiEntry_t*> HClass::m_rootFfiTable(0);


//========== コンストラクタ、デストラクタ ==========

HClass::HClass(SymbolID_t classSymbol, const Bytecode* pBytecode, const hyu8* code)
    : m_pBytecode(pBytecode), m_classSymbol(classSymbol),
      m_classVarSymArr(0), m_membVarSymArr(0), m_constVarSymArr(0),
      m_classVarHash(0), m_constVarHash(0)
{
    //HMD__PRINTF_FK("class init %s code=%x\n",gSymbolTable.id2str(classSymbol),code);
    int i;
    hyu16 flags = Endian::unpack<hyu16>(code);
    m_bPrimitive = ((flags & 1) != 0);
    m_numMethods = Endian::unpack<hyu16>(code + 2);
    m_numInnerClasses = Endian::unpack<hyu16>(code + 4);
    m_numSuper = Endian::unpack<hyu16>(code + 6);
    hyu16 numClassVar = Endian::unpack<hyu16>(code + 8);
    hyu16 numMembVar = Endian::unpack<hyu16>(code + 10);
    hyu16 numConstVar = Endian::unpack<hyu16>(code + 12);
    m_numDefaultVal = Endian::unpack<hyu16>(code + 14);
    m_numClosures = Endian::unpack<hyu16>(code + 16);
    int numMethods = m_numMethods + m_numClosures;

    //HMD__PRINTF_FK("this=%x methods=%d, closures=%d, class=%d, super=%d, classvar=%d, insvar=%d, const=%d\n", this, m_numMethods, m_numClosures, m_numInnerClasses, m_numSuper, numClassVar, numMembVar, numConstVar);

    

    m_methodSymTable = (SymbolID_t*)(code + 20);
    for (i = 0; i < m_numMethods; i++)
        Endian::rewrite<SymbolID_t>(&m_methodSymTable[i]); // endian書換え

    m_classSymTable = m_methodSymTable + m_numMethods;
    for (i = 0; i < m_numInnerClasses; i++)
        Endian::rewrite<SymbolID_t>(&m_classSymTable[i]);

    SymbolID_t* p = m_classSymTable + m_numInnerClasses;
    for (i = 0; i < numClassVar; ++i)
        Endian::rewrite<SymbolID_t>(&p[i]);
    m_classVarSymArr.initialize(p, numClassVar);

    p += numClassVar;
    for (i = 0; i < numMembVar; ++i)
        Endian::rewrite<SymbolID_t>(&p[i]);
    m_membVarSymArr.initialize(p, numMembVar);

    p += numMembVar;
    for (i = 0; i < numConstVar; ++i)
        Endian::rewrite<SymbolID_t>(&p[i]);
    m_constVarSymArr.initialize(p, numConstVar);

    p += numConstVar;
    m_methodTable = (hyu32*)alignInt<4>((int)p);
    m_innerClassTable = (hyu32*)(m_methodTable + numMethods);
    m_superTable = (hyu32*)(m_innerClassTable + m_numInnerClasses + 1);

    m_usingPaths = (HClass**)(m_superTable + m_numSuper);
    hyu16 usingInfoSize = Endian::unpack<hyu16>((const hyu8*)m_usingPaths);

    m_codes = ((const hyu8*)m_usingPaths) + usingInfoSize;

    // メソッドバイトコードアドレス書換え
    for (i = 0; i < numMethods; i++)
        m_methodTable[i] = (hyu32)(m_codes + Endian::unpack<hyu32>((const hyu8*)&m_methodTable[i]));
    // インナークラス初期化 & テーブル書換え
    for (i = 0; i < m_numInnerClasses; i++) {
        //HMD__PRINTF_FK(" inner class %d\n",i);
        m_innerClassTable[i] = (hyu32) new HClass(m_classSymTable[i], m_pBytecode, m_codes + Endian::unpack<hyu32>((const hyu8*)&m_innerClassTable[i]));
    }

    // クラス変数、定数のプールを確保
    int numPoolVar = m_classVarSymArr.size() + m_constVarSymArr.size() + m_numDefaultVal;
    if (numPoolVar > 0) {
        m_varPool = gMemPool->allocT<Value>(numPoolVar, "VARP");
        for (i = 0; i < numPoolVar; i++) {
            m_varPool[i] = NIL_VALUE;
        }
    } else {
        m_varPool = NULL;
    }

    // ハッシュのメモリ領域確保
    m_classVarHash.initialize(m_classVarSymArr.size());
    m_constVarHash.initialize(m_constVarSymArr.size());

    // ジャンプテーブルのendian書換え
    m_rewriteJumpTableEndian(m_codes);    // classInitializer
    for (i = 0; i < m_numMethods + m_numClosures; i++)
        m_rewriteJumpTableEndian((const hyu8*)m_methodTable[i]); // method+closure

    m_ffiTable = NULL;

    m_bInitialized = false;
#ifdef HMD_DEBUG
    m_defaultValSetLimit = 0;
#endif

    // スーパークラステーブル書換えと、usingパス情報書換えは、
    // 全クラスのロード終了後に m_resolveTable() にて行なう。
}
    

/*static*/ void HClass::m_rewriteJumpTableEndian(const hyu8* table)
{
    hyu16 tblSize = Endian::unpack<hyu16>(table); // ジャンプテーブルサイズ
    *(hyu16*)table = tblSize;
    int numTbl = table[2];
    hyu16* tblOffs = (hyu16*)(table + 4);
    const hyu8* p;
    hys32* jumpAddrs = (hys32*)(table + tblSize);
    for (int i = 0; i < numTbl; i++) {
        Endian::rewrite<hyu16>(tblOffs);
        p = table + *tblOffs++;
        int numLabel = *p;
        SymbolID_t* symAddr = (SymbolID_t*)(p + 4);
        jumpAddrs = (hys32*)(p + 4 + sizeof(SymbolID_t) * numLabel);
        alignPtr<4>(&jumpAddrs);
        while (numLabel-- > 0) {
            Endian::rewriteP<SymbolID_t>(&symAddr);
            Endian::rewriteP<hys32>(&jumpAddrs);
        }
    }
    HMD_DEBUG_ASSERT(table + tblSize == (const hyu8*)jumpAddrs);
}


HClass::HClass(SymbolID_t symbol)
    : m_pBytecode(NULL),
      m_classSymbol(symbol),
      m_bPrimitive(true),
      m_numMethods(0),
      m_numInnerClasses(0),
      m_numSuper(0),
      m_classVarSymArr(0),
      m_membVarSymArr(0),
      m_constVarSymArr(0),
      m_varPool(NULL),
      m_classVarHash(0),
      m_constVarHash(0)
{
}

HClass::~HClass(void)
{
    //HMD_PRINTF("~HClass()  %x\n",this);
    if (m_varPool != NULL)
        gMemPool->free(m_varPool);
    m_varPool = NULL;
    for (int i = m_numInnerClasses - 1; i >= 0; i--) {
        delete (HClass*)(m_innerClassTable[i]);
        m_innerClassTable[i] = (hyu32)NULL;
    }
}


void* HClass::operator new(size_t size)
{
    return (void*) gMemPool->alloc(size, "CLAS");
}
    
void HClass::operator delete(void* p)
{
    HMD_DEBUG_ASSERTMSG(((MemCell*)p)->size() > 0,
                        M_M("delete BUG !!  in class %s\n"), ((HClass*)p)->name());
    gMemPool->free(p);
}



//========== インスタンスメソッド ==========


// self, inner
const HClass* HClass::getInnerClass(SymbolID_t classSymbol) const
{
    if (classSymbol == m_classSymbol)
        return this;
    int n;
    if (binarySearch<hyu16>(m_classSymTable, m_numInnerClasses, classSymbol, &n))
        return (const HClass*)m_innerClassTable[n];
    return NULL;
}


// self, inner, super, super->inner, super->..->super->inner
const HClass* HClass::getNearClass(SymbolID_t classSymbol) const
{
    const HClass* pClass = getInnerClass(classSymbol);
    if (pClass != NULL) return pClass;
    for (int i = 0; i < m_numSuper; i++) {
        pClass = m_super(i)->getNearClass(classSymbol);
        if (pClass != NULL) return pClass;
    }
    return NULL;
}


// このクラスのusingパスから探す
const HClass* HClass::getClassInUsing(SymbolID_t classSymbol) const
{
    for (int i = 0; i < m_numUsingPaths; i++) {
        const HClass* pClass = m_usingPaths[i]->getNearClass(classSymbol);
        if (pClass != NULL) return pClass;
    }
    return NULL;
}


const HClass* HClass::getScopeClass(SymbolID_t classSymbol, bool bNotSearchUsingPath) const
{
    // nearクラスから探す
    const HClass* pClass = getNearClass(classSymbol);
    if (pClass != NULL) return pClass;

    // outerのnearクラスから探す
    const HClass* oc = m_outerClass;
    while (oc != NULL) {
        pClass = oc->getNearClass(classSymbol);
        if (pClass != NULL) return pClass;
        oc = oc->m_outerClass;
    }
    
    if (bNotSearchUsingPath)
        return NULL;
    // usingから探す
    pClass = getClassInUsing(classSymbol);
    if (pClass != NULL) return pClass;
    oc = m_outerClass;
    while (oc != NULL) {
        pClass = oc->getClassInUsing(classSymbol);
        if (pClass != NULL) return pClass;
        oc = oc->m_outerClass;
    }
    return NULL;


    // ルート、バイトコードリンクから探す
    //    return m_pBytecode->searchClass(classSymbol);
}

size_t HClass::fieldSize(void) const
{
    return m_membVarSymArr.size() * sizeof(Value);
}

Object* HClass::createInstance(Context* context, Signature* pSig, bool bInit) const
{
    // C++インスタンス部分のサイズを取得するために
    // クラスメソッド "*cppSize" の呼び出し
    size_t cppFieldSize = 0;
#ifdef HMD_DEBUG
    hyu32 frameSP;
    hyu32 sp;
#endif
    Value d(HC_Class, (void*)this);
    MethodPtr methodPtr(d);
    Signature zeroSig(0, context);
    if (searchMethod(&methodPtr, HSymR_cppSize, &zeroSig)) {
        HMD_DEBUG_ASSERT(context != NULL);
#ifdef HMD_DEBUG
        frameSP = context->frameStack.size();
        sp = context->stack.size();
#endif
        if (! context->execMethod(Value::fromClass(this), HSymR_cppSize, 0))
            HMD_FATAL_ERROR("method %s::*cppSize execute failed", name());
        d = context->pop();
        if (d.type->symCheck(HSym_Int))
            cppFieldSize = d.data;
        else
            cppFieldSize = 0;
#ifdef HMD_DEBUG
        HMD_DEBUG_ASSERT(frameSP == context->frameStack.size());
        HMD_DEBUG_ASSERT(sp == context->stack.size());
#endif
    }
        
    //HMD__PRINTF_FK("createInstance class=%s objsize=%d+%d numArgs=%d\n",name(),fieldSize(),cppFieldSize,numArgs);
    Object* obj = createInstance_noInit(cppFieldSize);
        
    // インスタンスメソッドinitialize()の呼び出し
    if (context != NULL && bInit) {
        HMD_ASSERT(pSig != NULL);
#ifdef HMD_DEBUG
        frameSP = context->frameStack.size();
        sp = context->stack.size() - pSig->getNumOnStack();
#endif
        int er = context->execMethod_ifExist(Value::fromObj(obj), HSym_initialize, pSig);
        if (er == 1) {
            // 実行成功
            context->pop();    // initializeの戻り値を捨てる
        } else if (er == -1) {
            // 実行失敗：exceptionが投げられている
            return NULL;
        } else {
            // initialize()が無かった
            pSig->clearRight();
        }
        HMD_DEBUG_ASSERTMSG(frameSP == context->frameStack.size(),
                            M_M("frameSP=%d,frameStack.sp=%d"),
                            frameSP,context->frameStack.size());
        HMD_DEBUG_ASSERTMSG(context->stack.size() == sp,
                            M_M("stack size expected %d but actually %d"),
                            sp, context->stack.size());
    } else {
        HMD_ASSERT(pSig == NULL);
    }

    return obj;
}

Object* HClass::createInstance_noInit(size_t cppFieldSize) const
{
    Object* obj = Object::create(this, fieldSize() + sizeof(hyu32) + cppFieldSize);
    int nn = (int)m_membVarSymArr.size();
    Value* op = (Value*)obj->field(0);
	for (int i = nn; i > 0; --i) {
        *op++ = NIL_VALUE;
    }
	*(hyu32*)op = nn;

    return obj;
}


void HClass::changeSymbol(SymbolID_t sym)
{
    HMD_DEBUG_ASSERT(sym != SYMBOL_ID_ERROR);
    m_classSymbol = sym;
}

const hyu8* HClass::name(void) const
{
    return gSymbolTable.id2str(m_classSymbol);
}



bool HClass::doesInherit(const HClass* other) const
{
    for (hyu16 i = 0; i < m_numSuper; ++i) {
        const HClass* sup = m_super(i);
        if (sup == other)
            return true;
        if (sup->doesInherit(other))
            return true;
    }
    return false;
}




bool HClass::m_lookupRoutine(MethodPtr* pMethodPtr, SymbolID_t methodSymbol, Signature* pSig) const
{
    //HMD__PRINTF_FK("m_lookupRoutine(%s) of class %s\n",gSymbolTable.id2str(methodSymbol),name());
    int iMin, iMax;
    if (binarySearchRange<hyu16>(m_methodSymTable, m_numMethods, methodSymbol, &iMin, &iMax)) {


        const Signature::Sig_t* sigBytes;
        hyu16 defValOffs;
        for (int i = iMin; i <= iMax; ++i) {
            getMethodSignatureBytes(i, &sigBytes, &defValOffs);
            if (pSig->canSubstTo(sigBytes, this, defValOffs)) {
                pMethodPtr->setMethodAddr(this, (const hyu8*)m_methodTable[i], i+1);
                //HMD__PRINTF_FK("   --> found %d\n", n);
                return true;
            }
        }
    }

    return false;
}


bool HClass::haveMethod(SymbolID_t methodSymbol) const
{
    // バイトコードチェック
    int idx;
    if (binarySearch<hyu16>(m_methodSymTable, m_numMethods, methodSymbol, &idx))
        return true;

    // ffiチェック
    Ffi_t ffi = m_searchFfi(methodSymbol, NULL);
    if (ffi != NULL)
        return true;

    // 親クラスチェック
    for (int i = 0; i < m_numSuper; i++)
        if (m_super(i)->haveMethod(methodSymbol))
            return true;

    // Objectクラスチェック
    if (! symCheck(HSym_Object))
        if (HC_Object->haveMethod(methodSymbol))
            return true;

    return false;
}


void HClass::getMethodSignatureBytes(int methodNum, const Signature::Sig_t** pSigBytes, hyu16* pDefValOffs) const
{
    HMD_DEBUG_ASSERT(methodNum >= 0 && methodNum < m_numMethods + m_numClosures);
    const hyu8* pMethod = (const hyu8*) m_methodTable[methodNum];
    pMethod += *(hyu16*)pMethod;
    hyu16 signatureID = *(hyu16*)pMethod;
    *pSigBytes = m_pBytecode->getSignatureBytes(signatureID);
    *pDefValOffs = *(hyu16*)(pMethod + sizeof(hyu16));
}

bool HClass::m_lookupFfi(MethodPtr* pMethodPtr, SymbolID_t methodSymbol, Signature* pSig) const
{
    //HMD__PRINTF_FK("m_lookupFfi(%s) of class %s\n",gSymbolTable.id2str(methodSymbol),name());
    Ffi_t ffi = m_searchFfi(methodSymbol, pSig);

    //pMethodPtr->setLexicalScope(this);
    if (ffi != NULL) {
        pMethodPtr->setFfi(this, ffi);
        //HMD__PRINTF_FK("   --> found\n");
        return true;
    }

    return false;
}


bool HClass::m_lookupMethod(MethodPtr* pMethodPtr, SymbolID_t methodSymbol, Signature* pSig) const
{
    if (m_lookupRoutine(pMethodPtr, methodSymbol, pSig)) {
        return true;
    }
    if (m_lookupFfi(pMethodPtr, methodSymbol, pSig)) {
        return true;
    }
    return false;
}

bool HClass::searchMethod(MethodPtr* pMethodPtr, SymbolID_t methodSymbol, Signature* pSig) const
{
    if (m_lookupMethod(pMethodPtr, methodSymbol, pSig))
        return true;

    // 親クラス検索
    for (int i = 0; i < m_numSuper; i++) {
        if (m_super(i)->searchMethod(pMethodPtr, methodSymbol, pSig))
            return true;
    }

    // Object(全ての親クラス)のメソッド
    if (m_classSymbol != HSym_Object)
        if (HC_Object->m_lookupMethod(pMethodPtr, methodSymbol, pSig))
            return true;
        
    // usingパスから検索
    for (int i = 0; i < m_numUsingPaths; i++) {
        if (m_usingPaths[i]->searchMethod(pMethodPtr, methodSymbol, pSig))
            return true;
    }

    return false;
}


void HClass::setClosureMethodPtr(int num, MethodPtr* pmptr) const
{
    HMD_ASSERT((num >= 0) && (num < m_numClosures));
    pmptr->setMethodAddr(this, (const hyu8*)m_methodTable[m_numMethods + num], m_numMethods + num + 1);
}


Value* HClass::getClassVarAddr(SymbolID_t classVarSymbol) const
{
    //HMD__PRINTF_FK("getClassVarAddr class=%s, var=%s\n",name(),gSymbolTable.id2str(classVarSymbol));
    Value** vpp = m_classVarHash.find(classVarSymbol);
    if (vpp != NULL)
        return *vpp;
    return NULL;
}


Value* HClass::getConstVarAddr(SymbolID_t constVarSymbol) const
{
    // 自身から探す
    Value** vpp = m_constVarHash.find(constVarSymbol);
    if (vpp != NULL)
        return *vpp;

    // usingから探す
    for (int i = 0; i < m_numUsingPaths; ++i) {
        vpp = m_usingPaths[i]->m_constVarHash.find(constVarSymbol);
        if (vpp != NULL)
            return *vpp;
    }

    // outerから再帰的に探す
    if (m_outerClass != NULL) {
        Value* vp = m_outerClass->getConstVarAddr(constVarSymbol);
        if (vp != NULL)
            return vp;
    }

    return NULL;
}

Value* HClass::getMembVarAddr(Object* obj, SymbolID_t varSymbol) const
{
    int i;
    //HMD__PRINTF_FK("search ins var %s (class %s)\n", gSymbolTable.id2str(varSymbol), name());
    if (m_membVarSymArr.BSearch(varSymbol, &i)) {
        //HMD__PRINTF_FK(" -> %d\n", i);
        //HMD__PRINTF_FK("   obj=%x,var addr=%x\n", obj,obj->field(i * sizeof(Value)));
        return (Value*) obj->field(i * sizeof(Value));
    }
    //HMD__PRINTF_FK(" -> not found\n");
    return NULL;
}
    
Value* HClass::getDefaultValAddr(hyu16 defValIdx) const
{
    hyu32 offs = m_classVarSymArr.size() + m_constVarSymArr.size();
#ifdef HMD_DEBUG
    HMD_DEBUG_ASSERT(m_defaultValSetLimit <= m_numDefaultVal);
    if (defValIdx < m_defaultValSetLimit)
        return &m_varPool[offs + defValIdx];
#else
    if (defValIdx < m_numDefaultVal)
        return &m_varPool[offs + defValIdx];
#endif
    return NULL;
}

void HClass::setDefaultVal(hyu16 defValIdx, const Value& val)
{
    HMD_ASSERT(defValIdx < m_numDefaultVal);
    hyu32 offs = m_classVarSymArr.size() + m_constVarSymArr.size();
    m_varPool[offs + defValIdx] = val;
#ifdef HMD_DEBUG
    if (defValIdx >= m_defaultValSetLimit)
        m_defaultValSetLimit = defValIdx + 1;
#endif
}



void HClass::m_resolveTable(const HClass* outer)
{
    // コンパイラとの整合性チェック
    HMD_DEBUG_ASSERT_EQUAL((size_t)4, sizeof(Object*));
    HMD_DEBUG_ASSERT_EQUAL((size_t)8, sizeof(Value));

    m_outerClass = outer;
    //HMD_PRINTF("resolve: %s(%x) outer=%s(%x)\n",name(),this,(outer==NULL)?"NULL":(const char*)outer->name(),outer);
    const HClass* base;
    if (outer == NULL)
        base = m_pBytecode->mainClass();
    else
        base = outer;

    // m_superTable の書換え
    for (int i = 0; i < m_numSuper; i++) {
        const HClass* cls = base;
        SymbolID_t supsym = (SymbolID_t)Endian::unpack<hyu32>((const hyu8*)&m_superTable[i]);
        if (cls != this) {
            cls = cls->getScopeClass(supsym);
        } else {
            // m_superTableまだ構築中なので getScopeClass() が使えない
            cls = NULL;
            for (int j = 0; j < i; ++j) {
                if (m_super(j)->m_classSymbol == supsym) {
                    cls = m_super(j);
                    break;
                }
            }
            if (cls == NULL) {
                for (int j = 0; j < i; ++j) {
                    cls = m_super(j)->getScopeClass(supsym, true);
                    if (cls != NULL)
                        break;
                }
            }
            if (cls == NULL) {
                const HClass* oc = m_outerClass;
                while (oc != NULL && oc != this) {
                    cls = oc->getScopeClass(supsym);
                    if (cls != NULL)
                        break;
                    oc = oc->m_outerClass;
                }
            }

            cls = m_pBytecode->searchClass(supsym);
        }
        HMD_ASSERTMSG(cls != NULL, M_M("super class %s not found"), gSymbolTable.id2str((SymbolID_t)m_superTable[i]));
        m_superTable[i] = (hyu32)cls;
    }

    // m_usingPaths の書換え
    m_numUsingPaths = 0;
    const hyu8* ptr = ((const hyu8*)m_usingPaths) + 4;
    for (;;) {
        SymbolID_t sym = Endian::unpackP<SymbolID_t>(&ptr);
        if (sym == SYMBOL_ID_ERROR)
            break;

        const HClass* cls;
        if (sym == HSym_nil) {
            sym = Endian::unpackP<SymbolID_t>(&ptr);
            cls = m_pBytecode->searchClass(sym);
            HMD_ASSERTMSG(cls != NULL, M_M("using class ::%s not found"), gSymbolTable.id2str(sym));
        } else {
            cls = getScopeClass(sym, true);
            HMD_ASSERTMSG(cls != NULL, M_M("using class %s not found"), gSymbolTable.id2str(sym));
        }
        for (;;) {
            sym = Endian::unpackP<SymbolID_t>(&ptr);
            if (sym == SYMBOL_ID_ERROR)
                break;
            cls = cls->getInnerClass(sym);
            HMD_ASSERTMSG(cls != NULL, M_M("using class ...::%s not found"), gSymbolTable.id2str(sym));
        }
        m_usingPaths[m_numUsingPaths++] = const_cast<HClass*>(cls);
    }
    HMD_DEBUG_ASSERT(ptr > (const hyu8*)&m_usingPaths[m_numUsingPaths]);

    // innerClassについて再帰
    for (int i = 0; i < m_numInnerClasses; i++) {
        ((HClass*)m_innerClassTable[i])->m_resolveTable(this);
    }


    // クラス変数、定数のハッシュ構築
    m_initHashes();
}


void HClass::m_initHashes(void)
{
    // クラス変数ハッシュ
    for (int i = 0; i < m_numSuper; i++) {
        m_classVarHash.copyFrom(const_cast<HClass*>(m_super(i))->m_classVarHash);
    }
    int size = (int) m_classVarSymArr.size();
    for (int i = 0; i < size; ++i) {
        SymbolID_t varSym = m_classVarSymArr[i];
        if (NULL == m_classVarHash.find(varSym))
            m_classVarHash[varSym] = &m_varPool[i];
    }

    // 定数ハッシュ
    for (int i = 0; i < m_numSuper; i++) {
        m_constVarHash.copyFrom(const_cast<HClass*>(m_super(i))->m_constVarHash);
    }
    size = m_constVarSymArr.size();
    for (int i = 0; i < size; ++i) {
        SymbolID_t varSym = m_constVarSymArr[i];
        if (NULL == m_constVarHash.find(varSym))
            m_constVarHash[varSym] = &m_varPool[i + m_classVarSymArr.size()];
    }
}



void HClass::recSetNodeFlag(hyu8 val)
{
    m_nodeFlag = val;
    for (int i = 0; i < (int)m_numInnerClasses; ++i) {
        HClass* inner = (HClass*) m_innerClassTable[i];
        inner->recSetNodeFlag(val);
    }
}


//======================================================================
//
//          バイトコードリロード
//
//======================================================================


#ifdef HY_ENABLE_BYTECODE_RELOAD

// 新クラスで無くなった変数も保持し続けるように内部テーブルを変更する。
// 初期化コード実行前に呼ぶべし
void HClass::keepObsoleteVar(const HClass* oldClass)
{
    // 旧変数シンボルをマージ
    m_classVarSymArr.BMerge(oldClass->m_classVarSymArr);
    m_constVarSymArr.BMerge(oldClass->m_constVarSymArr);
    m_membVarSymArr.BMerge(oldClass->m_membVarSymArr);

    int numPoolVar = m_classVarSymArr.size() + m_constVarSymArr.size() + m_numDefaultVal;
    m_varPool = gMemPool->reallocT<Value>(m_varPool, numPoolVar);
    for (int i = 0; i < numPoolVar; i++)
        m_varPool[i] = NIL_VALUE;


    // クラス変数ハッシュと定数ハッシュを再構築
    m_classVarHash.finalize();
    m_constVarHash.finalize();
    m_initHashes();

    for (int i = 0; i < (int)m_numInnerClasses; ++i) {
        HClass* inner = (HClass*) m_innerClassTable[i];
        SymbolID_t innerClassSym = inner->getSymbol();
        HClass* oldInner = const_cast<HClass*>(oldClass->getInnerClass(innerClassSym));
        if (oldInner != NULL)
            inner->keepObsoleteVar(oldInner);
    }
}


static void s_copyVarData(BMap<SymbolID_t, Value*>& src, BMap<SymbolID_t, Value*>& dst)
{
    TArray<SymbolID_t>& srcKeys = src.keys();
    for (hys32 n = (hys32) srcKeys.size() - 1; n >= 0; --n) {
        SymbolID_t s = srcKeys[n];
        Value** vp = dst.find(s);
        if (vp != NULL) {
            //HMD_PRINTF("overwrite var %s data addr %x <-- %x\n",gSymbolTable.id2str(s),*vp,src[s]);
            **vp = *(src[s]);
        }
    }
}
// クラス変数、定数の値を新クラスにコピーする
// 旧クラス→新クラスの対応表をpGenMapにストアする
void HClass::copyDataTo(HClass* dest, BMap<const HClass*,const HClass*>* pGenMap, bool bCopyClassVar, bool bCopyConstVar)
{
    //HMD_PRINTF("reload: copy data (%x)%s -> (%x)%s  (%d,%d)\n",this,name(),dest,dest->name(),bCopyClassVar,bCopyConstVar);
    (*pGenMap)[this] = dest;
    m_nodeFlag = 1;
    dest->m_nodeFlag = 1;

    if (bCopyClassVar)
        s_copyVarData(m_classVarHash, dest->m_classVarHash);
    if (bCopyConstVar)
        s_copyVarData(m_constVarHash, dest->m_constVarHash);
    for (int i = 0; i < (int)m_numInnerClasses; ++i) {
        HClass* inner = (HClass*) m_innerClassTable[i];
        SymbolID_t innerClassSym = inner->getSymbol();
        HClass* dstInner = const_cast<HClass*>(dest->getInnerClass(innerClassSym));
        if (dstInner != NULL)
            inner->copyDataTo(dstInner, pGenMap, bCopyClassVar, bCopyConstVar);
    }
}

// スーパークラスがリロードされていたら調整する
void HClass::regulateReloadedSuper(BMap<const HClass*,const HClass*>& genMap)
{
    if (m_nodeFlag != 0)
        return;                 // 調整済み

    for (int i = 0; i < m_numSuper; i++) {
        HClass* sup = (HClass*)m_superTable[i];
        const HClass** pNewSup = genMap.find(sup);
        if (pNewSup != NULL) {
            m_superTable[i] = (hyu32)*pNewSup;
        } else {
            sup->regulateReloadedSuper(genMap);
        }
    }

    m_nodeFlag = 1;

    // クラス変数ハッシュと定数ハッシュを再構築
    m_classVarHash.finalize();
    m_constVarHash.finalize();
    m_initHashes();

    // インナークラスも調整
    for (int i = 0; i < (int)m_numInnerClasses; ++i) {
        HClass* inner = (HClass*) m_innerClassTable[i];
        inner->regulateReloadedSuper(genMap);
    }
}

#endif



//======================================================================
//
//          Ffi
//
//======================================================================


GC::MarkFunc_t HClass::getGCMarkFunc(void) const
{
    if (m_ffiTable == NULL)
        return NULL;
    return m_ffiTable->GCMarkFunc;
}

Ffi_t HClass::m_searchFfi(SymbolID_t methodSymbol, Signature* pSig) const
{
    if (m_ffiTable == NULL)
        return NULL;
    const FfiMethodEntry_t* pSysMethodTable = m_ffiTable->ffiMethodTable;
    int i = 0;
    int j = m_ffiTable->numFfis;
    // binary search
    while (i < j) {
        int k = (i + j) / 2;
        SymbolID_t kSym = pSysMethodTable[k].methodSymbol;
        if (kSym == methodSymbol) {
            // 1 found.
            if (pSig == NULL)
                return pSysMethodTable[k].func;
            // search around to find ffi that have same methodSymbol
            i = k - 1;
            while ((i >= 0) && (pSysMethodTable[i].methodSymbol == methodSymbol))
                --i;
            j = k + 1;
            while ((j < m_ffiTable->numFfis) && (pSysMethodTable[j].methodSymbol == methodSymbol))
                ++j;
            ++i; --j;
            // range found. [i, j]

            for ( ; i <= j; ++i) {
                hyu16 sigID = pSysMethodTable[i].signatureID;
                hyu16 defValOffs = pSysMethodTable[i].defaultValueOffset;
                const Signature::Sig_t* sigBytes = m_pBytecode->getSignatureBytes(sigID);
                if (pSig->canSubstTo(sigBytes, this, defValOffs)) {
                    return pSysMethodTable[i].func;
                }
            }

            return NULL;        // signature mismatch

        }
        if (kSym < methodSymbol) {
            i = k + 1;
        } else {
            j = k;
        }
    }
    return NULL;
}

void HClass::initializeRootFfiTable(void)
{
    m_rootFfiTable.initialize(0);
}

void HClass::finalizeRootFfiTable(void)
{
    m_rootFfiTable.finalize();
}


static int cmpCSTsym(const HClass::ClassFfiEntry_t*& ent, SymbolID_t& sym)
{
    return (int)(ent->classSymbol) - (int)sym;
}
const HClass::ClassFfiEntry_t* HClass::findFfiTable(SymbolID_t bytecodeSymbol)
{
    int idx;
    if (binarySearchFn<const ClassFfiEntry_t*, SymbolID_t>(m_rootFfiTable.top(), (int)m_rootFfiTable.size(), cmpCSTsym, bytecodeSymbol, &idx)) {
        return m_rootFfiTable[idx];
    }
    return NULL;
}

void HClass::linkRootFfiTable(const HClass::ClassFfiEntry_t* tbl)
{
    int idx;
    SymbolID_t bcSym = tbl->classSymbol;
    if (binarySearchFn<const ClassFfiEntry_t*, SymbolID_t>(m_rootFfiTable.top(), (int)m_rootFfiTable.size(), cmpCSTsym, bcSym, &idx)) {
        HMD_ERRPRINTF("ffi table of %s has been linked already\n", gSymbolTable.id2str(tbl->classSymbol));
    } else {
        m_rootFfiTable.insert(idx) = tbl;
    }
}

void HClass::bindFfiTable(const ClassFfiEntry_t* tbl)
{
    //HMD_PRINTF("bind class '%s' FFI table to %x\n",name(),tbl);
    m_ffiTable = tbl;
    if (tbl == NULL)
        return;
    for (hyu16 i = 0; i < tbl->numInnerClasses; ++i) {
        const ClassFfiEntry_t* intbl = tbl->innerClassFfiTable[i];
        HClass* inner = const_cast<HClass*>(getInnerClass(intbl->classSymbol));
        HMD_ASSERTMSG(inner != NULL,
                      M_M("ffi table mismatch: table has entry '%s' but class %s does not have it"),
                      gSymbolTable.id2str(intbl->classSymbol),
                      name());
        inner->bindFfiTable(intbl);
    }
}



//======================================================================
//
//          GC
//
//======================================================================

void HClass::m_GC_mark_membVar(Object* obj) const
{
    HMD_DEBUG_ASSERT(obj->type() == this);
    Value* pv = (Value*) obj->field(0);
    int i = m_membVarSymArr.size();
    while (i-- > 0) {
        HMD__PRINTF_GC("mark membVar %x (class %s) <%s,%x>\n",pv,name(),pv->getType()->name(),pv->data);
        GC::markValue(*pv++);
    }
}

void HClass::m_GC_mark_staticVar(void)
{
    hyu32 num = m_classVarSymArr.size() + m_constVarSymArr.size();
    HMD__PRINTF_GC("GC mark class %x %s classvar+constvar num=%d\n", this, name(),num);
    if (m_varPool != NULL) {
        Value* pv = m_varPool;
        for (int i = num + m_numDefaultVal; i > 0; --i) {
            // HMD__PRINTF_GC("GC mark classvar %x\n", pv);
            GC::markValue(*pv++);
            --GC::m_countdown_inc;
        }
    }
        
    // inner class
    for (int i = 0; i < m_numInnerClasses; i++) {
        ((HClass*)m_innerClassTable[i])->m_GC_mark_staticVar();
    }
}

#ifdef HMD_DEBUG
void HClass::debugPrintAbsName(void) const
{
    if (m_outerClass != NULL) {
        m_outerClass->debugPrintAbsName();
        HMD_PRINTF("::%s",name());
    } else {
        HMD_PRINTF("%s",name());        // top level
    }
}
#endif
