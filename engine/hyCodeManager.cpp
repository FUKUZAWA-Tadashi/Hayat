/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCodeManager.h"
#include "hyBytecode.h"
#include "hyMemPool.h"
#include "hyVM.h"
#include "hyTuning.h"
#include "hyDebug.h"
#include "hySymbolTable.h"
#include <string.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;

CodeManager     Hayat::Engine::gCodeManager;


CodeManager::CodeManager(void)
    : m_contexts(0), m_loadedCallback(NULL), m_tbl(0), m_replaced(0),
      m_ov_keepObsoleteVar(false),
      m_ov_copyClassVar(true), m_ov_copyConstVar(true),
      m_workingBytecode(NULL)
{
}
CodeManager::~CodeManager()
{
    finalize();
}

void CodeManager::initialize(void)
{
}

void CodeManager::finalize(void)
{
    TArray<tbl_st>& tbls = m_tbl.values();
    for (hys32 i = (hys32)tbls.size() - 1; i >= 0; --i) {
        tbl_st* pTbl = tbls.nthAddr(i);
        if (pTbl->autoloadFlag)
            delete pTbl->bytecode;
    }
    m_tbl.finalize();

    for (hys32 i = (hys32)m_replaced.size() - 1; i >= 0; --i) {
        delete m_replaced[i].bytecode;
    }
    m_replaced.finalize();

    for (int i = m_contexts.size() - 1; i >= 0; --i) {
        Context* context = m_contexts[i];
        if (context != NULL)
            delete context;
    }

    m_contexts.finalize();
    m_stringBox.finalize();

    m_workingBytecode = NULL;
}

Bytecode* CodeManager::readBytecode(const char* filename)
{
    Bytecode* pBytecode = new Bytecode();
    if (! pBytecode->readFile(filename, m_loadedCallback)) {
        delete pBytecode;
        return NULL;
    }
    addBytecode(pBytecode, true);
    return pBytecode;
}
void CodeManager::addBytecode(Bytecode* pBytecode, bool bAutoload)
{
    SymbolID_t sym = pBytecode->bytecodeSymbol();

    tbl_st* pTbl = m_searchTbl(sym);
    if (pTbl != NULL) {
        // そのバイトコートは既にある
        HMD_DEBUG_ASSERT(pTbl->bytecode == pBytecode);
        return;
    }

    pTbl = &m_tbl[sym];
    pTbl->bytecode = pBytecode;
    pTbl->autoloadFlag = bAutoload;
}



Bytecode* CodeManager::getBytecode(SymbolID_t bytecodeSym)
{
    tbl_st* pTbl = m_searchTbl(bytecodeSym);
    if (pTbl == NULL)
        return NULL;
    return pTbl->bytecode;
}

void CodeManager::deleteBytecode(Bytecode* pBytecode)
{
    SymbolID_t bcSym = pBytecode->bytecodeSymbol();
    tbl_st* pTbl = m_searchTbl(bcSym);
    if (pTbl == NULL) {
        HMD_ERRPRINTF("CodeManager::deleteBytecode(%x) no such bytecode", pBytecode);
        return;
    }

    if (pTbl->autoloadFlag)
        delete pBytecode;

    m_tbl.remove(bcSym);

    if (m_workingBytecode == pBytecode)
        m_workingBytecode = NULL;
}


void CodeManager::clearAllFlag(void)
{
    TArray<tbl_st>& tbls = m_tbl.values();
    for (hys32 i = (hys32)tbls.size() - 1; i >= 0; --i)
        tbls[i].flag = false;
}
    
void CodeManager::setFlag(SymbolID_t bytecodeSym)
{
    tbl_st* pTbl = m_searchTbl(bytecodeSym);
    if (pTbl != NULL)
        pTbl->flag = true;
}
    
bool CodeManager::getFlag(SymbolID_t bytecodeSym)
{
    tbl_st* pTbl = m_searchTbl(bytecodeSym);
    HMD_ASSERT(pTbl != NULL);
    return pTbl->flag;
}

Bytecode* CodeManager::require(SymbolID_t bytecodeSym, const hyu8* bytecodeName)
{
    //HMD__PRINTF_FK("required bytecode %s{%d}\n", bytecodeName, bytecodeSym);
    tbl_st* pTbl = m_searchTbl(bytecodeSym);
    if (pTbl != NULL) {
        return pTbl->bytecode;
    }

    Bytecode* pBytecode = new Bytecode();
    if (! pBytecode->readFile((const char*)bytecodeName, m_loadedCallback))
        return NULL;
    HMD_ASSERTMSG(pBytecode->bytecodeSymbol() == bytecodeSym,
                  "{%d} != {%d}",pBytecode->bytecodeSymbol(),bytecodeSym);
    addBytecode(pBytecode, true);
    //HMD__PRINTF_FK("  bytecode %s{%d} loaded\n", bytecodeName, bytecodeSym);
    return pBytecode;
}

Context* CodeManager::createContext(void)
{
    Context* context = new Context();
    addContext(context);
    return context;
}

