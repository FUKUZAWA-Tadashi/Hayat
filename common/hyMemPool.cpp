/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyMMes.h"
#include "hyEndian.h"
#include <string.h>
#ifndef HY_WORK_WITHOUT_VM
# include "hyObject.h"
# include "hyDebug.h"
#endif


#ifdef MS_DLL_EXPORT_DECL
# define DLL_EXPORT     _declspec(dllexport)
#else
# define DLL_EXPORT
#endif


#ifdef HMD__MEM_CHECK
#define DEBUG_CHECK_MEMORY()     HMD_DEBUG_ASSERT(check())
#else
#define DEBUG_CHECK_MEMORY()     ((void)0)
#endif



using namespace Hayat::Common;

HMD_DLL_EXPORT MemPool* Hayat::Common::gMemPool = NULL;


const size_t MemPool::m_binSizes[MA_NUMBINS] = {
    MA_BIN0SIZE, MA_BIN1SIZE, MA_BIN2SIZE, };
const size_t MemPool::m_binLimits[MA_NUMBINS+1] = {
    0, MA_BIN0LIMIT, MA_BIN1LIMIT, MA_BIN2LIMIT, };
const int MemPool::m_binIndMax[MA_NUMBINS+1] = {
    0, MA_BIN0INDMAX, MA_BIN1INDMAX, MA_BIN2INDMAX, };

#ifdef m_MEMCELL_JUST_ARENA_SIZE_
size_t MemPool::m_binIndex2size[MA_NUM_TOTAL_BINS];
bool MemPool::m_binIndex2size_initialized_flag = false;
#endif



void MemCell::m_setFree(void)
{
    size_t mySize = size();
    size_t* ps = (size_t*)addAddr(this, mySize);
    *ps |= MASK_PREV_FREE;
    *--ps = mySize;
}

void MemCell::setGCObject(void)
{
    size_t* addr= (size_t*)addAddr(this, size());
    (*addr) |= MASK_GC_OBJECT;
}
bool MemCell::isGCObject(void)
{
    size_t* addr= (size_t*)addAddr(this, size());
    //HMD__PRINTF_MEM("%x-%x",this,addr);
    //HMD__PRINTF_MEM(m_bFree()?" free\n":" use\n");
    return ((*addr) & MASK_GC_OBJECT_FREE) == MASK_GC_OBJECT;
}
void MemCell::unsetGCObject(void)
{
    size_t* addr= (size_t*)addAddr(this, size());
    (*addr) &= ~MASK_GC_OBJECT;
}

void MemCell::fillZero(void)
{
    if (m_bFree()) {
        memset(((char*)this) + SIZEINFO_SIZE * 2, 0, size() - SIZEINFO_SIZE * 3);
    } else {
        memset(this, 0, size());
    }
}

#ifdef HMD_DEBUG_MEMORY_ID
void MemCell::setMemID(const char* id)
{
    memcpy(m_pMemID(), id, 4);
}
#endif




/*static*/ MemPool* MemPool::manage(void* addr, size_t size)
{
    HMD__PRINTF_MEM("MemPool manage addr = %x, size = %x\n", (int)addr, (int)size);
    HMD_ASSERT(((int)addr & 3) == 0);      // 4byte align
    size = (size_t)alignInt<4>(size);
    HMD_ASSERT(size >= sizeof(MemPool) + 64); // 最低でも64バイトは管理する
    MemPool* self = (MemPool*)addr;
    self->initialize(addAddr(addr,sizeof(MemPool)), size - sizeof(MemPool));
    return self;
}

void MemPool::initGMemPool(void* addr, size_t size)
{
    gMemPool = manage(addr, size);
}

