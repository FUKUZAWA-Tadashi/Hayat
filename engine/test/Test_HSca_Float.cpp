/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "../stdlib/ffiout/HSca_Float.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>
#include <math.h>
#include "hyContext.h"

using namespace Hayat::Common;
using namespace Hayat::Ffi::HScb_stdlib;

class Test_HSca_Float : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_HSca_Float);
    CPPUNIT_TEST(test_HSca_Float_plus);
    CPPUNIT_TEST(test_HSca_Float_minus);
    CPPUNIT_TEST(test_HSca_Float_mul);
    CPPUNIT_TEST(test_HSca_Float_div);
    CPPUNIT_TEST(test_HSca_Float_pow);
    CPPUNIT_TEST(test_HSca_Float_eq);
    CPPUNIT_TEST(test_HSca_Float_comp);
    CPPUNIT_TEST(test_HSca_Float_lt);
    CPPUNIT_TEST(test_HSca_Float_le);
    CPPUNIT_TEST(test_HSca_Float_gt);
    CPPUNIT_TEST(test_HSca_Float_ge);
    CPPUNIT_TEST(test_HSca_Float_toInt);
    CPPUNIT_TEST(test_HSca_Float_floor);
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

    void test_HSca_Float_plus(void)
    {
        context->pushFloat(-10.125f);
        HSca_Float::m_HSfx_2b40(context, 0);        // 単項プラス
        CPPUNIT_ASSERT_EQUAL(-10.125f, context->popFloat());

        context->pushFloat(134.625f);
        context->pushFloat(23.25f);
        HSca_Float::m_HSfx_2b_1(context, 1);        // 2項プラス
        CPPUNIT_ASSERT_EQUAL(157.875f, context->popFloat());

        context->pushFloat(-300.5f);
        context->pushFloat(200.125f);
        HSca_Float::m_HSfx_2b_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(-100.375f, context->popFloat());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Float_minus(void)
    {
        context->pushFloat(-11.625f);
        HSca_Float::m_HSfx_2d40_0(context, 0);        // 単項マイナス
        CPPUNIT_ASSERT_EQUAL(11.625f, context->popFloat());

        context->pushFloat(50.375f);
        context->pushFloat(30.75f);
        HSca_Float::m_HSfx_2d_1(context, 1);        // 2項マイナス
        CPPUNIT_ASSERT_EQUAL(-19.625f, context->popFloat());

        context->pushFloat(-3000.0f);
        context->pushFloat(-1000.0f);
        HSca_Float::m_HSfx_2d_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(2000.0f, context->popFloat());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Float_mul(void)
    {
        context->pushFloat(5.5f);
        context->pushFloat(3.5f);
        HSca_Float::m_HSfx_2a_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(19.25f, context->popFloat());

        context->pushFloat(7.25f);
        context->pushFloat(-4.75f);
        HSca_Float::m_HSfx_2a_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(-34.4375f, context->popFloat());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Float_div(void)
    {
        context->pushFloat(40.0f);
        context->pushFloat(2000.0f);
        HSca_Float::m_HSfx_2f(context, 1);
        CPPUNIT_ASSERT_EQUAL(50.0f, context->popFloat());

        context->pushFloat(42.125f);
        context->pushFloat(57.921875f);
        HSca_Float::m_HSfx_2f(context, 1);
        CPPUNIT_ASSERT_EQUAL(1.375f, context->popFloat());

        context->pushFloat(0.0f);
        context->pushFloat(234.5f);
        HSca_Float::m_HSfx_2f(context, 1);
        CPPUNIT_ASSERT(0 != isinf(context->popFloat()));       // infinite

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Float_pow(void)
    {
        context->pushFloat(3.0f);
        context->pushFloat(4.0f);
        HSca_Float::m_HSfx_2a2a_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(64.0f, context->popFloat());

        context->pushFloat(0.25f);
        context->pushFloat(150.0625f);
        HSca_Float::m_HSfx_2a2a_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(3.5f, context->popFloat());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Float_eq(void)
    {
        context->pushFloat(4.0f);
        context->pushFloat(3.0f);
        HSca_Float::m_HSfx_3d3d_1(context, 1);
        CPPUNIT_ASSERT(!context->popBool());

        context->pushFloat(-12.0125f);
        context->pushFloat(-12.0125f);
        HSca_Float::m_HSfx_3d3d_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Float_comp(void)
    {
        context->pushFloat(3.0f);
        context->pushFloat(4.0f);
        HSca_Float::m_HSfx_3c3d3e_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(1, context->popInt());

        context->pushFloat(99.1234f);
        context->pushFloat(99.1234f);
        HSca_Float::m_HSfx_3c3d3e_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(0, context->popInt());

        context->pushFloat(-56.0f);
        context->pushFloat(65.0f);
        HSca_Float::m_HSfx_3c3d3e_1(context, 1);
        CPPUNIT_ASSERT_EQUAL(1, context->popInt());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Float_lt(void)
    {
        context->pushFloat(10.0001f);
        context->pushFloat(10.0f);
        HSca_Float::m_HSfx_3c_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        context->pushFloat(11.111f);
        context->pushFloat(11.111f);
        HSca_Float::m_HSfx_3c_1(context, 1);
        CPPUNIT_ASSERT(!context->popBool());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Float_le(void)
    {
        context->pushFloat(10.0001f);
        context->pushFloat(10.0f);
        HSca_Float::m_HSfx_3c3d_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        context->pushFloat(10.0001f);
        context->pushFloat(10.0001f);
        HSca_Float::m_HSfx_3c3d_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        context->pushFloat(10.0001f);
        context->pushFloat(10.0002f);
        HSca_Float::m_HSfx_3c3d_1(context, 1);
        CPPUNIT_ASSERT(!context->popBool());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Float_gt(void)
    {
        context->pushFloat(-24.0f);
        context->pushFloat(-23.0f);
        HSca_Float::m_HSfx_3e_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        context->pushFloat(-23.0f);
        context->pushFloat(-23.0f);
        HSca_Float::m_HSfx_3e_1(context, 1);
        CPPUNIT_ASSERT(!context->popBool());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Float_ge(void)
    {
        context->pushFloat(-23.0001f);
        context->pushFloat(-23.0f);
        HSca_Float::m_HSfx_3e3d_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        context->pushFloat(-23.0f);
        context->pushFloat(-23.0f);
        HSca_Float::m_HSfx_3e3d_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());

        context->pushFloat(-22.9999f);
        context->pushFloat(-23.0f);
        HSca_Float::m_HSfx_3e3d_1(context, 1);
        CPPUNIT_ASSERT(!context->popBool());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Float_toInt(void)
    {
        context->pushFloat(12345.67f);
        HSca_Float::m_HSfa_toInt_0(context, 0);
        CPPUNIT_ASSERT_EQUAL(12345, context->popInt());

        context->pushFloat(-123.5f);
        HSca_Float::m_HSfa_toInt_0(context, 0);
        CPPUNIT_ASSERT_EQUAL(-123, context->popInt());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

    void test_HSca_Float_floor(void)
    {
        context->pushFloat(12345.67f);
        HSca_Float::m_HSfa_floor_0(context, 0);
        CPPUNIT_ASSERT_EQUAL(12345, context->popInt());

        context->pushFloat(-123.5f);
        HSca_Float::m_HSfa_floor_0(context, 0);
        CPPUNIT_ASSERT_EQUAL(-124, context->popInt());

        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_HSca_Float);
