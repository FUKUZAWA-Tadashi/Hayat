/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

/*
 * 低レベルメモリ管理を行なう。
 * 1つの連続メモリ領域を保持し、
 * メモリの貸し出し、返却を行なう。
 */

/*
  使用例

  void* p = System_Allocate(size);
  MemPool* pool = MemPool::manage(p, size);
  MemCell* cell = pool->alloc(x);
  pool->free(cell);
  System_Free(p);  または  System_Free(pool);    // pool == p
*/

#ifndef m_HYMEMPOOL_H_
#define m_HYMEMPOOL_H_


#include "machdep.h"



// HMD__PRINTF_MEM
//      メモリ関連情報を表示
// HMD__MEM_CHECK
//      メモリの確保・開放・GC時に、管理テーブルの整合性チェックをする
// HMD_DEBUG_MEMORY_ID
//      各メモリセルにID情報4バイトを確保するかどうか
// HMD_PRINT_MEMCELL_WHEN_ALLOC_FAILED
//      メモリ確保失敗時に、メモリセル情報を出力するかどうか


#ifdef HMD_DEBUG
// 使いたい時にアンコメントする
//# define HMD__PRINTF_MEM(...)	HMD_PRINTF(__VA_ARGS__)
//# define HMD__MEM_CHECK
# define HMD_DEBUG_MEMORY_ID
#ifndef TEST__CPPUNIT
# define HMD_PRINT_MEMCELL_WHEN_ALLOC_FAILED
#endif
#else
// 以下変更しない事
# undef HMD__PRINTF_MEM
# undef HMD__MEM_CHECK
# undef HMD_DEBUG_MEMORY_ID
# undef HMD_PRINT_MEMCELL_WHEN_ALLOC_FAILED
#endif


// 以下変更しない事
#ifndef HMD__PRINTF_MEM
# define HMD__PRINTF_MEM(...)   ((void)0)
#endif




// メモリを確保する時に、要求するサイズがアリーナにぴったり納まるように
// サイズ調整を行なう場合は define する
// これをdefineしても、coalesceするとアリーナにぴったりでなくなる場合はある
#define m_MEMCELL_JUST_ARENA_SIZE_


class Mock_MemPool;      // for unittest
class Test_hyMemPool;    // for unittest
class Test_hyGC;         // for unittest
namespace Hayat {
    namespace Engine {
        class GC;
    }
}


namespace Hayat {
    namespace Common {

        class CellIterator;

        inline void* addAddr(void* addr, size_t d) {
            return (void*)(((char*)addr) + d);
        }


        // ==================== class MemCell ====================
        //
        // +------------
        // | prevSize       直前のセルのサイズ
        // |                直前のセルがフリーの時のみ有効
        // |                直前のセルのメモリ領域と重なっている
        // +----(A)-----
        // | size           このセルのサイズ：this以降の使用可能なサイズ
        // |                常に有効
        // |                下位2bitはフラグ
        // |                  bit0 = 直前のセルがフリーなら 1
        // |                  bit1 = 直前のセルがGC対象なら 1 で、内容は Object である

        // +------------
        // | memID    HMD_DEBUG_MEMORY_ID が define されている時にのみ有効

        // +----(B)----- this : ここがMemCellオブジェクトのアドレス
        // | nextFree       アリーナ中で次のフリーなセルのアドレス
        // |                このセルがフリーの時のみ有効
        // +------------
        // | prevFree       アリーナ中で前のフリーなセルのアドレス
        // |                このセルがフリーの時のみ有効
        // +------------
        // | セル情報の入っていないメモリ
        // +------------
        // | 次のセルのprevSize  このセルがフリーの時のみ有効で、その時
        // | (== size)           次のセルのsizeのbit0が1になっている
        // +----(C)-----
        //
        // 1つのMemCellは(A)から(C)までのメモリを管理する。
        // アプリは(B)から(C)までのメモリを使用可能。
        // アプリが使用していないフリーなMemCellの場合、nextFree,prevFree,
        // 次のセルの prevSize の部分のデータが有効。
        //

        class MemCell {
            friend class MemPool;
            friend class CellIterator;
            friend class Engine::GC;
            friend class ::Mock_MemPool;      // for unittest
            friend class ::Test_hyMemPool;    // for unittest
            friend class ::Test_hyGC;         // for unittest

        private:
            MemCell(void) {HMD_HALT();}     // コンストラクタ無効