void MemPool::initialize(void* addr, size_t size)
{
    // 初期化時のメモリ配置
    // 空のMemCellが1つと、最後のセルがフリーかどうかのフラグを持つ。
    // +--(A)--- m_poolAddrをalignしたアドレス
    // | prevSize (未使用：念のため確保)
    // +--------
    // | size       (B)～(C)のサイズ

    // +--------
    // | memID      HMD_DEBUG_MEMORY_IDがdefineされている時のみ有効

    // +--(B)--- m_head[m_size2binIndex(size)]
    // | m_nextFree  (B) を指している
    // +--------
    // | m_prevFree  (B) を指している
    // +--------
    // |
    // | 空き
    // |
    // +--------
    // | prevSize
    // +--(C)---
    // | (size)     bit0 = 最後のセルがフリーかどうかのフラグ
    // +--(D)--- m_poolEnd
        
    m_GC_working_in_alloc = false;
    m_poolAddr = addr;
#if defined(HMD_DEBUG) || defined(TEST__CPPUNIT)
    memCellPrintFunc = NULL;
#endif

    // 4 byte align
    m_poolEnd = addAddr(addr, size-3);
    alignPtr<4>(&m_poolEnd);
    addr = m_poolAddr;
    alignPtr<4>(&addr);
    size = (char*)m_poolEnd - (char*)addr;

    // アリーナ初期化
    for (int i = 0; i < MA_NUM_TOTAL_BINS; i++)
        m_head[i] = NULL;

    // 最初のMemCellを作成
    MemCell* p = (MemCell*)addAddr(addr, MemCell::SIZEINFO_SIZE * 2 + m_MEMID_SIZE);
    p->setMemID("INIT");
    *(p->m_pSize()) = size - MemCell::SIZEINFO_SIZE * 3 - m_MEMID_SIZE;
    m_attach(p);
    m_freeCellSizeTotal = size - MemCell::SIZEINFO_SIZE * 3 - m_MEMID_SIZE;
#ifdef HMD_DEBUG
    m_maxFreeCellSize = m_freeCellSizeTotal;
#endif

    registeredCellIterator = NULL;

    m_absorbAlertEnableFlag = true;
    setAlertLevel(DEFAULT_ALERT_LEVEL);

#ifdef m_MEMCELL_JUST_ARENA_SIZE_
    // 未初期化なら m_binIndex2size[] を初期化
    if (! m_binIndex2size_initialized_flag) {
        size_t size = 0;
        for (int n = 0; n < MA_NUMBINS; n++) {
            HMD_DEBUG_ASSERT(size == m_binLimits[n]);
            // size = m_binLimits[n];
            for (int i = m_binIndMax[n]; i < m_binIndMax[n+1]; i++) {
                m_binIndex2size[i] = size;
                size += m_binSizes[n];
            }
        }
        m_binIndex2size[MA_BINLASTINDMAX] = size;
        m_binIndex2size_initialized_flag = true;
    }
#endif
}


#if 0
// allocメモリアドレスチェック用プログラム
static void* m_debug_checkAddr_[1] = {
    (void*)0xefefefef // チェックしたいアドレスを入れる
};
static void m_debug_checkAllocAddr_(void* p)
{
    if (p == m_debug_checkAddr_[0])
        HMD_PRINTF("m_debug_checkAllocAddr_(%x)\n", p); // デバッガでここにブレークを仕掛ける
}
#else
static inline void m_debug_checkAllocAddr_(MemCell*) {}
#endif

MemCell* MemPool::alloc(size_t size)
{
    if (size <= MemCell::MINIMUM_FREECELL_SIZE)
        size = MemCell::MINIMUM_FREECELL_SIZE;
    else
        size = (size + 3) & ~3;         // 4 bytes align
    DEBUG_CHECK_MEMORY();
    MemCell* pCell = m_alloc_1(size);
    m_debug_checkAllocAddr_(pCell);
    if (pCell != NULL) {
        DEBUG_CHECK_MEMORY();
        return pCell;
    }
        
    HMD_ASSERTMSG(!m_GC_working_in_alloc, M_M("call alloc() inside alloc() : finalize may called alloc()"));
    m_GC_working_in_alloc = true;

#if 0
#ifdef HMD_DEBUG
#ifndef TEST__CPPUNIT
    printCellInfo(memCellPrintFunc);
    HMD__PRINTF_FK("***********************************\n");
#endif
#endif
#endif

    // ここで GC::collect() すると、C++のローカル変数に入っている
    // オブジェクトを取りこぼすので、してはいけない。

    coalesce(); // coalesceのみやる。


    m_GC_working_in_alloc = false;
    pCell = m_alloc_1(size);
    m_debug_checkAllocAddr_(pCell);        


    if (pCell == NULL) {
#ifdef HMD_PRINT_MEMCELL_WHEN_ALLOC_FAILED
        printCellInfo(memCellPrintFunc);
#endif
        HMD_FATAL_ERROR("cannot allocate memory size=0x%x\n",size);
    }

    return pCell;
}


