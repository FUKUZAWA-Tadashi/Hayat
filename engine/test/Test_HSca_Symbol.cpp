/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "../stdlib/ffiout/HSca_Symbol.h"
#include "hyValue.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>


using namespace Hayat::Common;
using namespace Hayat::Ffi::HScb_stdlib;

class Test_HSca_Symbol : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_HSca_Symbol);
    CPPUNIT_TEST(test_HSca_Symbol_eq);
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

    void test_HSca_Symbol_eq(void)
    {
        context->pushSymbol((SymbolID_t)1);
        context->pushSymbol((SymbolID_t)99);
        HSca_Symbol::m_HSfx_3d3d_1(context, 1);
        CPPUNIT_ASSERT(!context->popBool());
        
        context->pushSymbol((SymbolID_t)66);
        context->pushSymbol((SymbolID_t)66);
        HSca_Symbol::m_HSfx_3d3d_1(context, 1);
        CPPUNIT_ASSERT(context->popBool());
        
        CPPUNIT_ASSERT_EQUAL(0, (int)context->stack.size());
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_HSca_Symbol);
