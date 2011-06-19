/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyArray.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;

class Test_Array : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_Array);
    CPPUNIT_TEST(test_array);
    CPPUNIT_TEST(test_insert_remove);
    CPPUNIT_TEST(test_deleteVal);
    CPPUNIT_TEST(test_replace);
    CPPUNIT_TEST(test_external_data);
    CPPUNIT_TEST_SUITE_END();
public:
    Test_Array(void) {}

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

    void test_array(void)
    {
        int dat[10] = { 100, 110, 120, -130, 140, -15000, 16000, 1700, 18, 19000 };
        TArray<int> arr;

        arr.initialize(0);
        CPPUNIT_ASSERT_EQUAL((int*)NULL, arr.m_contents);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, arr.size());
        CPPUNIT_ASSERT_EQUAL((hyu32)0, arr.m_capacity);

        arr.initialize(2);
        CPPUNIT_ASSERT(arr.m_contents != NULL);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, arr.size());
        CPPUNIT_ASSERT_EQUAL((hyu32)2, arr.m_capacity);
        arr.add(dat, 5);
        CPPUNIT_ASSERT_EQUAL((hyu32)5, arr.size());
        CPPUNIT_ASSERT_EQUAL(100, arr[0]);
        CPPUNIT_ASSERT_EQUAL(140, arr[4]);
        arr.fill(7);
        CPPUNIT_ASSERT_EQUAL(7, arr[0]);
        CPPUNIT_ASSERT_EQUAL(7, arr[4]);

        CPPUNIT_ASSERT(arr.checkIndex(100));
        CPPUNIT_ASSERT(arr.checkIndex(-5));
        CPPUNIT_ASSERT(! arr.checkIndex(-6));

        arr[2] = 222;
        CPPUNIT_ASSERT_EQUAL(222, *(arr.nthAddr(2)));

        arr.subst(20, 202020, -1);
        CPPUNIT_ASSERT_EQUAL((hyu32)21, arr.size());
        CPPUNIT_ASSERT_EQUAL(-1, arr[5]);
        CPPUNIT_ASSERT_EQUAL(-1, arr[19]);
        CPPUNIT_ASSERT_EQUAL(202020, arr[20]);

        arr.add(&dat[5], 5);
        arr.add(123);
        arr.add(456);
        CPPUNIT_ASSERT_EQUAL((hyu32)28, arr.size());
        CPPUNIT_ASSERT_EQUAL(-15000, arr[21]);
        CPPUNIT_ASSERT_EQUAL(19000, arr[25]);
        CPPUNIT_ASSERT_EQUAL(123, arr[26]);
        CPPUNIT_ASSERT_EQUAL(456, arr[27]);

        int x = 222;
        CPPUNIT_ASSERT_EQUAL(2, arr.issue(x));
        x = 202020;
        CPPUNIT_ASSERT_EQUAL(20, arr.issue(x));
        x = 123;
        CPPUNIT_ASSERT_EQUAL(26, arr.issue(x));
        CPPUNIT_ASSERT_EQUAL((hyu32)28, arr.size());
        x = 98765;
        CPPUNIT_ASSERT_EQUAL(28, arr.issue(x));
        CPPUNIT_ASSERT_EQUAL(98765, arr[28]);

        arr.clear();
        CPPUNIT_ASSERT_EQUAL((hyu32)0, arr.size());

        hyu32 capa = arr.m_capacity;
        arr.reserve(capa);
        CPPUNIT_ASSERT(capa != arr.m_capacity);
    }


    void test_insert_remove(void)
    {
        TArray<int> arr;

        arr.initialize(2);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, arr.size());
        HMD_ASSERT_HALT(arr.insert(1));
        arr.insert(0) = 10;
        CPPUNIT_ASSERT_EQUAL((hyu32)1, arr.size());
        CPPUNIT_ASSERT_EQUAL(10, arr[0]);
        arr.insert(0) = 20;
        CPPUNIT_ASSERT_EQUAL((hyu32)2, arr.size());
        CPPUNIT_ASSERT_EQUAL(20, arr[0]);
        CPPUNIT_ASSERT_EQUAL(10, arr[1]);
        arr.insert(1) = 30;
        CPPUNIT_ASSERT_EQUAL((hyu32)3, arr.size());
        CPPUNIT_ASSERT_EQUAL(20, arr[0]);
        CPPUNIT_ASSERT_EQUAL(30, arr[1]);
        CPPUNIT_ASSERT_EQUAL(10, arr[2]);
        arr.insert(3) = 40;
        CPPUNIT_ASSERT_EQUAL((hyu32)4, arr.size());
        CPPUNIT_ASSERT_EQUAL(20, arr[0]);
        CPPUNIT_ASSERT_EQUAL(30, arr[1]);
        CPPUNIT_ASSERT_EQUAL(10, arr[2]);
        CPPUNIT_ASSERT_EQUAL(40, arr[3]);
        arr.insert(-1) = 50;
        CPPUNIT_ASSERT_EQUAL((hyu32)5, arr.size());
        CPPUNIT_ASSERT_EQUAL(20, arr[0]);
        CPPUNIT_ASSERT_EQUAL(30, arr[1]);
        CPPUNIT_ASSERT_EQUAL(10, arr[2]);
        CPPUNIT_ASSERT_EQUAL(50, arr[3]);
        CPPUNIT_ASSERT_EQUAL(40, arr[4]);

        arr.remove(2);
        CPPUNIT_ASSERT_EQUAL((hyu32)4, arr.size());
        CPPUNIT_ASSERT_EQUAL(20, arr[0]);
        CPPUNIT_ASSERT_EQUAL(30, arr[1]);
        CPPUNIT_ASSERT_EQUAL(50, arr[2]);
        CPPUNIT_ASSERT_EQUAL(40, arr[3]);
        arr.remove(-3);
        CPPUNIT_ASSERT_EQUAL((hyu32)3, arr.size());
        CPPUNIT_ASSERT_EQUAL(20, arr[0]);
        CPPUNIT_ASSERT_EQUAL(50, arr[1]);
        CPPUNIT_ASSERT_EQUAL(40, arr[2]);
        HMD_ASSERT_HALT(arr.remove(3));
        HMD_ASSERT_HALT(arr.remove(-4));

        int* p = arr.nthAddr(3);
        int* q = arr.addSpaces(2);
        CPPUNIT_ASSERT_EQUAL((hyu32)5, arr.size());
        CPPUNIT_ASSERT_EQUAL(p, q);
    }

    void test_deleteVal(void)
    {
        TArray<int> arr;

        arr.initialize(10);
        for (int i = 0; i < 10; ++i)
            arr.add(i * 10);
        arr[3] = 80;
        arr[6] = 80;
        CPPUNIT_ASSERT_EQUAL((hyu32)10, arr.size());
        arr.deleteVal(20);
        CPPUNIT_ASSERT_EQUAL((hyu32)9, arr.size());
        arr.deleteVal(80);
        CPPUNIT_ASSERT_EQUAL((hyu32)6, arr.size());
        CPPUNIT_ASSERT_EQUAL(0, arr[0]);
        CPPUNIT_ASSERT_EQUAL(10, arr[1]);
        CPPUNIT_ASSERT_EQUAL(40, arr[2]);
        CPPUNIT_ASSERT_EQUAL(50, arr[3]);
        CPPUNIT_ASSERT_EQUAL(70, arr[4]);
        CPPUNIT_ASSERT_EQUAL(90, arr[5]);
    }

    void test_replace(void)
    {
        TArray<int> arr;
        arr.initialize(10);
        for (int i = 0; i < 10; ++i)
            arr.add(i * 10);
        CPPUNIT_ASSERT_EQUAL(30, arr.replace(3, -10));
        CPPUNIT_ASSERT_EQUAL(50, arr.replace(5, -10));
        CPPUNIT_ASSERT_EQUAL(80, arr.replace(8, 800));
        CPPUNIT_ASSERT_EQUAL(-10, arr.replace(5, 10));
        CPPUNIT_ASSERT_EQUAL(-10, arr.replace(3, 10));
        CPPUNIT_ASSERT_EQUAL(800, arr.replace(8, 10));
        CPPUNIT_ASSERT_EQUAL((hyu32)10, arr.size());
        arr.deleteVal(10);
        CPPUNIT_ASSERT_EQUAL((hyu32)6, arr.size());
    }

    void test_external_data(void)
    {
        // 外部参照状態から通常状態に移行する関数のテスト

        TArray<int> arr(0);

        int data[8] = { 1, 11, 21, 31, 41, -1, -1, -1 };
        int x;

        arr.initialize(data, 5);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, arr.capacity());
        CPPUNIT_ASSERT_EQUAL((hyu32)5, arr.size());
        CPPUNIT_ASSERT(data == arr.top());
        CPPUNIT_ASSERT_EQUAL(21, arr[2]);
        arr.subst(2, 201, 999);
        CPPUNIT_ASSERT(arr.capacity() >= 5);
        CPPUNIT_ASSERT_EQUAL((hyu32)5, arr.size());
        CPPUNIT_ASSERT(data != arr.top());
        CPPUNIT_ASSERT_EQUAL(201, arr[2]);
        CPPUNIT_ASSERT_EQUAL(21, data[2]);
        arr.finalize();
        
        arr.initialize(data, 5);
        arr.insert(1,1) = 88;
        CPPUNIT_ASSERT_EQUAL((hyu32)6, arr.size());
        CPPUNIT_ASSERT(arr.capacity() >= 6);
        CPPUNIT_ASSERT_EQUAL(88, arr[1]);
        CPPUNIT_ASSERT_EQUAL(11, arr[2]);
        arr.finalize();

        arr.initialize(data, 5);
        arr.remove(3);
        CPPUNIT_ASSERT_EQUAL(41, arr[3]);
        CPPUNIT_ASSERT_EQUAL(31, data[3]);
        arr.finalize();

        arr.initialize(data, 5);
        arr.fill(99);
        CPPUNIT_ASSERT_EQUAL(99, arr[1]);
        CPPUNIT_ASSERT_EQUAL(11, data[1]);
        arr.finalize();
        
        arr.initialize(data, 5);
        arr.clear();
        CPPUNIT_ASSERT(NULL == arr.top());
        arr.finalize();

        arr.initialize(data, 5);
        arr.addSpaces(2);
        CPPUNIT_ASSERT_EQUAL((hyu32)7, arr.size());
        CPPUNIT_ASSERT(arr.capacity() >= 7);
        arr.finalize();

        arr.initialize(data, 5);
        x = 21;
        CPPUNIT_ASSERT_EQUAL((hys32)2, arr.issue(x));
        CPPUNIT_ASSERT(data == arr.top());
        x = 55;
        CPPUNIT_ASSERT_EQUAL((hys32)5, arr.issue(x));
        CPPUNIT_ASSERT(data != arr.top());
        arr.finalize();

        CPPUNIT_ASSERT_EQUAL( 1, data[0]);
        CPPUNIT_ASSERT_EQUAL(11, data[1]);
        CPPUNIT_ASSERT_EQUAL(21, data[2]);
        CPPUNIT_ASSERT_EQUAL(31, data[3]);
        CPPUNIT_ASSERT_EQUAL(41, data[4]);
        CPPUNIT_ASSERT_EQUAL(-1, data[5]);
        CPPUNIT_ASSERT_EQUAL(-1, data[6]);
        CPPUNIT_ASSERT_EQUAL(-1, data[7]);
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Array);