MemCell* MemPool::m_alloc_1(size_t size)
{
    HMD_DEBUG_ASSERT((size & 3) == 0);
    MemCell* pCell;
    int ind = m_size2binIndex_alloc(size);
    if (ind < MA_BINLASTINDMAX) {
        pCell = m_head[ind];
        if (pCell != NULL) {
            // ジャストフィットなセルがあった
            m_detachFrom(pCell, ind);
            pCell->m_setAlloc();
            pCell->fillZero();
            //HMD_PRINTF("alloc just %x for size %x, cellSize %x\n",pCell,size,pCell->size());
            m_freeCellSizeTotal -= pCell->size();
            return pCell;
        }
    }
    // より大きいサイズを探す
    for ( ; ind < MA_BINLASTINDMAX; ind++) {
        if (m_head[ind] != NULL) {
            pCell = m_divide(ind, m_head[ind], size);
            pCell->fillZero();
            m_freeCellSizeTotal -= pCell->size();
            return pCell;
        }
    }
    // アリーナに入らないサイズなので m_head[MA_BINLASTINDMAX] から探す
    pCell = m_head[MA_BINLASTINDMAX];
    if (pCell == NULL)
        return NULL;
    do {
        HMD_DEBUG_ASSERT(pCell->m_bFree());
        if (pCell->size() >= size) {
            pCell = m_divide(MA_BINLASTINDMAX, pCell, size);
            pCell->fillZero();
            m_freeCellSizeTotal -= pCell->size();
            return pCell;
        }
        pCell = pCell->m_nextFree;
    } while (pCell != m_head[MA_BINLASTINDMAX]);
        
    return NULL;
}

// m_head[ind]のセルを(適切なら)分割してsizeのセルを切り出す
MemCell* MemPool::m_divide(int ind, MemCell* pCell, size_t size)
{
    HMD_ASSERT(size > 0);
    size_t msize = pCell->size();
    HMD_ASSERT(msize >= size);

#ifdef m_MEMCELL_JUST_ARENA_SIZE_
    size_t justSize = m_binIndex2size[m_size2binIndex_alloc(size)];
    if ((msize >= justSize) && (size < justSize)) {
        size = justSize;
    }
#endif

    m_detachFrom(pCell, ind);
    if (msize >= size + MemCell::MINIMUM_FREECELL_SIZE + MemCell::MEMCELL_HDR_SIZE) {
        // 分割
        HMD__PRINTF_MEM("alloc div %x size %x\n",pCell,msize);

        MemCell* pRemain = (MemCell*)addAddr(pCell, size + MemCell::MEMCELL_HDR_SIZE);
        msize -= size + MemCell::MEMCELL_HDR_SIZE;
        HMD_DEBUG_ASSERT(msize >= MemCell::MINIMUM_FREECELL_SIZE);
        *(pRemain->m_pSize()) = msize; // prev_free フラグは落ちるので
        // pCell->m_setAlloc(); は不要
        pRemain->setMemID("DIV2");
        m_attach(pRemain);

        pCell->setSize(size);
        //pCell->setMemID("DIV1");
        HMD__PRINTF_MEM("   -> %x size %x : remain %x size %x\n",pCell,pCell->size(),pRemain,pRemain->size());
        m_freeCellSizeTotal -= MemCell::MEMCELL_HDR_SIZE;
    } else {
        // 分割しない
        pCell->m_setAlloc();
        HMD__PRINTF_MEM("alloc large %x size %x (need %x)\n",pCell,pCell->size(),size);
    }
    return pCell;
}

