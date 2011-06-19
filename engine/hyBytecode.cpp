/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyMemPool.h"
#include "hyBytecode.h"
#include "hyCodePtr.h"
#include "hyEndian.h"
#include "hyObject.h"
#include "hyContext.h"
#include "hyCodeManager.h"
#include "hyStringBuffer.h"
#include "HSymbol.h"
#include "hySymbolTable.h"
#include "hyDebugSourceInfo.h"
#include "hyVM.h"

#ifdef HMD_DEBUG
extern "C" {
#include <stdio.h>
#include <string.h>
}
#endif


using namespace Hayat::Common;
using namespace Hayat::Engine;


#ifdef HMD_DEBUG
# ifdef HMD_DEBUG_INFO_LOAD
bool Bytecode::m_bReadDebugInf = true;
# else
bool Bytecode::m_bReadDebugInf = false;
# endif
#endif



// このテーブルは、 hyCSignature.cpp の事前登録シグネチャと
// 一致していなければならない
static const hyu8 BASIC_SIGNATURE_TABLE[] = {
    Signature::NESTNUM_1, 0,
    Signature::NESTNUM_1, 1,
    Signature::NESTNUM_1, 2,
    Signature::NESTNUM_1, 3,
    Signature::NESTNUM_1, 4,
    Signature::NESTNUM_1, 5,
    Signature::NESTNUM_1, 6,
    Signature::NESTNUM_1, 7,
    Signature::NESTNUM_1, 8,
    Signature::NESTNUM_1, Signature::MULTI, // (*)
    Signature::NESTNUM_1, Signature::MULTI2, // (...)
    Signature::NESTNUM_1, Signature::DEFVAL, // ($0)
    Signature::NESTNUM_2, 1, Signature::DEFVAL, // (1,$0)
    Signature::NESTNUM_2, 2, Signature::DEFVAL, // (2,$0)
    Signature::NESTNUM_2, 3, Signature::DEFVAL, // (3,$0)
    Signature::NESTNUM_2, 1, Signature::MULTI, // (1,*)
    Signature::NESTNUM_2, 2, Signature::MULTI, // (2,*)
    Signature::NESTNUM_2, 3, Signature::MULTI, // (3,*)
    Signature::NESTNUM_2, Signature::DEFVAL, Signature::DEFVAL+1, // ($0,$1)
};
static const int NUM_BASIC_SIGNATURE = 12+7;
static const hyu32 BASIC_SIGNATURE_TABLE_OFFS[NUM_BASIC_SIGNATURE+1] = {
    0,2,4,6,8,10,12,14,16,18,20,22,
    24,27,30,33,36,39,42,45
};
static const hyu8 BASIC_SIGNATURE_ARITY_TABLE[NUM_BASIC_SIGNATURE] = {
    0,1,2,3,4,5,6,7,8,1,1,1,
    2,3,4,2,3,4,2
};




Bytecode::Bytecode()
    : m_fileBuf(NULL),
      m_mainClass(NULL),
      m_bytecodeBuf(NULL),
      m_bytecodeSize(0),
      m_bytecodeEndAddr(NULL),
      m_stringTableSize(0),
      m_stringTable(NULL),      
      m_signatureTableNum(0),
      m_signatureTableOffs(NULL),
      m_signatureTable(NULL),
      m_signatureArityTable(NULL),
      m_mySymbol(0),
      m_numLinkBytecodes(0),
      m_linkBytecodes(NULL),
      m_initializedFlag(false)
#ifdef HY_ENABLE_BYTECODE_RELOAD
    , m_generationMap(0)
#endif
#ifdef HMD_DEBUG
    , m_debugInfos()
#endif
{
}

Bytecode::~Bytecode()
{
    finalize();
}

void* Bytecode::operator new(size_t size)
{
    return (Bytecode*) gMemPool->alloc(size, "BYTC");
}
    
void Bytecode::operator delete(void* p)
{
    gMemPool->free(p);
}

bool Bytecode::readFile(const char* fname, void (*loadedCallback)(const char*))
{
    static const char* BYTECODE_EXT = ".hyb";
#ifdef HMD_DEBUG
    static const char* DEBINFO_EXT = ".hdb";
#endif
    char* fn;
    size_t len = HMD_STRLEN(fname);
    fn = (char*)HMD_ALLOCA(len + 8);
    HMD_STRNCPY(fn, fname, len+1);
    if (0 != HMD_STRCMP(fname + len - 4, BYTECODE_EXT)) {
        HMD_STRSCAT(fn, BYTECODE_EXT, len+8);
        len += HMD_STRLEN(BYTECODE_EXT);
    }
    char* pathBuf = NULL;
    hyu32 length = hmd_loadFile_inPath(fn, &m_fileBuf, &pathBuf, gMemPool);
    if (length == 0) {
        return false;
    }
        
    if (loadedCallback != NULL)
        loadedCallback((const char*)pathBuf);

    setBytecode((const hyu8*)m_fileBuf, length);


#ifdef HMD_DEBUG
    if (m_bReadDebugInf) {
        HMD_STRNCPY(fn + len - 4, DEBINFO_EXT, 5);
        m_debugInfos.readDebugInfo(fn);
    }
#endif

    gMemPool->free(pathBuf);
    return true;
}

