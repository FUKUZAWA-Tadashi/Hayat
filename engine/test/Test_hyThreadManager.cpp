/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyThreadManager.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;

class Test_hyThreadManager : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyThreadManager);
    CPPUNIT_TEST(test_nextId);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(20480);
        MemPool::initGMemPool(hayatMemory, 20480);
    }

    void tearDown(void)
    {
        HMD_FREE(hayatMemory);
    }

    void test_nextId(void)
    {
        ThreadManager::firstOfAll();
        ThreadManager mgr;
        mgr.initialize();

        mgr.m_tbl[0].threadObj = (Object*)100;
        mgr.m_tbl[0].threadID = 3;
        mgr.m_tbl[1].threadObj = (Object*)100;
        mgr.m_tbl[1].threadID = 5;
        CPPUNIT_ASSERT_EQUAL(0, (int)mgr.m_lastId);
        mgr.m_nextId();
        CPPUNIT_ASSERT_EQUAL(1, (int)mgr.m_lastId);
        mgr.m_nextId();
        CPPUNIT_ASSERT_EQUAL(2, (int)mgr.m_lastId);
        mgr.m_nextId();  // skip 3
        CPPUNIT_ASSERT_EQUAL(4, (int)mgr.m_lastId);
        mgr.m_nextId();  // skip 5
        CPPUNIT_ASSERT_EQUAL(6, (int)mgr.m_lastId);
        mgr.m_nextId();
        CPPUNIT_ASSERT_EQUAL(7, (int)mgr.m_lastId);

        mgr.m_tbl[1].threadID = 1;
        mgr.m_lastId = 32767;
        mgr.m_nextId();  // 32767+1 -> -1 -> 1  : but skip 1
        CPPUNIT_ASSERT_EQUAL(2, (int)mgr.m_lastId);

        // mgr.finalize() でエラーにならないように
        // インチキThreadオブジェクトは消しておく
        mgr.m_tbl[0].threadObj = NULL;
        mgr.m_tbl[1].threadObj = NULL;
        mgr.finalize();
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyThreadManager);
