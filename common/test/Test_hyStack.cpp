/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyStack.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;

class Test_hyStack : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyStack);
    CPPUNIT_TEST(test_hyStack);
    CPPUNIT_TEST_SUITE_END();
public:

    void test_hyStack(void)
    {
        int pool[128];
        MemPool::initGMemPool((void*)pool, sizeof(pool));

        Stack<int> s(3);

        int d1 = 2;
        int d2 = 5;
        int d3 = 123;

        s.push(d1);
        int x = s.pop();
        CPPUNIT_ASSERT_EQUAL(d1, x);

        s.push(d2);
        s.push(d3);
        s.push(d1);
        x = s.pop();
        CPPUNIT_ASSERT_EQUAL(d1, x);
        x = s.pop();
        CPPUNIT_ASSERT_EQUAL(d3, x);
        x = s.pop();
        CPPUNIT_ASSERT_EQUAL(d2, x);

        HMD_ASSERT_HALT(s.pop());

        s.push(d3);
        s.push(d1);
        HMD_ASSERT_NO_HALT(s.push(d1));
        HMD_ASSERT_NO_HALT(s.push(d1));

        x = s.getNth(4);
        CPPUNIT_ASSERT_EQUAL(d3, x);
        HMD_ASSERT_HALT(s.getNth(5));

        HMD_ASSERT_NO_HALT(s.drop(2));
        HMD_ASSERT_HALT(s.drop(3));

        s.clean();
        s.push(d3);
        s.push(d1);
        s.push(d2);
        s.rot(3);
        x = s.pop();
        CPPUNIT_ASSERT_EQUAL(d3, x);
        x = s.pop();
        CPPUNIT_ASSERT_EQUAL(d2, x);
        x = s.pop();
        CPPUNIT_ASSERT_EQUAL(d1, x);


        s.clean();
        s.push(d1);
        s.push(d2);
        s.push(d3);
        s.copyNth(1);
        s.copyNth(3);
        s.copyNth(5);
        x = s.pop();
        CPPUNIT_ASSERT_EQUAL(d1, x);
        x = s.pop();
        CPPUNIT_ASSERT_EQUAL(d2, x);
        x = s.pop();
        CPPUNIT_ASSERT_EQUAL(d3, x);
        s.drop(3);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, s.size());

        s.push(d3);
        s.push(d2);
        s.push(d1);
        s.rot(2);
        CPPUNIT_ASSERT_EQUAL(d2, s.getNth(1));
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(2));
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(3));
        s.rot(3);
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(1));
        CPPUNIT_ASSERT_EQUAL(d2, s.getNth(2));
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(3));
        s.rotr(2);
        CPPUNIT_ASSERT_EQUAL(d2, s.getNth(1));
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(2));
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(3));
        s.rotr(3);
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(1));
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(2));
        CPPUNIT_ASSERT_EQUAL(d2, s.getNth(3));
        s.rotr(3);
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(1));
        CPPUNIT_ASSERT_EQUAL(d2, s.getNth(2));
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(3));

        s.copyNth(3);
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(1));
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(2));
        CPPUNIT_ASSERT_EQUAL(d2, s.getNth(3));
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(4));
        s.copyNth(2);
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(1));
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(2));
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(3));
        CPPUNIT_ASSERT_EQUAL(d2, s.getNth(4));
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(5));
        s.copyNth(1);
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(1));
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(2));
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(3));
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(4));
        CPPUNIT_ASSERT_EQUAL(d2, s.getNth(5));
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(6));
        
        CPPUNIT_ASSERT_EQUAL(d1, s.getAt(5));
        CPPUNIT_ASSERT_EQUAL(d1, s.getAt(4));
        CPPUNIT_ASSERT_EQUAL(d3, s.getAt(3));
        s.insertAt(4, 1);
        s.setAt(4, d2);
        CPPUNIT_ASSERT_EQUAL(d1, s.getAt(6));
        CPPUNIT_ASSERT_EQUAL(d1, s.getAt(5));
        CPPUNIT_ASSERT_EQUAL(d2, s.getAt(4));
        CPPUNIT_ASSERT_EQUAL(d3, s.getAt(3));

        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(1));
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(2));
        CPPUNIT_ASSERT_EQUAL(d2, s.getNth(3));
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(4));

        s.insertAt(0, 3);
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(1));
        CPPUNIT_ASSERT_EQUAL(d1, s.getNth(2));
        CPPUNIT_ASSERT_EQUAL(d2, s.getNth(3));
        CPPUNIT_ASSERT_EQUAL(d3, s.getNth(4));


        s.drop(10);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, s.size());
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyStack);