void Bytecode::takeBytecode(const hyu8* buf, hyu32 size)
{
    m_fileBuf = hmd_allocFileBuf(size);
    memcpy(m_fileBuf, buf, size);
    setBytecode((const hyu8*)m_fileBuf, size);
}
    
void Bytecode::setBytecode(const hyu8* buf, hyu32 size)
{
    m_bytecodeBuf = buf;
    m_bytecodeSize = size;
    m_bytecodeEndAddr = m_bytecodeBuf + m_bytecodeSize;

    m_numLinkBytecodes = Endian::unpack<hyu16>(m_bytecodeBuf);
    m_mySymbol = Endian::unpack<SymbolID_t>(m_bytecodeBuf + 4);
#ifdef HMD_DEBUG
    m_debugInfos.setBytecodeSymbol(m_mySymbol);
#endif
    m_linkBytecodes = (const SymbolID_t*)(m_bytecodeBuf + 4 + sizeof(SymbolID_t));
    for (int i = 0; i < m_numLinkBytecodes; i++) {
        // endian書換え
        Endian::rewrite<SymbolID_t>(const_cast<SymbolID_t*>(&m_linkBytecodes[i]));
    }
    const hyu8* p = (const hyu8*)(m_linkBytecodes + m_numLinkBytecodes);
    alignPtr<4>(&p);
    hyu32 linkBytecodeNameTableSize = Endian::unpackP<hyu32>(&p);
    const hyu8* linkBytecodeName = p;
    p += alignInt<4>(linkBytecodeNameTableSize);

    m_stringTableSize = Endian::unpackP<hys32>(&p);
    m_stringTable = p;
    p += alignInt<4>(m_stringTableSize);
    HMD_ASSERTMSG(p[-1] == 0, M_M("string table not terminated"));

    m_signatureTableNum = Endian::unpackP<hyu32>(&p);
    m_signatureTableOffs = (const hyu32*)p;
    p += sizeof(hyu32) * m_signatureTableNum;
    hyu32 stblsize = Endian::unpackP<hyu32>(&p);
    m_signatureTable = (const hyu8*)p;
    p += alignInt<4>(stblsize);
    m_signatureArityTable = (const hyu8*)p;
    p += alignInt<4>(m_signatureTableNum);
    
    m_mainClass = new HClass(bytecodeSymbol(), this, p);

    // ロードされていないリンクバイトコードがあればロードする
    for (int i = 0; i < m_numLinkBytecodes; i++) {
        if (gCodeManager.require(m_linkBytecodes[i], linkBytecodeName) == NULL) {
            HMD_FATAL_ERROR("cant load required bytecode '%s'\n", linkBytecodeName);
        }
        linkBytecodeName += HMD_STRLEN((const char*)linkBytecodeName) + 1;
    }

    const_cast<HClass*>(m_mainClass)->m_resolveTable(NULL);
    const_cast<HClass*>(m_mainClass)->bindFfiTable(HClass::findFfiTable(bytecodeSymbol()));
}
    
void Bytecode::finalize(void)
{
    //HMD_PRINTF("Bytecode::finalize()  %x\n",this);
    delete m_mainClass;
    m_mainClass = NULL;
    hmd_freeFileBuf(m_fileBuf);
    m_fileBuf = NULL;
    m_bytecodeBuf = NULL;
#ifdef HY_ENABLE_BYTECODE_RELOAD
    m_generationMap.finalize();
#endif
    // バイトコードが無くなるのでopcodeは取れなくなる
    VM::clearOpcodeHistory();
}

void Bytecode::initLinkBytecode(Context* context, bool bExec)
{
    for (int i = 0; i < m_numLinkBytecodes; i++) {
        Bytecode* pBytecode = gCodeManager.getBytecode(m_linkBytecodes[i]);
        pBytecode->initLinkBytecode(context, true);
    }
    gCodeManager.setWorkingBytecode(this);
    if (! m_initializedFlag) {
        //HMD_PRINTF("init bytecode %s (%s)\n",gSymbolTable.id2str(bytecodeSymbol()),bExec ? "true" : "false");
        if (bExec) {
            if (context->execBytecodeTop(this)) {
                context->pop();         // 戻り値は捨てる
                m_initializedFlag = true;
            } else {
                HMD_FATAL_ERROR("error on initializing bytecode %s",
                                gSymbolTable.id2str(bytecodeSymbol()));
            }
        }
    }
}

