/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCellList.h"
#include "hyMemPool.h"
#include "hyThreadManager.h"
#include <cppunit/extensions/HelperMacros.h>


using namespace Hayat::Common;
using namespace Hayat::Engine;

static bool cmpFunc(const int& i, void* p)
{
    return i == (int)p;
}

class Test_hyCellList : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyCellList);
    CPPUNIT_TEST(test_CellList_headtail);
    CPPUNIT_TEST(test_CellList_addremove);
    CPPUNIT_TEST(test_CellList_iterator);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(20480);
        MemPool::initGMemPool(hayatMemory, 20480);
        ThreadManager::firstOfAll();
        gThreadManager.initialize();
    }

    void tearDown(void)
    {
        gThreadManager.finalize();
        HMD_FREE(hayatMemory);
    }

    void test_CellList_headtail(void)
    {
        typedef CellList<int> IntList;

        IntList* p;
        IntList* q;

        p = new IntList(100);
        p = IntList::cons(50, p);
        p = IntList::cons(25, p);
        p = IntList::cons(12, p);

        CPPUNIT_ASSERT_EQUAL(12, p->head());
        q = p->tail();
        delete p;
        p = q;
        CPPUNIT_ASSERT_EQUAL(25, IntList::shift(&p));
        CPPUNIT_ASSERT_EQUAL(50, p->head());
        q = p->tail();
        delete p;
        p = q;
        CPPUNIT_ASSERT_EQUAL(100, IntList::shift(&p));
        CPPUNIT_ASSERT_EQUAL((IntList*)NULL, p);
    }

    void test_CellList_addremove(void)
    {
        CellList<int> root;

        root.add(0);
        root.add(2);
        root.add(3);
        root.next()->insert(1);
        root.add(4);
        root.add(5);
            
        int i;
        CellList<int>* p;

        // 0,1,2,3,4,5
        for (i = 0, p = root.next(); p != NULL; i++, p = p->next()) {
            CPPUNIT_ASSERT_EQUAL(i, p->content());
        }
        CPPUNIT_ASSERT_EQUAL(6, i);

        CPPUNIT_ASSERT(root.isInclude(4));
        CPPUNIT_ASSERT(! root.isInclude(6));

        CPPUNIT_ASSERT(root.remove(2));
        root.next()->next()->next()->removeNext(); // remove(4)
        CPPUNIT_ASSERT(! root.remove(8));
        root.removeNext();          // remove(0)

        // 1,3,5
        for (i = 1, p = root.next(); p != NULL; i+=2, p = p->next()) {
            CPPUNIT_ASSERT_EQUAL(i, p->content());
        }
        CPPUNIT_ASSERT_EQUAL(7, i);
            
        CPPUNIT_ASSERT(root.condRemove(cmpFunc, (void*)3));

        // 1,5
        CPPUNIT_ASSERT(! root.isInclude(3));
        for (i = 1, p = root.next(); p != NULL; i+=4, p = p->next()) {
            CPPUNIT_ASSERT_EQUAL(i, p->content());
        }
    }

    void test_CellList_iterator(void)
    {
        CellList<int>* pRoot = NULL;
        CellList<int>::Iterator itr(&pRoot);

        CPPUNIT_ASSERT(itr.empty());
            
        pRoot = new CellList<int>(0);       // [0]
        CPPUNIT_ASSERT(! itr.empty());
        CPPUNIT_ASSERT_EQUAL(0, itr.get()->content());

        itr.insert(1);                      // [0;1]
        CPPUNIT_ASSERT_EQUAL(0, itr.get()->content());
        CPPUNIT_ASSERT_EQUAL(0, (*itr)->content());

        CellList<int>*p = itr.next();
        CPPUNIT_ASSERT_EQUAL(1, p->content());

        itr.insertPrev(2);                  // [0;2;1]
        CPPUNIT_ASSERT_EQUAL(1, itr.get()->content());

        itr.initialize(&pRoot);
        CPPUNIT_ASSERT_EQUAL(0, itr.get()->content());
        CPPUNIT_ASSERT_EQUAL(2, itr.next()->content());
        p = itr.remove();                   // [0;1]
        CPPUNIT_ASSERT_EQUAL(2, p->content());
        CPPUNIT_ASSERT_EQUAL(1, itr.get()->content());
        delete p;

        p = itr.remove();                   // [0]
        CPPUNIT_ASSERT_EQUAL(1, p->content());
        CPPUNIT_ASSERT((CellList<int>*)NULL == itr.get());
        delete p;

        itr.initialize(&pRoot);
        p = itr.remove();                   // []
        CPPUNIT_ASSERT_EQUAL(0, p->content());
        CPPUNIT_ASSERT((CellList<int>*)NULL == itr.get());
        delete p;
        itr.initialize(&pRoot);
        CPPUNIT_ASSERT((CellList<int>*)NULL == itr.get());
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyCellList);