        protected:
            static const size_t MASK_SIZE = (size_t)~3;
            static const size_t MASK_FLAGS = (size_t)3;
            static const size_t MASK_PREV_FREE = 0x00000001;
            static const size_t MASK_GC_OBJECT = 0x00000002;
            static const size_t MASK_GC_OBJECT_FREE = (MASK_PREV_FREE | MASK_GC_OBJECT);
            static const size_t SIZEINFO_SIZE = sizeof(size_t);

#ifdef HMD_DEBUG_MEMORY_ID
#define m_MEMID_SIZE 4
#else
#define m_MEMID_SIZE 0
#endif
            static const size_t MEMCELL_HDR_SIZE = SIZEINFO_SIZE + m_MEMID_SIZE;

#ifdef HMD_DEBUG_MEMORY_ID
# ifdef m_MEMCELL_JUST_ARENA_SIZE_
            static const size_t MINIMUM_FREECELL_SIZE = SIZEINFO_SIZE + sizeof(MemCell*) * 2 + MEMCELL_HDR_SIZE + 4;
# else
            static const size_t MINIMUM_FREECELL_SIZE = SIZEINFO_SIZE + sizeof(MemCell*) * 2 + MEMCELL_HDR_SIZE;
# endif
#else
            static const size_t MINIMUM_FREECELL_SIZE = SIZEINFO_SIZE + sizeof(MemCell*) * 2 + MEMCELL_HDR_SIZE;
#endif

            MemCell*        m_nextFree;
            MemCell*        m_prevFree;

#ifdef HMD_DEBUG_MEMORY_ID
            // サイズを格納しているアドレスを取得
            size_t*         m_pSize(void) { return ((size_t*)this) - 2; }
            // 直前セルのサイズを格納しているアドレスを取得
            size_t*         m_pPrevSize(void) { return ((size_t*)this) - 3; }
            // デバッグ用メモリIDを格納しているアドレスを取得
            char*           m_pMemID(void) { return ((char*)this) - 4; }
            // 直後のセルのアドレス (メモリオーバーに注意)
            MemCell*        m_pNextNeighbor(void) { return (MemCell*)addAddr(this, size() + SIZEINFO_SIZE + 4); }
#else
            // サイズを格納しているアドレスを取得
            size_t*         m_pSize(void) { return ((size_t*)this) - 1; }
            // 直前セルのサイズを格納しているアドレスを取得
            size_t*         m_pPrevSize(void) { return ((size_t*)this) - 2; }
            // 直後のセルのアドレス (メモリオーバーに注意)
            MemCell*        m_pNextNeighbor(void) { return (MemCell*)addAddr(this, size() + SIZEINFO_SIZE); }
#endif
            // 直後のセルのm_pPrevSize()
            size_t*         m_pNextNeighborsPrevSize(void) { return (size_t*)addAddr(this, size() - SIZEINFO_SIZE); }
            // このセルがフリーか？
            bool            m_bFree(void) { return ((*(size_t*)addAddr(this, size())) & MASK_PREV_FREE) != 0; }
            // 直前のセルがフリーか？
            bool            m_bPrevFree(void) { return ((*m_pSize()) & MASK_PREV_FREE) != 0; }
            // このセルをフリーとする
            void            m_setFree(void);
            // このセルをフリーではなく、GCObjectでもないとする
            // allocした直後はこの状態になる。setGCObject()は必要があれば
            // allocを呼び出したルーチンが実行する。
            void            m_setAlloc(void) { (*(size_t*)addAddr(this, size())) &= ~MASK_GC_OBJECT_FREE; }


        public:
            // セルが使用可能なメモリサイズを取得 
            size_t  size(void) { return (*m_pSize()) & MASK_SIZE; }
            // フラグを変更せずにメモリサイズを設定
            void    setSize(size_t size) {
                HMD_DEBUG_ASSERT((size & MASK_FLAGS) == 0);
                size_t* p = m_pSize();
                *p &= MASK_FLAGS;
                *p += size; }
            // 中身を 0 で埋める
            void    fillZero(void);
            // Garbage Collection 対象であるフラグを立てる
            void    setGCObject(void);
            bool    isGCObject(void);
            void    unsetGCObject(void);
#ifdef HMD_DEBUG_MEMORY_ID
            char*   memID(void) { return m_pMemID(); }
            void    setMemID(const char* id);
#else
            inline void     setMemID(const char*) {}
#endif

            void    printMarkNode(void);
        };













        // ==================== class MemPool ====================

        class MemPool {
            friend class ::Test_hyMemPool;      // for unittest
            friend class ::Test_hyGC;           // for unittest
            friend class Engine::GC;