void Bytecode::initCodePtr(CodePtr* ptr) const
{
    ptr->initialize(m_mainClass->classInitializer());
}


void Bytecode::changeBytecodeSymbol(SymbolID_t asName)
{
    if (asName == SYMBOL_ID_ERROR)
        return;
    m_mySymbol = asName;
    const_cast<HClass*>(m_mainClass)->changeSymbol(asName);
#ifdef HMD_DEBUG
    m_debugInfos.setBytecodeSymbol(asName);
#endif
}

// リンクバイトコードも含めてクラスを検索する
const HClass* Bytecode::searchClass(SymbolID_t classSym) const
{
    //HMD__PRINTF_FK("search class %s\n", gSymbolTable.id2str(classSym));
    gCodeManager.clearAllFlag();
    return m_searchClass(classSym);
}

const HClass* Bytecode::m_searchClass(SymbolID_t classSym) const
{    
    const HClass* pClass;
    const Bytecode* pBytecode;
    //HMD__PRINTF_FK("search in bytecode %s\n", gSymbolTable.id2str(m_mySymbol));
    // 自バイトコードを検索
    pClass = m_mainClass->getInnerClass(classSym);
    if (pClass != NULL) return pClass;
    gCodeManager.setFlag(m_mySymbol); // 自バイトコードは検索終了
    // リンクバイトコードを検索
    for (int i = m_numLinkBytecodes - 1; i >= 0; i--) {
        if (! gCodeManager.getFlag(m_linkBytecodes[i])) {
            pBytecode = gCodeManager.getBytecode(m_linkBytecodes[i]);
            if (pBytecode != NULL) {
                pClass = pBytecode->m_searchClass(classSym);
                if (pClass != NULL) return pClass;
            } else {
                HMD_FATAL_ERROR(M_M("no link bytecode found %s"),gSymbolTable.id2str(m_linkBytecodes[i]));
            }
            gCodeManager.setFlag(m_linkBytecodes[i]); // このバイトコード検索終了
        }
    }
    // 無かった
    //HMD__PRINTF_FK(" not found\n");
    return NULL;
}

bool Bytecode::searchMethod(MethodPtr* pMethodPtr, SymbolID_t methodSym, Signature* pSig) const
{
    //HMD__PRINTF_FK("search method %s\n", gSymbolTable.id2str(methodSym));
    gCodeManager.clearAllFlag();
    return m_searchMethod(pMethodPtr, methodSym, pSig);
}

bool Bytecode::m_searchMethod(MethodPtr* pMethodPtr, SymbolID_t methodSym, Signature* pSig) const
{
    //HMD__PRINTF_FK("search in bytecode %s\n", gSymbolTable.id2str(m_mySymbol));
    const Bytecode* pBytecode;
    Value mainKlass(HC_Class, (void*)m_mainClass);
    pMethodPtr->setSelf(mainKlass);
    if (m_mainClass->m_lookupMethod(pMethodPtr, methodSym, pSig))
        return true;

    gCodeManager.setFlag(m_mySymbol); // 自バイトコードは検索終了
    // リンクバイトコードを検索
    for (int i = m_numLinkBytecodes - 1; i >= 0; i--) {
        if (! gCodeManager.getFlag(m_linkBytecodes[i])) {
            pBytecode = gCodeManager.getBytecode(m_linkBytecodes[i]);
            if (pBytecode != NULL) {
                if (pBytecode->m_searchMethod(pMethodPtr, methodSym, pSig))
                    return true;
            } else {
                HMD_FATAL_ERROR(M_M("no link bytecode found %s"),gSymbolTable.id2str(m_linkBytecodes[i]));
            }
            gCodeManager.setFlag(m_linkBytecodes[i]); // このバイトコード検索終了
        }
    }
    // 無かった
    //HMD__PRINTF_FK(" not found\n");
    return false;
}


// リンクバイトコードも含めてトップレベル定数を検索
Value* Bytecode::getConstVarAddr(SymbolID_t varSym) const
{
    gCodeManager.clearAllFlag();
    return m_getConstVarAddr(varSym);
}