// contextスレッドと関連がなければ破棄する
// 関連があれば後始末はスレッドに任せてCodeManagerの管理下から外す
void CodeManager::releaseContext(Context* context)
{
    if (context != NULL) {
        removeContext(context);
        //if (context->thread == NULL)
        //    context->destroy();
    }
}

void CodeManager::addContext(Context* context)
{
    if (m_contexts.find(context))
        return;
    hys32 idx;
    if (m_contexts.find(NULL, &idx)) {
        m_contexts[idx] = context;
        return;
    }
    m_contexts.add(context);
}

void CodeManager::removeContext(Context* context)
{
    hys32 idx;
    if (m_contexts.find(context, &idx))
        m_contexts[idx] = NULL;
}


// 置き換えられたバイトコードの実行中フラグをクリア
void CodeManager::clearCodeUsingFlag(void)
{
    for (hys32 i = (hys32)m_replaced.size() - 1; i >= 0; --i) {
        m_replaced[i].inUse = false;
    }
}

// codeAddrの部分のバイトコードが実行中である事を登録
void CodeManager::usingCodeAt(const hyu8* codeAddr)
{
    for (hys32 i = (hys32)m_replaced.size() - 1; i >= 0; --i) {
        repl_st& r = m_replaced[i];
        if (! r.inUse) {
            if (r.bytecode->isInCodeAddr(codeAddr)) {
                //HMD_PRINTF("bytecode '%s' (%x-%x) is in use (code=%x)\n",gSymbolTable.id2str(r.bytecode->bytecodeSymbol()),r.bytecode->codeStartAddr(),r.bytecode->codeEndAddr(),codeAddr);
                r.inUse = true;
                return;
            }
        }
    }
}

// 文字列が置き換えられたバイトコードの中のテーブルを指していたら、
// m_stringBoxにコピーしてそちらを指すように変更
// m_stringBoxの中を指していたらそれをマーク
void CodeManager::markString(const char** pStr)
{
    for (hys32 i = (hys32)m_replaced.size() - 1; i >= 0; --i) {
        repl_st& r = m_replaced[i];
        if (r.bytecode->bHaveString((const hyu8*)*pStr)) {
            //HMD_PRINTF("replace str '%s' from %x",*pStr,*pStr);
            *pStr = m_stringBox.store(*pStr);
            //HMD_PRINTF(" to %x\n", *pStr);
            return;
        }            
    }
    m_stringBox.mark(*pStr);
}


// バイトコードリロードされた後、どこからも参照されなくなった
// 古いバイトコードを削除
void CodeManager::deleteUnnecessaryBytecode(void)
{
    for (hys32 i = (hys32)m_replaced.size() - 1; i >= 0; --i) {
        if (! m_replaced[i].inUse) {
            //repl_st& r = m_replaced[i];HMD_PRINTF("delete bytecode '%s' (%x-%x)\n",gSymbolTable.id2str(r.bytecode->bytecodeSymbol()),r.bytecode->codeStartAddr(),r.bytecode->codeEndAddr());
            delete m_replaced[i].bytecode;
            m_replaced.remove(i);
        }
    }
}



//============================================================
//              バイトコードリロード
//============================================================

#ifdef HY_ENABLE_BYTECODE_RELOAD

Bytecode* CodeManager::reloadBytecode(const char* filename, SymbolID_t asName, bool bInit)
{
    GC::full();
    Bytecode* pBytecode = new Bytecode();
    if (! pBytecode->readFile(filename, m_loadedCallback)) {
        delete pBytecode;
        return NULL;
    }
    overrideBytecode(pBytecode, asName, bInit);
    return pBytecode;
}