MemCell* MemPool::realloc(MemCell* pCell, size_t size, size_t copySize)
{
    if (pCell == NULL) {
        MemCell* p = alloc(size);
        p->setMemID("REA0");
        return p;
    }
    HMD_ASSERT(! pCell->m_bFree());
    if (pCell->size() >= size)
        return pCell;

    MemCell* nextCell = pCell->m_pNextNeighbor();
    if ((void*)nextCell < m_poolEnd && nextCell->m_bFree()) {
        size_t nSize = nextCell->size();
        if (nSize < size) {
            size_t coaSize = pCell->size() + nSize + MemCell::MEMCELL_HDR_SIZE;
            if (coaSize >= size) {
                // 隣りのフリーなセルとくっつけて容量確保
                m_detach(nextCell);
                // nextCell->m_setAlloc(); // 後での pCell->m_setAllocとかぶる
                pCell->setSize(coaSize);
                pCell->m_setAlloc();
                DEBUG_CHECK_MEMORY();
                m_freeCellSizeTotal -= nSize + MemCell::MEMCELL_HDR_SIZE;
                if ((registeredCellIterator != NULL) && (nextCell == **registeredCellIterator)) {
                    HMD__PRINTF_MEM("COALESCE on realloc REGISTERED CELL ITERATOR %x",**registeredCellIterator);
                    (*registeredCellIterator)++;
                    HMD__PRINTF_MEM(" to %x\n",**registeredCellIterator);
                }
                return pCell;
            }
        }
    }

    // 別セルを確保
    MemCell* newCell = alloc(size);
#ifdef HMD_DEBUG_MEMORY_ID
    newCell->setMemID(pCell->memID());
#endif
    if (newCell == NULL)
        return NULL;
    if (copySize == (size_t)-1)
        copySize = pCell->size();
    memcpy(newCell, pCell, copySize);
    free(pCell);
    return newCell;
}


void MemPool::free(MemCell* pCell)
{
    if (pCell == NULL)
        return;
    size_t size = pCell->size();
    HMD__PRINTF_MEM("free %x size %x\n",pCell,size);
    HMD_DEBUG_ASSERT(size > 0);
    HMD_ASSERT(! pCell->m_bFree());
    m_attach(pCell);
    m_freeCellSizeTotal += size;
}

// 2つの隣あったフリーなセルを結合
void MemPool::m_coalesce2(MemCell* p, MemCell* q)
{
    HMD_DEBUG_ASSERT(q < m_poolEnd);
    HMD_DEBUG_ASSERT(p->m_pNextNeighbor() == q);
    HMD_DEBUG_ASSERT(p->m_bFree());
    HMD_DEBUG_ASSERT(q->m_bFree());

    HMD__PRINTF_MEM("coalesce %x(%x) and %x(%x) ->",p,p->size(),q,q->size());
    if ((registeredCellIterator != NULL) && (q == **registeredCellIterator)) {
        HMD__PRINTF_MEM("COALESCE REGISTERED CELL ITERATOR %x",**registeredCellIterator);
        (*registeredCellIterator)++;
        HMD__PRINTF_MEM(" to %x\n",**registeredCellIterator);
    }
    m_detach(p);
    m_detach(q);
    // p->m_setAlloc(); pの隣はqなので不要 
    // q->m_setAlloc(); 最後の m_attach(p) でつじつまが合うので不要

    p->setSize(p->size() + q->size() + MemCell::MEMCELL_HDR_SIZE);

    m_attach(p);
    p->setMemID("COAL");
    HMD__PRINTF_MEM(" %x(%x)\n",p,p->size());

    m_freeCellSizeTotal += MemCell::MEMCELL_HDR_SIZE;

}


