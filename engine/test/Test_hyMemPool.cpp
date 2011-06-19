/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyMemPool.h"
#include "hyVarTable.h"
#include "hyException.h"
#include <cppunit/extensions/HelperMacros.h>


using namespace Hayat::Common;
using namespace Hayat::Engine;


#ifdef HMD_DEBUG_MEMORY_ID
#define m_MEMID_SIZE 4
#else
#define m_MEMID_SIZE 0
#endif


class Mock_MemPool : public MemPool {
public:
    static Mock_MemPool* manage(void* addr, size_t size) {
        return (Mock_MemPool*)MemPool::manage(addr, size);
    }

    /*
    void dispCells(void)
    {
        for (int i = 0; i < MA_NUM_TOTAL_BINS; i++) {
            if (m_head[i] != NULL) {
                HMD_PRINTF("m_head[%d]\n",i);
                MemCell* p = m_head[i];
                do {
                    HMD_PRINTF("  %x : %d\n", p, p->size());
                    p = p->m_nextFree;
                } while (p != m_head[i]);
            }
        }
    }
    */
        
    int arena_count_alloc(size_t size)
    {
        MemCell* head = m_head[m_size2binIndex_alloc(size)];
        if (head == NULL) {
            // HMD_PRINTF("arena_count_alloc(%d) -> m_head[%d] -> 0\n", size, m_size2binIndex_alloc(size));
            return 0;
        }

        MemCell* p = head;
        int count = 1;
        while (p->m_nextFree != head) {
            p = p->m_nextFree;
            count++;
        }
        // HMD_PRINTF("arena_count_alloc(%d) -> m_head[%d] -> %d\n", size, m_size2binIndex_alloc(size), count);
        return count;
    }

    int arena_count_free(size_t size)
    {
#ifdef m_MEMCELL_JUST_ARENA_SIZE_
        MemCell* head = m_head[m_size2binIndex_free(m_binIndex2size[m_size2binIndex_alloc(size)])];
#else
        MemCell* head = m_head[m_size2binIndex_free(size)];
#endif
        if (head == NULL) {
            //HMD_PRINTF("arena_count_free(%d) -> m_head[%d] -> 0\n", size, m_size2binIndex_free(size));
            return 0;
        }

        MemCell* p = head;
        int count = 1;
        while (p->m_nextFree != head) {
            p = p->m_nextFree;
            count++;
        }
        //HMD_PRINTF("arena_count_free(%d) -> m_head[%d] -> %d\n", size, m_size2binIndex_free(size), count);
        return count;
    }

};




class Test_hyMemPool : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyMemPool);
    CPPUNIT_TEST(test_MemCell);
    CPPUNIT_TEST(test_MemPool);
    CPPUNIT_TEST(test_realloc);
    CPPUNIT_TEST(test_size2binIndex);
    CPPUNIT_TEST(test_arena);
    CPPUNIT_TEST(test_gcObjFlag);
    CPPUNIT_TEST_SUITE_END();