void CodeManager::overrideBytecode(Bytecode* pBytecode, SymbolID_t asName, bool bInit)
{
    // GCでsweep終了後にバイトコードの読み込みとこの関数の呼出しが行なわれる
    HMD_ASSERT(GC::m_phase == GC::PHASE_SWEPT);

    // stdlibのリロードには対応していない
    SymbolID_t bytecodeSym = pBytecode->bytecodeSymbol();
    //HMD_PRINTF("reload bytecode '%s' as '%s'\n",gSymbolTable.id2str(bytecodeSym),gSymbolTable.id2str(asName));
    HMD_ASSERT(bytecodeSym != HSymb_stdlib && asName != HSymb_stdlib);
    if (asName != SYMBOL_ID_ERROR) {
        pBytecode->changeBytecodeSymbol(asName);
        bytecodeSym = asName;
    }
    tbl_st* pTbl = m_searchTbl(bytecodeSym);
    if (pTbl == NULL) {
        addBytecode(pBytecode, true);
        return;
    }

    // 旧バイトコードを m_tbl から m_replaced 管理に移動
    repl_st repl;
    repl.bytecode = pTbl->bytecode;
    repl.inUse = true;          // GC::unmark時にfalseにされる
    m_replaced.add(repl);

    // 新バイトコードを m_tbl に登録
    pTbl->bytecode = pBytecode;
    pTbl->autoloadFlag = true;

    if (m_ov_keepObsoleteVar) {
        // 旧変数を保持
        const_cast<HClass*>(pBytecode->mainClass())->keepObsoleteVar(repl.bytecode->mainClass());
    }
    
    // 新バイトコードの初期化
    const Bytecode* curWorkingBC = gCodeManager.getWorkingBytecode();
    Context* curContext = VM::getContext();
    Context* context = createContext();
    pBytecode->initLinkBytecode(context, bInit);
    releaseContext(context);
    VM::setContext(curContext);
    gCodeManager.setWorkingBytecode(curWorkingBC);
    
    // 全クラスのnodeFlagを0にする
    TArray<tbl_st>& tbls = m_tbl.values();
    for (hys32 i = (hys32)tbls.size() - 1; i >= 0; --i)
        const_cast<HClass*>(tbls[i].bytecode->mainClass())->recSetNodeFlag(0);

    // 旧バイトコードから新バイトコードへデータのコピー
    repl.bytecode->copyClassDataTo(pBytecode, m_ov_copyClassVar, m_ov_copyConstVar);
    // copyClassDataToで構築された新旧HClass*対応表 //
    BMap<const HClass*, const HClass*>& genMap = repl.bytecode->m_generationMap;


    // 既存クラスのスーパークラスへのポインタの調整
    for (hys32 i = (hys32)tbls.size() - 1; i >= 0; --i)
        const_cast<HClass*>(tbls[i].bytecode->mainClass())->regulateReloadedSuper(genMap);

    // 既存オブジェクトのHClass*の付け替え : GC::mark で付け替えが行なわれる
    GC::pGenerationMap = &genMap;
    GC::full();
    GC::pGenerationMap = NULL;

    repl.bytecode->m_generationMap.finalize();
}

void CodeManager::takeReplacedBytecodeSyms(ValueArray* arr)
{
    for (hys32 i = (hys32)m_replaced.size() - 1; i >= 0; --i) {
        arr->add(Value::fromSymbol(m_replaced[i].bytecode->bytecodeSymbol()));
    }
    //return arr;
}

#endif // HY_ENABLE_BYTECODE_RELOAD





//============================================================



// GC mark
// 各バイトコード → メインインスタンス、クラス変数
// 登録Context
void CodeManager::m_GC_mark(void)
{
    //HMD_PRINTF("CodeManager::m_GC_mark()  m_contexts.size()=%d\n",m_contexts.size());
    Debug::incMarkTreeLevel();
    TArray<tbl_st>& tbls = m_tbl.values();
    for (hys32 i = (hys32)tbls.size() - 1; i >= 0; --i) {
        Bytecode* pBytecode = tbls[i].bytecode;
        HMD_DEBUG_ASSERT(pBytecode != NULL);
        HMD__PRINTF_GC("GC mark bytecode %x\n", pBytecode);
        pBytecode->m_GC_mark_staticVar();
    }
    for (int i = m_contexts.size() - 1; i >= 0; --i) {
        Context* context = m_contexts[i];
        if (context != NULL) {
            GC::markObj(Object::fromCppObj(context));
        }
    }
    Debug::decMarkTreeLevel();
}


void CodeManager::startMarkIncremental(void)
{
    GC::m_arrIdx = m_tbl.size();
    GC::m_countdown_inc = GC::m_num_perPart_inc;
    Debug::incMarkTreeLevel();
}

void CodeManager::markIncremental(void)
{
    TArray<tbl_st>& tbls = m_tbl.values();
    while (--GC::m_arrIdx >= 0) {
        Bytecode* pBytecode = tbls[GC::m_arrIdx].bytecode;
        HMD_DEBUG_ASSERT(pBytecode != NULL);
        HMD__PRINTF_GC("GC mark bytecode %x\n", pBytecode);
        pBytecode->m_GC_mark_staticVar();
        if (--GC::m_countdown_inc <= 0) {
            GC::m_countdown_inc += GC::m_num_perPart_inc;
            return;
        }
    }
    GC::m_subPhase_end = true;
    Debug::decMarkTreeLevel();
}

void CodeManager::startMarkIncremental_2(void)
{
    GC::m_arrIdx = m_contexts.size();
    Debug::incMarkTreeLevel();
}

void CodeManager::markIncremental_2(void)
{
    while (--GC::m_arrIdx >= 0) {
        Context* context = m_contexts[GC::m_arrIdx];
        HMD__PRINTF_GC("GC mark context %x idx %d\n", context, GC::m_arrIdx);
        if (context != NULL) {
            GC::markObj(Object::fromCppObj(context));
            return;
        }
    }
    GC::m_subPhase_end = true;
    Debug::decMarkTreeLevel();
}

void CodeManager::markAllStack(void)
{
    int i = m_contexts.size();
    while (--i >= 0) {
        Context* context = m_contexts[i];
        HMD__PRINTF_GC("GC CodeManager::markAllStack context=%x\n",context);
        if (context != NULL)
            context->m_GC_mark_stack();
    }
}
