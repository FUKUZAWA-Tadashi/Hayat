/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "../stdlib/ffiout/HSca_Bool.h"
#include "hyValue.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

using namespace Hayat::Common;
using namespace Hayat::Ffi::HScb_stdlib;

class Test_HSca_Bool : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_HSca_Bool);
    CPPUNIT_TEST(test_HSca_Bool_not);
    CPPUNIT_TEST(test_HSca_Bool_eq);
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

    void test_HSca_Bool_not(void)
    {
        Value d;

        context->pushBool(true);
        HSca_Bool::m_HSfx_21_0(context, 0);
        d = context->pop();
        CPPUNIT_ASSERT_EQUAL(FALSE_VALUE, d);

        context->pushBool(false);
        HSca_Bool::m_HSfx_21_0(context, 0);
        d = context->pop();
        CPPUNIT_ASSERT_EQUAL(TRUE_VALUE, d);
    }

    void test_HSca_Bool_eq(void)
    {
        Value d;

        context->pushBool(true);
        context->pushBool(true);
        HSca_Bool::m_HSfx_3d3d_1(context, 1);
        d = context->pop();
        CPPUNIT_ASSERT_EQUAL(TRUE_VALUE, d);

        context->pushBool(true);
        context->pushBool(false);
        HSca_Bool::m_HSfx_3d3d_1(context, 1);
        d = context->pop();
        CPPUNIT_ASSERT_EQUAL(FALSE_VALUE, d);

        context->pushBool(false);
        context->pushBool(true);
        HSca_Bool::m_HSfx_3d3d_1(context, 1);
        d = context->pop();
        CPPUNIT_ASSERT_EQUAL(FALSE_VALUE, d);

        context->pushBool(false);
        context->pushBool(false);
        HSca_Bool::m_HSfx_3d3d_1(context, 1);
        d = context->pop();
        CPPUNIT_ASSERT_EQUAL(TRUE_VALUE, d);
        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_HSca_Bool);

