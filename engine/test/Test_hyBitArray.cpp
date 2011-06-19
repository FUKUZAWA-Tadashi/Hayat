/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyBitArray.h"
#include "hyMemPool.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;

class Test_BitArray : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_BitArray);
    CPPUNIT_TEST(test_bitArray);
    CPPUNIT_TEST_SUITE_END();
public:
    Test_BitArray(void) {}


    void* hayatMemory;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(10240);
        MemPool::initGMemPool(hayatMemory, 10240);
    }

    void tearDown(void)
    {
        HMD_FREE(hayatMemory);
    }

    void test_bitArray(void)
    {
        BitArray ba(20);

        CPPUNIT_ASSERT_EQUAL(20, ba.size());
        ba.changeSize(100);
        CPPUNIT_ASSERT_EQUAL(100, ba.size());
        ba.changeSize(50);
        CPPUNIT_ASSERT_EQUAL(50, ba.size());

        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(10));
        ba.setAt(10, true);
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(10));
        ba.setAt(10, false);
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(10));

        ba.setAll(true);
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(30));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(40));
        ba.setRange(10, 35, false);
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(9));
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(10));
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(35));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(36));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(40));

        ba.insertAt(20, true);
        CPPUNIT_ASSERT_EQUAL(51, ba.size());
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(20));
        CPPUNIT_ASSERT_EQUAL(false, ba.getAt(36));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(37));
        CPPUNIT_ASSERT_EQUAL(true, ba.getAt(41));
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_BitArray);
