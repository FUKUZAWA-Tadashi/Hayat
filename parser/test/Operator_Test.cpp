/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hpOperator.h"
#include "hpParser.h"
#include "hpInputBuffer.h"
#include <cppunit/extensions/HelperMacros.h>


using namespace Hayat::Parser;


class Operator_Test : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Operator_Test);
    CPPUNIT_TEST(test_op);
    CPPUNIT_TEST_SUITE_END();
public:
    Operator_Test(void) {}
        
    StringInputBuffer* inp;
    Operator* op;
    void* houkenMemory;

    void setUp(void)
    {
        houkenMemory = HMD_ALLOC(1024 * 16);
        MemPool::initGMemPool(houkenMemory, 1024 * 16);
        inp = new StringInputBuffer("asd qwe +-*/");
        gpInp = inp;
        Parser::initialize();
        op = new Operator();
    }

    void tearDown(void)
    {
        delete op;
        Parser::finalize();
        delete inp;
        HMD_FREE(houkenMemory);
    }

    void test_op(void)
    {
        Substr asd(0,3);
        Substr qwe(4,7);
        Substr plus(8,9);
        Substr minus(9,10);
        Substr mul(10,11);
        Substr div(11,12);
        CPPUNIT_ASSERT(! op->isOperand(asd));
        CPPUNIT_ASSERT(! op->isOperand(qwe));

        //CPPUNIT_ASSERT(! op->entry(asd, 0)); // error print
        CPPUNIT_ASSERT(op->entry(asd, 10));
        //CPPUNIT_ASSERT(! op->entry(asd, 10)); // error print
        CPPUNIT_ASSERT(op->entry(asd, 30));
        CPPUNIT_ASSERT(op->entry(asd, 20));

        CPPUNIT_ASSERT(op->entry(qwe, 5));
        CPPUNIT_ASSERT(op->entry(qwe, 30));
        CPPUNIT_ASSERT(op->entry(qwe, 15));
        CPPUNIT_ASSERT(op->entry(qwe, 20));
        CPPUNIT_ASSERT(op->entry(qwe, 10));

        CPPUNIT_ASSERT(op->isOperand(asd));
        CPPUNIT_ASSERT(op->isOperand(qwe));

        op->sort();

        CPPUNIT_ASSERT_EQUAL(20, (int) op->getHigher(asd,10));
        CPPUNIT_ASSERT_EQUAL(30, (int) op->getHigher(asd,20));
        CPPUNIT_ASSERT_EQUAL(0, (int) op->getHigher(asd,30));

        CPPUNIT_ASSERT_EQUAL(10, (int) op->getHigher(qwe,5));
        CPPUNIT_ASSERT_EQUAL(15, (int) op->getHigher(qwe,10));
        CPPUNIT_ASSERT_EQUAL(20, (int) op->getHigher(qwe,15));
        CPPUNIT_ASSERT_EQUAL(30, (int) op->getHigher(qwe,20));
        CPPUNIT_ASSERT_EQUAL(0, (int) op->getHigher(qwe,30));

        CPPUNIT_ASSERT_EQUAL(10, (int) op->getPrecs(asd)->nth(0));
        CPPUNIT_ASSERT_EQUAL(5, (int) op->getPrecs(qwe)->nth(0));
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Operator_Test);
