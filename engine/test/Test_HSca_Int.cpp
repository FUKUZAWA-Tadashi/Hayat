/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "../stdlib/ffiout/HSca_Int.h"
#include "hyValue.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

using namespace Hayat::Common;
using namespace Hayat::Ffi::HScb_stdlib;

class Test_HSca_Int : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_HSca_Int);
    CPPUNIT_TEST(test_HSca_Int_plus);
    CPPUNIT_TEST(test_HSca_Int_minus);
    CPPUNIT_TEST(test_HSca_Int_mul);
    CPPUNIT_TEST(test_HSca_Int_div);
    CPPUNIT_TEST(test_HSca_Int_mod);
    CPPUNIT_TEST(test_HSca_Int_pow);
    CPPUNIT_TEST(test_HSca_Int_eq);
    CPPUNIT_TEST(test_HSca_Int_comp);
    CPPUNIT_TEST(test_HSca_Int_lt);
    CPPUNIT_TEST(test_HSca_Int_le);
    CPPUNIT_TEST(test_HSca_Int_gt);
    CPPUNIT_TEST(test_HSca_Int_ge);
    CPPUNIT_TEST(test_HSca_Int_and);
    CPPUNIT_TEST(test_HSca_Int_or);
    CPPUNIT_TEST(test_HSca_Int_xor);
    CPPUNIT_TEST(test_HSca_Int_not);
    CPPUNIT_TEST(test_HSca_Int_toFloat);
    CPPUNIT_TEST(test_HSca_Int_abs);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;
    Context* context;


    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(20480);
        initMemory(hayatMemory, 20480);
        initStdlib();
        context = gCodeManager.createContext();
    }

    void tearDown(void)
    {
        finalizeAll();
        HMD_FREE(hayatMemory);
    }

    void test_HSca_Int_plus(void)
    {
        context->pushInt(-10);
        HSca_Int::m_HSfx_2b40(context, 0);        // 単項プラス
        CPPUNIT_ASSERT_EQUAL(-10, context->popInt());

        context->pushInt(3);
        context->pushInt(5);
        HSca_Int::m_HSfx_2b_1(context, 1);        // 2項プラス
        CPPUNIT_ASSERT_EQUAL(8, context->popInt());

        context->pushInt(-20000);
        context->pushInt(30000);
        HSca_Int::m_HSfx_2b_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(10000, context->popInt());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_minus(void)
    {
        context->pushInt(-11);
        HSca_Int::m_HSfx_2d40_0(context, 0);        // 単項マイナス
        CPPUNIT_ASSERT_EQUAL(11, context->popInt());

        context->pushInt(50);
        context->pushInt(30);
        HSca_Int::m_HSfx_2d_1(context, 1);        // 2項マイナス
        CPPUNIT_ASSERT_EQUAL(-20, context->popInt());

        context->pushInt(-3000);
        context->pushInt(-1000);
        HSca_Int::m_HSfx_2d_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(2000, context->popInt());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_mul(void)
    {
        context->pushInt(5);
        context->pushInt(3);
        HSca_Int::m_HSfx_2a_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(15, context->popInt());

        context->pushInt(7);
        context->pushInt(-4);
        HSca_Int::m_HSfx_2a_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(-28, context->popInt());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_div(void)
    {
        context->pushInt(40);
        context->pushInt(2000);
        HSca_Int::m_HSfx_2f(context, 1);
        CPPUNIT_ASSERT_EQUAL(50, context->popInt());

        context->pushInt(234);
        context->pushInt(34567);
        HSca_Int::m_HSfx_2f(context, 1);
        CPPUNIT_ASSERT_EQUAL(147, context->popInt());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());

        /*
          CPPUNIT で Hayat の Exception を捕獲できないのでコメントアウト
        context->pushInt(0);
        context->pushInt(123);
        HMD_ASSERT_HALT(HSca_Int::m_HSfx_2f(context, 1));

        CPPUNIT_ASSERT_EQUAL(1, (int)context->stack.size());
        */
    }

    void test_HSca_Int_mod(void)
    {
        context->pushInt(234);
        context->pushInt(34567);
        HSca_Int::m_HSfx_25(context, 1);
        CPPUNIT_ASSERT_EQUAL(34567 - 234 * 147, context->popInt());

        context->pushInt(45);
        context->pushInt(123);
        HSca_Int::m_HSfx_25(context, 1);
        CPPUNIT_ASSERT_EQUAL(33, context->popInt());

        context->pushInt(45);
        context->pushInt(-123);
        HSca_Int::m_HSfx_25(context, 1);
        CPPUNIT_ASSERT_EQUAL(-123-(-123/45)*45, context->popInt());

        context->pushInt(-45);
        context->pushInt(123);
        HSca_Int::m_HSfx_25(context, 1);
        CPPUNIT_ASSERT_EQUAL(123-(123/-45)*-45, context->popInt());

        context->pushInt(-45);
        context->pushInt(-123);
        HSca_Int::m_HSfx_25(context, 1);
        CPPUNIT_ASSERT_EQUAL(-123-(-123/-45)*-45, context->popInt());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }


    void test_HSca_Int_pow(void)
    {
        context->pushInt(3);
        context->pushInt(4);
        HSca_Int::m_HSfx_2a2a_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(64, context->popInt());

        context->pushInt(10);
        context->pushInt(2);
        HSca_Int::m_HSfx_2a2a_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(1024, context->popInt());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_eq(void)
    {
        context->pushInt(4);
        context->pushInt(3);
        HSca_Int::m_HSfx_3d3d_1(context, 1);
        CPPUNIT_ASSERT(!context->popBool());

        context->pushInt(-12);
        context->pushInt(-12);
        HSca_Int::m_HSfx_3d3d_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_comp(void)
    {
        context->pushInt(3);
        context->pushInt(4);
        HSca_Int::m_HSfx_3c3d3e_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(1, context->popInt());

        context->pushInt(99);
        context->pushInt(99);
        HSca_Int::m_HSfx_3c3d3e_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(0, context->popInt());

        context->pushInt(-56);
        context->pushInt(65);
        HSca_Int::m_HSfx_3c3d3e_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(1, context->popInt());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_lt(void)
    {
        context->pushInt(11);
        context->pushInt(10);
        HSca_Int::m_HSfx_3c_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        context->pushInt(11);
        context->pushInt(11);
        HSca_Int::m_HSfx_3c_1(context, 1);
        CPPUNIT_ASSERT(!context->popBool());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_le(void)
    {
        context->pushInt(11);
        context->pushInt(10);
        HSca_Int::m_HSfx_3c3d_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        context->pushInt(11);
        context->pushInt(11);
        HSca_Int::m_HSfx_3c3d_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        context->pushInt(11);
        context->pushInt(12);
        HSca_Int::m_HSfx_3c3d_1(context, 1);
        CPPUNIT_ASSERT(!context->popBool());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_gt(void)
    {
        context->pushInt(-24);
        context->pushInt(-23);
        HSca_Int::m_HSfx_3e_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        context->pushInt(-23);
        context->pushInt(-23);
        HSca_Int::m_HSfx_3e_1(context, 1);
        CPPUNIT_ASSERT(!context->popBool());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_ge(void)
    {
        context->pushInt(-24);
        context->pushInt(-23);
        HSca_Int::m_HSfx_3e3d_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        context->pushInt(-23);
        context->pushInt(-23);
        HSca_Int::m_HSfx_3e3d_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        context->pushInt(-22);
        context->pushInt(-23);
        HSca_Int::m_HSfx_3e3d_1(context, 1);
        CPPUNIT_ASSERT(!context->popBool());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_and(void)
    {
        context->pushInt(0x3f9d);
        context->pushInt(0x5dc9);
        HSca_Int::m_HSfx_26_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(0x3f9d & 0x5dc9, context->popInt());

        context->pushInt(-123456789);        // f8a432eb
        context->pushInt(1234567890);        // 499602d2
        HSca_Int::m_HSfx_26_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(1216611010, context->popInt());    // 488402c2

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_or(void)
    {
        context->pushInt(24);
        context->pushInt(33);
        HSca_Int::m_HSfx_7c_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(24 | 33, context->popInt());

        context->pushInt(1234567890);        // 499602d2
        context->pushInt(-234567890);        // f204c72e
        HSca_Int::m_HSfx_7c_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(-74004482, context->popInt()); // fb96c7fe

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_xor(void)
    {
        context->pushInt(0x1234);
        context->pushInt(0x6789);
        HSca_Int::m_HSfx_5e_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(0x1234 ^ 0x6789, context->popInt());

        context->pushInt(987654321);         // 3ade68b1
        context->pushInt(-876543210);        // cbc10316
        HSca_Int::m_HSfx_5e_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(-249599065, context->popInt()); // f11f6ba7

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_not(void)
    {
        context->pushInt(0x12345678);
        HSca_Int::m_HSfx_7e_0(context, 0);
        CPPUNIT_ASSERT_EQUAL(~0x12345678, context->popInt());

        context->pushInt(0x12345678);
        HSca_Int::m_HSfx_7e_0(context, 0);
        CPPUNIT_ASSERT_EQUAL((hys32)0xedcba987, context->popInt());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_toFloat(void)
    {
        context->pushInt(12345);
        HSca_Int::m_HSfa_toFloat_0(context, 0);
        CPPUNIT_ASSERT_EQUAL(12345.0f, context->popFloat());

        context->pushInt(-123);
        HSca_Int::m_HSfa_toFloat_0(context, 0);
        CPPUNIT_ASSERT_EQUAL(-123.0f, context->popFloat());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Int_abs(void)
    {
        context->pushInt(12345);
        HSca_Int::m_HSfa_abs_0(context, 0);
        CPPUNIT_ASSERT_EQUAL(12345, context->popInt());

        context->pushInt(-123);
        HSca_Int::m_HSfa_abs_0(context, 0);
        CPPUNIT_ASSERT_EQUAL(123, context->popInt());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_HSca_Int);