        private:
            MemPool(void) {}
        public:
            // MemPoolインスタンスを管理メモリ中に確保し、管理開始
            // addrは4byte align でないといけない
            static MemPool* manage(void* addr, size_t size);

            static void     initGMemPool(void* addr, size_t size);

            static const int DEFAULT_ALERT_LEVEL = 80;

        public:
            // 管理するメモリ領域を設定
            void            initialize(void* addr, size_t size);

            MemCell*        alloc(size_t size); // 4byte alignで確保 
            template<typename T> T* allocT(size_t num) { return (T*)alloc(num * sizeof(T)); }

#ifdef HMD_DEBUG_MEMORY_ID
            MemCell*        alloc(size_t size, const char* id) { MemCell* p = alloc(size); p->setMemID(id); return p; }
            template<typename T> T* allocT(size_t num, const char* id) { MemCell* p = alloc(num * sizeof(T)); p->setMemID(id); return (T*) p;}
#else
            inline MemCell* alloc(size_t size, const char*) { return alloc(size); }
            template<typename T> T* allocT(size_t num, const char*) { return (T*)alloc(num * sizeof(T)); }
#endif


            // リアロケート
            // 同じセル内で確保できればそのまま。
            // 隣のセルがフリーで容量が確保できれば結合。
            // 別セルになるのなら copySize バイトの内容物をコピー。
            // copySizeに-1を渡すと、前のサイズを全コピー。
            MemCell*        realloc(MemCell* pCell, size_t size, size_t copySize = (size_t)-1);
            // T型のオブジェクトがnum個入るような容量でリアロケート
            // copyNumはTをコピーする個数
            template<typename T> T* reallocT(T* pCell, size_t num, size_t copyNum = (size_t)-1) {
                if (copyNum == (size_t)-1)
                    return (T*)realloc((MemCell*)pCell, num * sizeof(T), (size_t)-1);
                else
                    return (T*)realloc((MemCell*)pCell, num * sizeof(T), copyNum * sizeof(T));
            }

            void            free(MemCell* pCell);
            void            free(void* pMem) { free((MemCell*)pMem); }
            void            coalesce(void);  // フリーな隣りあったセルを全部結合

            size_t          manageSize(void) { return ((char*)m_poolEnd) - ((char*)m_poolAddr); }

            // 指定アドレスが管理領域内かどうか
            bool            inside(void* pMem) { return ((pMem >= m_poolAddr) && (pMem < m_poolEnd)); }


            // 使用メモリパーセント
            int     percentUsed(void);
            // 使用メモリ
            int     getUsed(void);

            // 危険レベルを設定
            // 使用メモリパーセントがこれ以上になると、isAlert()がtrueになる
            void    setAlertLevel(int percent);
            // メモリ使用状況が危険レベルかどうか
            bool    isAlertLevel(void);
            // メモリ使用状況がさらに危険レベルかどうか
            bool    isAlertAbsorbLevel(void);
            // sweep終了後に、m_alertAbsorbLevelを再計算
            void    calcAlertAbsorbLevel(void);
            // isAlertAbsorbLevel()がtrueを返す事を許可
            void    enableAbsorbAlert(void) { m_absorbAlertEnableFlag = true; }
            // isAlertAbsorbLevel()がtrueを返す事を禁止
            void    disableAbsorbAlert(void) { m_absorbAlertEnableFlag = false; }


        protected:
            // ジャストフィットなセルを探し、無ければより大きいセルを探して切り出す
            MemCell*        m_alloc_1(size_t size);

            // m_head[ind]のセル pCell を(適切なら)分割してsizeのセルを切り出す
            MemCell*        m_divide(int ind, MemCell* pCell, size_t size);

            // 2つの隣あったフリーなセルを結合
            void            m_coalesce2(MemCell* p, MemCell* q);

        protected:
            void*           m_poolAddr;
            void*           m_poolEnd;
            bool            m_GC_working_in_alloc;