Value* Bytecode::m_getConstVarAddr(SymbolID_t varSym) const
{
    const Bytecode* pBytecode;
    Value* addr;
    //HMD__PRINTF_FK("search const var '%s' in bytecode %s\n", gSymbolTable.id2str(varSym),gSymbolTable.id2str(m_mySymbol));
    // 自バイトコードを検索
    addr = m_mainClass->getConstVarAddr(varSym);
    if (addr != NULL)
        return addr;
    gCodeManager.setFlag(m_mySymbol); // 自バイトコードは検索終了
    // リンクバイトコードを検索
    for (int i = m_numLinkBytecodes - 1; i >= 0; i--) {
        if (! gCodeManager.getFlag(m_linkBytecodes[i])) {
            pBytecode = gCodeManager.getBytecode(m_linkBytecodes[i]);
            if (pBytecode != NULL) {
                addr = pBytecode->m_getConstVarAddr(varSym);
                if (addr != NULL) return addr;
            } else {
                HMD_FATAL_ERROR(M_M("no link bytecode found %s"),gSymbolTable.id2str(m_linkBytecodes[i]));
            }
            gCodeManager.setFlag(m_linkBytecodes[i]); // このバイトコード検索終了
        }
    }
    // 無かった
    //HMD__PRINTF_FK(" not found\n");
    return NULL;
}

const hyu8* Bytecode::getString(hyu32 offs) const
{
    HMD_ASSERTMSG(offs < m_stringTableSize, M_M("offs=%d m_stringTableSize=%d"),offs,m_stringTableSize);
    return m_stringTable + offs;
}

const Signature::Sig_t* Bytecode::getSignatureBytes(hyu16 id) const
{
    HMD_ASSERTMSG(id < m_signatureTableNum + NUM_BASIC_SIGNATURE, "id=%d m_signatureTableNum=%d",id,m_signatureTableNum);
    if (id < NUM_BASIC_SIGNATURE)
        return (const Signature::Sig_t*) (BASIC_SIGNATURE_TABLE + BASIC_SIGNATURE_TABLE_OFFS[id]);
    id -= NUM_BASIC_SIGNATURE;
    return (const Signature::Sig_t*) (m_signatureTable + m_signatureTableOffs[id]);
}
hyu32 Bytecode::getSignatureBytesLen(hyu16 id) const
{
    HMD_ASSERTMSG(id < m_signatureTableNum + NUM_BASIC_SIGNATURE, "id=%d m_signatureTableNum=%d",id,m_signatureTableNum);
    if (id < NUM_BASIC_SIGNATURE)
        return BASIC_SIGNATURE_TABLE_OFFS[id+1] - BASIC_SIGNATURE_TABLE_OFFS[id];
    id -= NUM_BASIC_SIGNATURE;
    return m_signatureTableOffs[id+1] - m_signatureTableOffs[id];
}
hyu8 Bytecode::getSignatureArity(hyu16 id) const
{
    HMD_ASSERTMSG(id < m_signatureTableNum + NUM_BASIC_SIGNATURE, "id=%d m_signatureTableNum=%d",id,m_signatureTableNum);
    if (id < NUM_BASIC_SIGNATURE)
        return BASIC_SIGNATURE_ARITY_TABLE[id];
    id -= NUM_BASIC_SIGNATURE;
    return m_signatureArityTable[id];
}


void Bytecode::m_GC_mark_staticVar(void)
{
    if (m_mainClass != NULL) {
        const_cast<HClass*>(m_mainClass)->m_GC_mark_staticVar();
    }
}


#ifdef HY_ENABLE_BYTECODE_RELOAD

void Bytecode::copyClassDataTo(Bytecode* dest, bool bCopyClassVar, bool bCopyConstVar)
{
    HClass* srcClass = const_cast<HClass*>(m_mainClass);
    HClass* dstClass = const_cast<HClass*>(dest->m_mainClass);
    srcClass->copyDataTo(dstClass, &m_generationMap, bCopyClassVar, bCopyConstVar);
}

#endif

void Bytecode::debugGetInfo(StringBuffer* sb, const hyu8* ptr) const
{
    char buf[512];
    debugGetInfo(buf, 512, ptr);
    hyu32 len = HMD_STRLEN(buf);
    sb->concat(buf, len);
}

#ifdef HMD_DEBUG

void Bytecode::debugGetInfo(char* buf, hyu32 bufSize, const hyu8* ptr) const
{
    hyu32 pos;
    if (isInCodeAddr(ptr))
        pos = ptr - m_bytecodeBuf;
    else
        pos = 0;
    m_debugInfos.debugGetInfo(buf, bufSize, pos);
}

#else

void Bytecode::debugGetInfo(char* buf, hyu32 bufSize, const hyu8* ptr) const
{
    hyu32 pos;
    if (isInCodeAddr(ptr))
        pos = ptr - m_bytecodeBuf;
    else
        pos = 0;
    HMD_SNPRINTF(buf, bufSize,
                 "%s:: %8x",
                 gSymbolTable.id2str(m_mySymbol),
                 pos);
}

#endif
