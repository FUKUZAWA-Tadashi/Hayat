/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "../stdlib/ffiout/HSca_Math.h"
#include "../stdlib/ffiout/HSca_Float.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>
#include <math.h>
#include "hyContext.h"

using namespace Hayat::Common;
using namespace Hayat::Ffi::HScb_stdlib;

class Test_HSca_Math : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_HSca_Math);
    CPPUNIT_TEST(test_HSca_Math_sincos);
    CPPUNIT_TEST(test_HSca_Math_sqrt);
    CPPUNIT_TEST(test_HSca_Math_log);
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

    void sin2cos2(hyf32 x)
    {
        context->pushFloat(2.0f);
        context->pushFloat(x);
        context->pushClass(HC_Float);
        HSca_Math::m_HSfa_sin_1(context, 1);
        HSca_Float::m_HSfx_2a2a_1(context, 1);
        // sin(x)^2

        context->pushFloat(x);
        context->pushClass(HC_Float);
        HSca_Math::m_HSfa_cos_1(context, 1);
        hyf32 o = context->popFloat();
        context->pushFloat(o);
        context->pushFloat(o);
        HSca_Float::m_HSfx_2a_1(context, 1);
        // cos(x) * cos(x)

        HSca_Float::m_HSfx_2b_1(context, 1);
        // sin^2 + cos^2
    }

    void test_HSca_Math_sincos(void)
    {
        sin2cos2(0.0f);
        hyf32 o = context->popFloat();
        CPPUNIT_ASSERT(o > 0.999999f); CPPUNIT_ASSERT(o < 1.000001f);

        sin2cos2(2.0f);
        o = context->popFloat();
        CPPUNIT_ASSERT(o > 0.999999f); CPPUNIT_ASSERT(o < 1.000001f);

        sin2cos2(-456.789f);
        o = context->popFloat();
        CPPUNIT_ASSERT(o > 0.999999f); CPPUNIT_ASSERT(o < 1.000001f);
    }
    

    void test_HSca_Math_sqrt(void)
    {
        context->pushFloat(1.0f);
        context->pushClass(HC_Math);
        HSca_Math::m_HSfa_sqrt_1(context, 1);
        hyf32 y = context->popFloat();
        CPPUNIT_ASSERT(y > 0.9999f); CPPUNIT_ASSERT(y < 1.0001f);

        context->pushFloat(9.0f);
        context->pushClass(HC_Math);
        HSca_Math::m_HSfa_sqrt_1(context, 1);
        y = context->popFloat();
        CPPUNIT_ASSERT(y > 2.9999f); CPPUNIT_ASSERT(y < 3.0001f);

        context->pushFloat(123.4f);
        context->pushClass(HC_Math);
        HSca_Math::m_HSfa_sqrt_1(context, 1);
        y = context->popFloat();
        CPPUNIT_ASSERT(y > 11.1085f); CPPUNIT_ASSERT(y < 11.1086f);
    }

    void test_HSca_Math_log(void)
    {
        context->pushFloat(2.7182818284590452354f);
        context->pushClass(HC_Math);
        HSca_Math::m_HSfa_log_1(context, 1);
        hyf32 y = context->popFloat();
        CPPUNIT_ASSERT(y > 0.9999f); CPPUNIT_ASSERT(y < 1.0001f);

        context->pushFloat(10.0f);
        context->pushClass(HC_Math);
        HSca_Math::m_HSfa_log_1(context, 1);
        y = context->popFloat();
        CPPUNIT_ASSERT(y > 2.3025f); CPPUNIT_ASSERT(y < 2.3026f);

        context->pushFloat(123.4f);
        context->pushClass(HC_Math);
        HSca_Math::m_HSfa_log_1(context, 1);
        y = context->popFloat();
        CPPUNIT_ASSERT(y > 4.8154f); CPPUNIT_ASSERT(y < 4.8155f);
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_HSca_Math);