// フリーな隣りあったセルを全部結合
void MemPool::coalesce(void)
{
    MemCell* p = (MemCell*)((((int)m_poolAddr) + 3 + MemCell::SIZEINFO_SIZE * 2 + m_MEMID_SIZE) & ~3);
    MemCell* q = p->m_pNextNeighbor();
    while ((void*)q < m_poolEnd) {
        if (p->m_bFree() && q->m_bFree()) {
            m_coalesce2(p, q);
        } else {
            p = q;
        }
        q = p->m_pNextNeighbor();
    }
}


// 使用中セルの割合
int MemPool::percentUsed(void)
{
    size_t all = (char*)m_poolEnd - (char*)m_poolAddr;
    if (m_freeCellSizeTotal < 0) // 念のため
        return 100;
    if (m_freeCellSizeTotal >= all) // 念のため
        return 0;
        
    return 100 * (all - m_freeCellSizeTotal) / all;
}

// 使用メモリ取得
int MemPool::getUsed(void)
{
    size_t all = (char*)m_poolEnd - (char*)m_poolAddr;
    if (m_freeCellSizeTotal < 0) // 念のため
        return 0;
    if (m_freeCellSizeTotal >= all) // 念のため
        return 0;
        
    return (all - m_freeCellSizeTotal);
}

// 危険レベルを設定
// 使用メモリパーセントがこれ以上になると、isAlert()がtrueになる
void MemPool::setAlertLevel(int percent)
{
    HMD_DEBUG_ASSERT((percent >= 0) && (percent <= 100));
    size_t all = (char*)m_poolEnd - (char*)m_poolAddr;
    m_alertLevel = (all / 100) * (100 - percent);
    calcAlertAbsorbLevel();
}

// メモリ使用状況が危険レベルかどうか
bool MemPool::isAlertLevel(void)
{
    return (m_freeCellSizeTotal <= m_alertLevel);
}
// メモリ使用状況がさらに危険レベルかどうか
bool MemPool::isAlertAbsorbLevel(void)
{
    return m_absorbAlertEnableFlag && (m_freeCellSizeTotal <= m_alertAbsorbLevel);
}
// sweep終了後に、m_alertAbsorbLevelを再計算
void MemPool::calcAlertAbsorbLevel(void)
{
    if (m_freeCellSizeTotal <= m_alertLevel) {
        m_alertAbsorbLevel = m_freeCellSizeTotal / 2;
        //HMD_PRINTF("mem alert: free=%x alert=%x absorb=%x\n",m_freeCellSizeTotal,m_alertLevel,m_alertAbsorbLevel);
    } else
        m_alertAbsorbLevel = m_alertLevel;
}



int MemPool::m_size2binIndex_alloc(size_t size)
{
    if (size < MemCell::MINIMUM_FREECELL_SIZE)
        size = MemCell::MINIMUM_FREECELL_SIZE;
    for (int i = 0; i < MA_NUMBINS; i++) {
        if (size < m_binLimits[i+1]) {
            return m_binIndMax[i] + (size - m_binLimits[i] + m_binSizes[i] - 1) / m_binSizes[i];
        }
    }
    return m_binIndMax[MA_NUMBINS];
}

int MemPool::m_size2binIndex_free(size_t size)
{
    if (size < MemCell::MINIMUM_FREECELL_SIZE)
        size = MemCell::MINIMUM_FREECELL_SIZE;
    for (int i = 0; i < MA_NUMBINS; i++) {
        if (size < m_binLimits[i+1]) {
            return m_binIndMax[i] + (size - m_binLimits[i]) / m_binSizes[i];
        }
    }
    return m_binIndMax[MA_NUMBINS];
}


// セルをアリーナの指定リストに追加
void MemPool::m_attachTo(MemCell* pCell, int ind)
{
    //HMD__PRINTF_MEM("attach %x(%x) to [%d]\n",pCell,pCell->size(),ind);
    pCell->m_setFree();
    MemCell* ph = m_head[ind];
    if (ph == NULL) {
        pCell->m_nextFree = pCell;
        pCell->m_prevFree = pCell;
    } else {
        pCell->m_nextFree = ph;
        pCell->m_prevFree = ph->m_prevFree;
        pCell->m_prevFree->m_nextFree = pCell;
        ph->m_prevFree = pCell;
    }
    m_head[ind] = pCell;
}