        protected:
            // アリーナ

#define MA_BIN0SIZE        8
#define MA_BIN0LIMIT       128
#define MA_BIN0INDMAX      (MA_BIN0LIMIT/MA_BIN0SIZE)
#define MA_BIN1SIZE        32
#define MA_BIN1LIMIT       512
#define MA_BIN1INDMAX      (MA_BIN0INDMAX + ((MA_BIN1LIMIT - MA_BIN0LIMIT)/MA_BIN1SIZE))
#define MA_BIN2SIZE        128
#define MA_BIN2LIMIT       2048
#define MA_BIN2INDMAX      (MA_BIN1INDMAX + ((MA_BIN2LIMIT - MA_BIN1LIMIT)/MA_BIN2SIZE))
#define MA_NUMBINS         3
#define MA_BINLASTINDMAX   MA_BIN2INDMAX
#define MA_NUM_TOTAL_BINS  (MA_BINLASTINDMAX+1)
            static const size_t     m_binSizes[MA_NUMBINS];
            static const size_t     m_binLimits[MA_NUMBINS+1];
            static const int        m_binIndMax[MA_NUMBINS+1];
            // m_binSize[n]刻みでアリーナのサイズが増えていく。
            // サイズがm_binLimits[n+1]になると次のステージ(++n)。

            // アリーナのサイズが 0x10, 0x18, 0x20, 0x28 .. となっている時、
            // 0x1cのセルをallocしたい時には 0x20 のアリーナから取得し、
            // 0x1cのセルをfreeしたい時には 0x18 のアリーナに追加する

#ifdef m_MEMCELL_JUST_ARENA_SIZE_
            static size_t   m_binIndex2size[MA_NUM_TOTAL_BINS];
            static bool     m_binIndex2size_initialized_flag;
#endif
            // サイズから対応する m_head のインデックスを取得: セルをallocする時
            static int      m_size2binIndex_alloc(size_t size);
            // サイズから対応する m_head のインデックスを取得: セルをfreeする時
            static int      m_size2binIndex_free(size_t size);

            // サイズ別フリーセルリスト
            MemCell*        m_head[MA_NUM_TOTAL_BINS];
            // フリーなセルのサイズ合計
            size_t          m_freeCellSizeTotal;
            // alertになるフリーセルサイズ合計
            size_t          m_alertLevel;
            // alertAbsorbLevelになるフリーセルサイズ合計
            size_t          m_alertAbsorbLevel;
            // これがfalseだとalertAbsorbLevelがfalseを返す
            bool            m_absorbAlertEnableFlag;
#ifdef HMD_DEBUG
            // GC直後の空きメモリセル最大サイズ
            size_t          m_maxFreeCellSize;
#endif
            // セルをアリーナの指定リストに追加
            void            m_attachTo(MemCell* pCell, int ind);
            // セルをアリーナに追加
            void            m_attach(MemCell* pCell) { m_attachTo(pCell, m_size2binIndex_free(pCell->size())); }
            // セルをアリーナの指定リストから外す
            void            m_detachFrom(MemCell* pCell, int ind);
            // セルをアリーナから外す
            void            m_detach(MemCell* pCell) { m_detachFrom(pCell, m_size2binIndex_free(pCell->size())); }



        public:
            // イテレータ

            CellIterator    begin(void) const;
            CellIterator    end(void) const;

            // coalesceする時に、ここに登録したイテレータの示すセルが
            // 統合されたなら、イテレータに次をポイントさせる
            void            registerCellIterator(CellIterator* pItr) { registeredCellIterator = pItr; }

        protected:
            CellIterator*   registeredCellIterator;


        public:
            typedef void (*MemCellPrintFunc_t)(MemCell*);
            MemCellPrintFunc_t      memCellPrintFunc;
            void    printCellInfo(MemCellPrintFunc_t prFunc = NULL);
            void    printFreeCellInfo(void);
            size_t  totalFreeSize(void) { return m_freeCellSizeTotal; } // 正確ではない

#if defined(HMD_DEBUG) || defined(TEST__CPPUNIT)
            int     check_neighbor(void);
            bool    check_free_consistency(MemCell* pHead);
            bool    check(void);
#endif


#ifdef HMD_DEBUG
            // GC直後にのみ正しい数値が返る
            // 最大フリーメモリセルサイズ
            size_t  maxFreeCellSize(void) { return m_maxFreeCellSize; }
#endif

        };


        // coalesceするとポインタが有効か保証できない
        class CellIterator {
        protected:
            MemCell*    p;
        public:
            CellIterator(void) { p = NULL; }
            CellIterator(MemCell* cell) { p = cell; }
            CellIterator(const CellIterator& itr) { p = itr.p; }
            
            MemCell*        operator*() const { return p; }
            void            operator++(int) { p = p->m_pNextNeighbor(); }
            // CellIterator&   operator++(int) { p = p->m_pNextNeighbor(); return *this; }
            bool            operator<(const CellIterator& itr) { return p < itr.p; }
        };


        // グローバルMemPool
        HMD_EXTERN_DECL MemPool* gMemPool;

    }
}

#endif /* m_HYMEMPOOL_H_ */
