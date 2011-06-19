/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyBMap.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;

class Test_BMap : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_BMap);
    CPPUNIT_TEST(test_map);
    CPPUNIT_TEST(test_forceAdd);
    CPPUNIT_TEST_SUITE_END();
public:
    Test_BMap(void) {}

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

    void test_map(void)
    {
        BMap<int,int> map;

        CPPUNIT_ASSERT_EQUAL((hyu32)0, map.size());
        map[100] = 10100;
        map[10] = 10010;
        map[77] = 10077;
        CPPUNIT_ASSERT_EQUAL((hyu32)3, map.size());
        CPPUNIT_ASSERT_EQUAL(10077, map[77]);
        CPPUNIT_ASSERT_EQUAL(10100, map[100]);
        CPPUNIT_ASSERT_EQUAL(10010, map[10]);
        map[55] = -55;
        CPPUNIT_ASSERT_EQUAL((hyu32)4, map.size());
        CPPUNIT_ASSERT_EQUAL(-55, map[55]);
        const TArray<int>& keys = map.keys();
        CPPUNIT_ASSERT_EQUAL(10, keys.nth(0));
        CPPUNIT_ASSERT_EQUAL(55, keys.nth(1));
        CPPUNIT_ASSERT_EQUAL(77, keys.nth(2));
        CPPUNIT_ASSERT_EQUAL(100, keys.nth(3));

        map[55] = 5555;
        CPPUNIT_ASSERT_EQUAL(5555, map[55]);

        const TArray<int>& values = map.values();
        CPPUNIT_ASSERT_EQUAL(values.nthAddr(2), map.find(77));
        CPPUNIT_ASSERT_EQUAL(values.nthAddr(1), map.find(55));
        CPPUNIT_ASSERT_EQUAL((int*)NULL, map.find(66));
        CPPUNIT_ASSERT_EQUAL((hyu32)4, map.size());

        CPPUNIT_ASSERT(!map.remove(444));
        CPPUNIT_ASSERT(map.remove(55));
        CPPUNIT_ASSERT_EQUAL((hyu32)3, map.size());
        CPPUNIT_ASSERT_EQUAL(10, keys.nth(0));
        CPPUNIT_ASSERT_EQUAL(77, keys.nth(1));
        CPPUNIT_ASSERT_EQUAL(100, keys.nth(2));
        CPPUNIT_ASSERT_EQUAL(10010, values.nth(0));
        CPPUNIT_ASSERT_EQUAL(10077, values.nth(1));
        CPPUNIT_ASSERT_EQUAL(10100, values.nth(2));
    }

    void test_forceAdd(void)
    {
        BMap<int,int> map;

        map[1] = 1;
        map[10] = 2;
        map[100] = 3;

        CPPUNIT_ASSERT_EQUAL((hyu32)3, map.size());
        map[10] = 4;
        CPPUNIT_ASSERT_EQUAL((hyu32)3, map.size());
        map.forceAdd(10, 5);
        CPPUNIT_ASSERT_EQUAL((hyu32)4, map.size());
        map.forceAdd(10, -6);
        CPPUNIT_ASSERT_EQUAL((hyu32)5, map.size());
        TArray<int>& a = map.values();
        CPPUNIT_ASSERT_EQUAL((hyu32)5, a.size());
        CPPUNIT_ASSERT_EQUAL(1, a[0]);
        CPPUNIT_ASSERT_EQUAL(4, a[1]);
        CPPUNIT_ASSERT_EQUAL(5, a[2]);
        CPPUNIT_ASSERT_EQUAL(-6, a[3]);
        CPPUNIT_ASSERT_EQUAL(3, a[4]);
    }


};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_BMap);
