/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "../stdlib/ffiout/HSca_StringBuffer.h"
#include "hyValue.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;
using namespace Hayat::Ffi::HScb_stdlib;


class Test_HSca_StringBuffer : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_HSca_StringBuffer);
    CPPUNIT_TEST(test_HSca_StringBuffer);
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

    void test_HSca_StringBuffer(void)
    {
        HClass HC_body_StringBuffer(HSym_StringBuffer);
        HC_StringBuffer = &HC_body_StringBuffer;

        context->pushClass(HC_StringBuffer);
        HSca_StringBuffer::m_HSfx_2a63707053697a65(context, 0);
        hys32 i = context->popInt();
        CPPUNIT_ASSERT_EQUAL((hys32)sizeof(StringBuffer), i);
        Object* obj = Object::create(HC_StringBuffer, i + sizeof(hyu32));
        CPPUNIT_ASSERT_EQUAL(HSym_StringBuffer, obj->type()->getSymbol());
        context->pushInt(StringBuffer::DEFAULT_BUFSIZE);
        context->pushObj(obj);
        HSca_StringBuffer::m_HSfa_initialize_1(context, 1);
        context->pop();

        context->pushObj(obj);
        HSca_StringBuffer::m_HSfa_bufSize_0(context, 0);
        i = context->popInt();
        CPPUNIT_ASSERT(i >= StringBuffer::DEFAULT_BUFSIZE);
#ifndef m_MEMCELL_JUST_ARENA_SIZE_
        CPPUNIT_ASSERT(i < StringBuffer::DEFAULT_BUFSIZE + 4);
#endif

        context->pushObj(obj);
        HSca_StringBuffer::m_HSfa_length_0(context, 0);
        CPPUNIT_ASSERT_EQUAL(0, context->popInt());
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_HSca_StringBuffer);

