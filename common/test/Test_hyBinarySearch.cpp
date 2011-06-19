/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyBinarySearch.h"
#include <cppunit/extensions/HelperMacros.h>


class Test_hyBinarySearch : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyBinarySearch);
    CPPUNIT_TEST(test_search);
    CPPUNIT_TEST(test_searchRange);
    CPPUNIT_TEST_SUITE_END();
public:

    void test_search(void)
    {
        //            0  1  2  3  4   5    6    7     8     9
        int tbl[] = { 1, 4, 7, 8, 9, 11, 100, 101, 2000, 2009 };
        int size = sizeof(tbl)/sizeof(tbl[0]);
        int idx;

        for (int i = 0; i < size; i++) {
            CPPUNIT_ASSERT(binarySearch(tbl,size,tbl[i], &idx));
            CPPUNIT_ASSERT_EQUAL(i, idx);
        }
        CPPUNIT_ASSERT(! binarySearch(tbl,size, -1, &idx));
        CPPUNIT_ASSERT_EQUAL(0, idx);
        CPPUNIT_ASSERT(! binarySearch(tbl,size, 0, &idx));
        CPPUNIT_ASSERT_EQUAL(0, idx);
        CPPUNIT_ASSERT(! binarySearch(tbl,size, 10, &idx));
        CPPUNIT_ASSERT_EQUAL(5, idx);
        CPPUNIT_ASSERT(! binarySearch(tbl,size, 102, &idx));
        CPPUNIT_ASSERT_EQUAL(8, idx);
        CPPUNIT_ASSERT(! binarySearch(tbl,size, 2001, &idx));
        CPPUNIT_ASSERT_EQUAL(9, idx);
        CPPUNIT_ASSERT(! binarySearch(tbl,size, 2010, &idx));
        CPPUNIT_ASSERT_EQUAL(10, idx);
        CPPUNIT_ASSERT(! binarySearch(tbl,size, 10000, &idx));
        CPPUNIT_ASSERT_EQUAL(10, idx);
    }

    void test_searchRange(void)
    {
        //            0  1  2  3  4  5   6   7   8   9  10
        int tbl[] = { 1, 4, 4, 7, 7, 7, 10, 11, 11, 20, 20 };
        int size = sizeof(tbl)/sizeof(tbl[0]);
        int idxMin, idxMax;

        CPPUNIT_ASSERT(! binarySearchRange(tbl, size, 0, &idxMin, &idxMax));
        CPPUNIT_ASSERT_EQUAL(0, idxMin); CPPUNIT_ASSERT_EQUAL(0, idxMax);
        CPPUNIT_ASSERT(binarySearchRange(tbl, size, 1, &idxMin, &idxMax));
        CPPUNIT_ASSERT_EQUAL(0, idxMin); CPPUNIT_ASSERT_EQUAL(0, idxMax);
        CPPUNIT_ASSERT(! binarySearchRange(tbl, size, 2, &idxMin, &idxMax));
        CPPUNIT_ASSERT_EQUAL(1, idxMin); CPPUNIT_ASSERT_EQUAL(1, idxMax);
        CPPUNIT_ASSERT(binarySearchRange(tbl, size, 4, &idxMin, &idxMax));
        CPPUNIT_ASSERT_EQUAL(1, idxMin); CPPUNIT_ASSERT_EQUAL(2, idxMax);
        CPPUNIT_ASSERT(! binarySearchRange(tbl, size, 5, &idxMin, &idxMax));
        CPPUNIT_ASSERT_EQUAL(3, idxMin); CPPUNIT_ASSERT_EQUAL(3, idxMax);
        CPPUNIT_ASSERT(binarySearchRange(tbl, size, 7, &idxMin, &idxMax));
        CPPUNIT_ASSERT_EQUAL(3, idxMin); CPPUNIT_ASSERT_EQUAL(5, idxMax);
        CPPUNIT_ASSERT(! binarySearchRange(tbl, size, 9, &idxMin, &idxMax));
        CPPUNIT_ASSERT_EQUAL(6, idxMin); CPPUNIT_ASSERT_EQUAL(6, idxMax);
        CPPUNIT_ASSERT(binarySearchRange(tbl, size, 10, &idxMin, &idxMax));
        CPPUNIT_ASSERT_EQUAL(6, idxMin); CPPUNIT_ASSERT_EQUAL(6, idxMax);
        CPPUNIT_ASSERT(binarySearchRange(tbl, size, 11, &idxMin, &idxMax));
        CPPUNIT_ASSERT_EQUAL(7, idxMin); CPPUNIT_ASSERT_EQUAL(8, idxMax);
        CPPUNIT_ASSERT(! binarySearchRange(tbl, size, 19, &idxMin, &idxMax));
        CPPUNIT_ASSERT_EQUAL(9, idxMin); CPPUNIT_ASSERT_EQUAL(9, idxMax);
        CPPUNIT_ASSERT(binarySearchRange(tbl, size, 20, &idxMin, &idxMax));
        CPPUNIT_ASSERT_EQUAL(9, idxMin); CPPUNIT_ASSERT_EQUAL(10, idxMax);
        CPPUNIT_ASSERT(! binarySearchRange(tbl, size, 21, &idxMin, &idxMax));
        CPPUNIT_ASSERT_EQUAL(11, idxMin); CPPUNIT_ASSERT_EQUAL(11, idxMax);
    }


};

CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyBinarySearch);