public:

    void test_MemCell(void)
    {
        char tmp[128];

        MemCell* p = (MemCell*)&tmp[32];
        *(p->m_pSize()) = 64;
        CPPUNIT_ASSERT_EQUAL((size_t)64, p->size());
        CPPUNIT_ASSERT(! p->m_bPrevFree());

        *(p->m_pSize()) = 64 | MemCell::MASK_PREV_FREE;
        CPPUNIT_ASSERT_EQUAL((size_t)64, p->size());
        CPPUNIT_ASSERT(p->m_bPrevFree());
        CPPUNIT_ASSERT_EQUAL((size_t*)&tmp[32 - sizeof(size_t) - m_MEMID_SIZE], 
                             p->m_pSize());
        CPPUNIT_ASSERT_EQUAL((size_t*)&tmp[32 - sizeof(size_t) * 2 - m_MEMID_SIZE], 
                             p->m_pPrevSize());
        CPPUNIT_ASSERT_EQUAL((MemCell*)&tmp[32 + 64 + sizeof(size_t) + m_MEMID_SIZE],
                             p->m_pNextNeighbor());

        p->m_setFree();
        CPPUNIT_ASSERT(p->m_bFree());
        CPPUNIT_ASSERT_EQUAL((size_t)64, *(p->m_pNextNeighborsPrevSize()));

        p->m_setAlloc();
        CPPUNIT_ASSERT(! p->m_bFree());
        CPPUNIT_ASSERT(! p->isGCObject());
        p->setGCObject();
        CPPUNIT_ASSERT(p->isGCObject());
    }


    void test_MemPool(void)
    {
        char tmp[2048+sizeof(MemPool)];
    
        MemPool* orgMemPool = gMemPool;
        MemPool::initGMemPool(tmp, 2048+sizeof(MemPool));
        MemPool* pool = gMemPool;
        gGlobalVar.initialize();

        // global var(2) と free(1) で3つ
        CPPUNIT_ASSERT_EQUAL(3, pool->check_neighbor());

        CPPUNIT_ASSERT(pool->check());
        HMD_ASSERT_HALT(pool->alloc(2050));
        MemCell* c1 = pool->alloc(17);
#ifdef m_MEMCELL_JUST_ARENA_SIZE_
        CPPUNIT_ASSERT_EQUAL(pool->m_binIndex2size[pool->m_size2binIndex_alloc(17)], c1->size());
#else
        CPPUNIT_ASSERT_EQUAL((size_t)20, c1->size()); // 4バイトalign
#endif
        CPPUNIT_ASSERT(! c1->m_bFree());
        CPPUNIT_ASSERT(pool->check());
        MemCell* c2 = pool->alloc(3);
        size_t m_minimumFreecellSize = MemCell::MINIMUM_FREECELL_SIZE;
        CPPUNIT_ASSERT_EQUAL(m_minimumFreecellSize, c2->size());
        CPPUNIT_ASSERT(! c2->m_bFree());
        CPPUNIT_ASSERT(pool->check());
        MemCell* c3 = pool->alloc(16);
        CPPUNIT_ASSERT(! c3->m_bFree());
        CPPUNIT_ASSERT(pool->check());
        MemCell* c4 = pool->alloc(18);
        CPPUNIT_ASSERT(! c4->m_bFree());
        CPPUNIT_ASSERT(pool->check());
        MemCell* c5 = pool->alloc(19);
        CPPUNIT_ASSERT(! c5->m_bFree());
        CPPUNIT_ASSERT(pool->check());
        MemCell* c6 = pool->alloc(22);
        CPPUNIT_ASSERT(c6 != (MemCell*)NULL);
        CPPUNIT_ASSERT(! c6->m_bFree());
        CPPUNIT_ASSERT(pool->check());
        HMD_ASSERT_HALT(pool->alloc(2048));

        CPPUNIT_ASSERT_EQUAL(9, pool->check_neighbor());
        pool->free(c6);              // 後ろと結合 
        CPPUNIT_ASSERT(c6->m_bFree());
        CPPUNIT_ASSERT(pool->check());
        pool->coalesce();
        CPPUNIT_ASSERT_EQUAL(8, pool->check_neighbor());
        CPPUNIT_ASSERT(pool->check());
        pool->free(c4);              // 挿入 
        CPPUNIT_ASSERT(c4->m_bFree());
        CPPUNIT_ASSERT(pool->check());
        pool->coalesce();
        CPPUNIT_ASSERT_EQUAL(8, pool->check_neighbor());
        CPPUNIT_ASSERT(pool->check());
        pool->free(c5);              // 前後と結合 
        CPPUNIT_ASSERT(c5->m_bFree());
        CPPUNIT_ASSERT(pool->check());
        pool->coalesce();
        CPPUNIT_ASSERT_EQUAL(6, pool->check_neighbor());
        CPPUNIT_ASSERT(pool->check());
        pool->free(c1);              // 挿入 
        CPPUNIT_ASSERT(c1->m_bFree());
        CPPUNIT_ASSERT(pool->check());
        pool->coalesce();
        CPPUNIT_ASSERT_EQUAL(6, pool->check_neighbor());
        CPPUNIT_ASSERT(pool->check());
        pool->free(c2);              // 前と結合 
        CPPUNIT_ASSERT(c2->m_bFree());
        CPPUNIT_ASSERT(pool->check());
        pool->coalesce();
        CPPUNIT_ASSERT_EQUAL(5, pool->check_neighbor());
        CPPUNIT_ASSERT(pool->check());
        pool->free(c3);              // 前後と結合
        CPPUNIT_ASSERT(c3->m_bFree());
        CPPUNIT_ASSERT(pool->check());
        pool->coalesce();
        CPPUNIT_ASSERT_EQUAL(3, pool->check_neighbor());
        CPPUNIT_ASSERT(pool->check());

        c1 = pool->alloc(512);
        CPPUNIT_ASSERT(c1 != NULL);
        c2 = pool->alloc(512);
        CPPUNIT_ASSERT(c2 != NULL);
        c3 = pool->alloc(768);
        CPPUNIT_ASSERT(c3 != NULL);
        pool->free(c2);
        c4 = pool->alloc(512); // c2の領域をもう一度確保
        CPPUNIT_ASSERT(c4 != (MemCell*)NULL);
        CPPUNIT_ASSERT_EQUAL(c2, c4);
        CPPUNIT_ASSERT(pool->check());
        pool->free(c1);
        pool->free(c4);
        pool->free(c3);
        CPPUNIT_ASSERT(pool->check());

        gMemPool = orgMemPool;
    }

    void test_realloc(void)
    {
        char tmp[2048+sizeof(MemPool)];
    
        MemPool* orgMemPool = gMemPool;
        MemPool::initGMemPool(tmp, 2048+sizeof(MemPool));
        MemPool* pool = gMemPool;
        gGlobalVar.initialize();

        // global var(2) と free(1) で3つ
        CPPUNIT_ASSERT_EQUAL(3, pool->check_neighbor());

        CPPUNIT_ASSERT(pool->check());
        MemCell* c1 = pool->alloc(32);
        MemCell* c2 = pool->alloc(24);
        MemCell* c3 = pool->alloc(48);

        CPPUNIT_ASSERT(c2 == c1->m_pNextNeighbor());
        CPPUNIT_ASSERT(c3 == c2->m_pNextNeighbor());

        // realloc不要
        CPPUNIT_ASSERT(c2 == pool->realloc(c2, 24));

        // 別cellにrealloc
        MemCell* c5 = pool->realloc(c2, 26);
        CPPUNIT_ASSERT(c2 != c5);
        CPPUNIT_ASSERT(c2->m_bFree());

        // c1とc2を結合
        MemCell* c6 = pool->realloc(c1, 32+24+1);
        CPPUNIT_ASSERT(c1 == c6);
        CPPUNIT_ASSERT(c3 == c1->m_pNextNeighbor());

        CPPUNIT_ASSERT(pool->check());

        gMemPool = orgMemPool;
    }

    void test_size2binIndex(void)
    {
        // MemPool::initialize()呼び出し ← m_binIndex2size[] 初期化のため
        char tmp[128+sizeof(Mock_MemPool)];
        Mock_MemPool* pool = Mock_MemPool::manage(tmp, 128+sizeof(Mock_MemPool));
        (void)pool;


        int ind_a = MemCell::MINIMUM_FREECELL_SIZE / MemPool::m_binSizes[0];
        int ind_f = ind_a;
        for (size_t i = 8; i < MemPool::m_binLimits[MA_NUMBINS] + MemPool::m_binSizes[MA_NUMBINS-1] + 16; i += 4) {
            int j = MemPool::m_size2binIndex_alloc(i);
            int k = MemPool::m_size2binIndex_free(i);
            CPPUNIT_ASSERT((k == j) || (k == j-1));
            CPPUNIT_ASSERT((ind_a == j) || (ind_a + 1 == j));
            ind_a = j;
            CPPUNIT_ASSERT((ind_f == k) || (ind_f + 1 == k));
            ind_f = k;
        }
        CPPUNIT_ASSERT_EQUAL(MemPool::m_binIndMax[MA_NUMBINS], ind_a);
        CPPUNIT_ASSERT_EQUAL(MemPool::m_binIndMax[MA_NUMBINS], ind_f);

#ifdef m_MEMCELL_JUST_ARENA_SIZE_
        for (int i = MemCell::MINIMUM_FREECELL_SIZE / MemPool::m_binSizes[0]; i < MA_NUM_TOTAL_BINS; i++) {
            CPPUNIT_ASSERT_EQUAL(i, MemPool::m_size2binIndex_free(MemPool::m_binIndex2size[i]));
        }
#endif

        size_t l = MemPool::m_binSizes[0] * 3;
        size_t h = MemPool::m_binSizes[0] * 4;
        HMD_ASSERT(h < MemPool::m_binLimits[1]);
        size_t k = (l + h) / 2;
        HMD_ASSERT((l < k) && (k < h));
        CPPUNIT_ASSERT(MemPool::m_size2binIndex_alloc(l) == MemPool::m_size2binIndex_free(l));
        CPPUNIT_ASSERT(MemPool::m_size2binIndex_alloc(h) == MemPool::m_size2binIndex_free(h));
        CPPUNIT_ASSERT(MemPool::m_size2binIndex_alloc(k) == MemPool::m_size2binIndex_alloc(h));
        CPPUNIT_ASSERT(MemPool::m_size2binIndex_free(k) == MemPool::m_size2binIndex_free(l));
    }

    void test_arena(void)
    {
        int memsize = MemPool::m_binLimits[MA_NUMBINS] * 2;
        // HMD_PRINTF("test_arena memsize = %d\n", memsize);
        void* tmp = HMD_ALLOC(memsize);

        Mock_MemPool* pool = Mock_MemPool::manage(tmp, memsize);
        //pool->dispCells();
        MemCell* c1 = pool->alloc(40);
        MemCell* c2 = pool->alloc(80);
        MemCell* c3 = pool->alloc(40);
        MemCell* c4 = pool->alloc(120);
        MemCell* c5 = pool->alloc(99);
        MemCell* c6 = pool->alloc(MemPool::m_binLimits[MA_NUMBINS]);

        //pool->dispCells();
        pool->free(c1);
        CPPUNIT_ASSERT_EQUAL(1, pool->arena_count_free(40));
        //pool->dispCells();

        int k = pool->arena_count_free(80);
        pool->free(c2);
        CPPUNIT_ASSERT_EQUAL(k+1, pool->arena_count_free(80));
        //pool->dispCells();

        k = pool->arena_count_free(40);
        pool->free(c3);
        CPPUNIT_ASSERT_EQUAL(k+1, pool->arena_count_free(40));
        //pool->dispCells();

        k = pool->arena_count_free(120);
        pool->free(c4);
        CPPUNIT_ASSERT_EQUAL(k+1, pool->arena_count_free(120));
        //pool->dispCells();

        k = pool->arena_count_free(99);
        pool->free(c5);
        CPPUNIT_ASSERT_EQUAL(k+1, pool->arena_count_free(99));
        //pool->dispCells();

        k = pool->arena_count_free(MemPool::m_binLimits[MA_NUMBINS]);
        pool->free(c6);
        CPPUNIT_ASSERT_EQUAL(k+1, pool->arena_count_free(MemPool::m_binLimits[MA_NUMBINS]));
        //pool->dispCells();

        pool->coalesce();
        //pool->dispCells();
        CPPUNIT_ASSERT_EQUAL(1, pool->check_neighbor());
        CPPUNIT_ASSERT(pool->check());

        HMD_FREE(tmp);
    }

    void test_gcObjFlag(void)
    {
        char tmp[2048+sizeof(MemPool)];
    
        MemPool* pool = MemPool::manage(tmp, 2048+sizeof(MemPool));

        MemCell* c1 = pool->alloc(20);
        *(int*)c1 = 1;

        MemCell* c2 = pool->alloc(20);
        c2->setGCObject();
        *(int*)c2 = 2;

        MemCell* c3 = pool->alloc(20);
        *(int*)c3 = 3;

        MemCell* c4 = pool->alloc(20);
        c4->setGCObject();
        *(int*)c4 = 4;


        MemCell* c5 = pool->alloc(20);
        *(int*)c5 = 5;
        c5->setGCObject();

        MemCell* c6 = pool->alloc(20);
        *(int*)c6 = 6;

        MemCell* c7 = pool->alloc(20);
        *(int*)c7 = 7;


        int gcCount = 0;
        int nCount = 0;
        for (CellIterator itr = pool->begin(); itr < pool->end(); itr++) {
            MemCell* p = *itr;
            if (p->isGCObject()) {
                gcCount++;
                int n = *(int*)p;
                CPPUNIT_ASSERT((n == 2) || (n == 4) || (n == 5));
            } else {
                nCount++;
            }
        }
        CPPUNIT_ASSERT(gcCount == 3);
        CPPUNIT_ASSERT(nCount == 5);
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyMemPool);