// セルをアリーナの指定リストから外す
void MemPool::m_detachFrom(MemCell* pCell, int ind)
{
    //HMD__PRINTF_MEM("detach %x(%x) from [%d]\n",pCell,pCell->size(),ind);
    HMD_DEBUG_ASSERT(pCell->m_bFree());
    MemCell* ph = pCell->m_nextFree;
    if (ph == pCell) {
        HMD_DEBUG_ASSERT(pCell->m_prevFree == pCell);
        HMD_DEBUG_ASSERT(m_head[ind] == pCell);
        m_head[ind] = NULL;
    } else {
        MemCell* pn = pCell->m_nextFree;
        MemCell* pp = pCell->m_prevFree;
        pn->m_prevFree = pp;
        pp->m_nextFree = pn;
        m_head[ind] = pn;
    }
}




//============================================================
// イテレータ
//============================================================

CellIterator MemPool::begin(void) const
{
    CellIterator itr((MemCell*)((((int)m_poolAddr) + 3 + MemCell::SIZEINFO_SIZE * 2 + m_MEMID_SIZE) & ~3));
    return itr;
}
    
CellIterator MemPool::end(void) const
{
    CellIterator itr((MemCell*)m_poolEnd);
    return itr;
}


//============================================================
// デバッグ
//============================================================

void MemPool::printFreeCellInfo(void)
{
    MemCell* pCell;
    MemCell* q;
    // アリーナ
    HMD_PRINTF("size: count ========== MemPool Free Cells ==========\n");
    for (int i = 0; i < MA_BINLASTINDMAX; i++) {
        pCell = m_head[i];
        if (pCell != NULL) {
            int count = 1;
            q = pCell->m_nextFree;
            while (q != pCell) {
                ++count;
                q = q->m_nextFree;
            }
            size_t size = 0;
            for (int j = 0; j < MA_NUMBINS; j++) {
                if (i < m_binIndMax[j+1]) {
                    size = m_binLimits[j] + m_binSizes[j] * (i - m_binIndMax[j]);
                    break;
                }
            }       
            HMD_PRINTF("%4d: %d\n", size, count);
        }
    }
    // 大サイズセル
    pCell = m_head[MA_BINLASTINDMAX];
    if (pCell != NULL) {
        q = pCell;
        HMD_PRINTF("Large size Cells:\n");
        do {
            HMD_PRINTF("%d ", q->size());
            q = q->m_nextFree;
        } while (q != pCell);
        HMD_PRINTF("\n");
    }
}
    
void MemPool::printCellInfo(MemCellPrintFunc_t prFunc)
{
    HMD_PRINTF("========== MemPool Cell Info ==========\n");
    CellIterator last(end());
    char id[5];
    id[0] = id[4] = '\0';
    for (CellIterator itr = begin(); itr < last; itr++) {
        if (prFunc != NULL) {
            prFunc(*itr);
        } else {
            MemCell* p = *itr;
            hyu32 s = p->size();
#ifdef HMD_DEBUG_MEMORY_ID
            memcpy(id, p->memID(), 4);
#endif

            if (p->m_bFree())
                HMD_PRINTF("%x Free    %s size=%x\n",p,id,s);
            else if (p->isGCObject())
#ifdef HY_WORK_WITHOUT_VM
                HMD_PRINTF("%x Object  %s size=%x\n",p,id,s);
#else
                HMD_PRINTF("%x Object  %s size=%x type=%s\n",p,id,s,
                           ((Engine::Object*)p)->type()->name());
#endif
            else
                HMD_PRINTF("%x MemCell %s size=%x\n",p,id,s);
        }                    
    }
    HMD_PRINTF("==========\n");
}


