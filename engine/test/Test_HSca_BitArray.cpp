/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hayat.h"
#include "hyBitArray.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;

class Test_hyBitArray : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyBitArray);
    CPPUNIT_TEST(test_hyBitArray);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;
    
    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(100000);
        initMemory(hayatMemory, 100000);
        initStdlib();
    }

    void tearDown(void)
    {
        finalizeAll();
        HMD_FREE(hayatMemory);
    }

    void test_hyBitArray(void)
    {
        BitArray ba(0);
        CPPUNIT_ASSERT_EQUAL(0, ba.size());
        CPPUNIT_ASSERT_EQUAL(0, ba.m_memSize);
        CPPUNIT_ASSERT(NULL == ba.m_memory);

        ba.changeSize(70);
        CPPUNIT_ASSERT_EQUAL(70, ba.size());
        CPPUNIT_ASSERT(0 != ba.m_memSize);
        CPPUNIT_ASSERT(NULL != ba.m_memory);

        ba.setAt(10, true);
        ba.setAt(30, true);
        ba.setAt(50, true);
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(10));
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(20));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(30));
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(40));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(50));

        ba.setAt(20, true);
        ba.setAt(30, false);
        ba.setAt(40, true);
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(10));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(20));
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(30));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(40));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(50));

        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(34));
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(35));
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(45));
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(60));
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(61));
        ba.setRange(35,60, true);
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(34));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(35));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(45));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(60));
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(61));
        
        BitArray ba2(ba);
        CPPUNIT_ASSERT(ba.isSame(ba2));

        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(60));
        ba2.changeSize(50);
        CPPUNIT_ASSERT_EQUAL(50, ba2.size());
        ba2.changeSize(70);
        CPPUNIT_ASSERT(! ba.isSame(ba2));
        CPPUNIT_ASSERT_EQUAL(false, ba2.getAt(60));
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyBitArray);