void MemCell::printMarkNode(void)
{
#ifdef HMD__DEBUG_MARK_TREE
    if (! Hayat::Engine::Debug::isPrintMarkTreeOn())
        return;
    
    int lv = Hayat::Engine::Debug::markTreeLevel();
    while (lv-- > 0) HMD_PRINTF(".");

    char id[5];
    id[0] = id[4] = '\0';
#ifdef HMD_DEBUG_MEMORY_ID
    memcpy(id, memID(), 4);
#endif
    hyu32 s = size();
    if (m_bFree()) {
        HMD_PRINTF("%x !!! FREE %s size=%x\n",this,id,s);
    } else if (isGCObject()) {
#ifdef HY_WORK_WITHOUT_VM
        HMD_PRINTF("%x Object  %s size=%x\n",this,id,s);
#else
        HMD_PRINTF("%x Object  %s size=%x type=%s\n",this,id,s,
                   ((Engine::Object*)this)->type()->name());
#endif
    } else {
        HMD_PRINTF("%x MemCell %s size=%x\n",this,id,s);
    }
#endif
}



#if defined(HMD_DEBUG) || defined(TEST__CPPUNIT)
// セルの数をカウント
// 矛盾があった場合は halt
int MemPool::check_neighbor(void)
{
    int count = 0;
    size_t total = 0;
    MemCell* p = (MemCell*)((((int)m_poolAddr) + 3 + MemCell::SIZEINFO_SIZE * 2 + m_MEMID_SIZE) & ~3);
    while ((void*)p < m_poolEnd) {
        ++count;
        total += p->size();
        if (p->m_bFree()) {
            HMD_ASSERT(p->size() == *(p->m_pNextNeighborsPrevSize()));
            HMD_ASSERT(p->m_nextFree >= m_poolAddr);
            HMD_ASSERT(p->m_nextFree <= m_poolEnd);
            HMD_ASSERT(((int)(p->m_nextFree) & 3) == 0);
            HMD_ASSERT(p->m_prevFree >= m_poolAddr);
            HMD_ASSERT(p->m_prevFree <= m_poolEnd);
            HMD_ASSERT(((int)(p->m_prevFree) & 3) == 0);
        }
        p = p->m_pNextNeighbor();
    }
    HMD_ASSERT((void*)p == addAddr(m_poolEnd,m_MEMID_SIZE));
    HMD_ASSERT((total + count * MemCell::MEMCELL_HDR_SIZE + 2 * MemCell::SIZEINFO_SIZE)
               == (hyu32)(((char*)m_poolEnd - (char*)m_poolAddr) & ~3));
            
    return count;
}

// freeなセルを辿っていき、内部情報が矛盾していないか、
// 最後に循環して元のセルに戻れるか
bool MemPool::check_free_consistency(MemCell* pHead)
{
    // チェックするfreeセル数の最大値
    int limit = 20000;
    MemCell* p = pHead;
    MemCell* q;
    do {
        q = p->m_pNextNeighbor();
        if (! q->m_bPrevFree()) {
            HMD_PRINTF("check_free_consistency() prev available check failed (limit=%d)\n",limit);
            return false;
        }
        if (p->size() != *(q->m_pPrevSize())) {
            HMD_PRINTF("check_free_consistency() %x size(%d) != neighbor->prevSize(%d) (limit=%d)\n", p, p->size(), *(q->m_pPrevSize()), limit);
            return false;
        }
        p = p->m_nextFree;
    } while ((p != pHead) && (--limit > 0));

    if (p != pHead) {
        HMD_PRINTF("check_free_consistency() free cell not loop\n");
        return false;
    }

    return true;
}

bool MemPool::check(void)
{
    if (check_neighbor() < 1) {
        HMD_PRINTF("check_neighbor failed\n");
        return false;
    }
    for (int i = 0; i < MA_NUM_TOTAL_BINS; i++) {
        if (m_head[i] != NULL)
            if (! check_free_consistency(m_head[i]))
                return false;
    }
    return true;
}

#endif // defined(HMD_DEBUG) || defined(TEST__CPPUNIT)
