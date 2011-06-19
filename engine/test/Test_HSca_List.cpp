/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "../stdlib/ffiout/HSca_List.h"
#include "hyValue.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>


using namespace Hayat::Common;
using namespace Hayat::Ffi::HScb_stdlib;

class Test_HSca_List : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_HSca_List);
    CPPUNIT_TEST(test_HSca_List);
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


    static bool isEqual(Context* context, ValueList* a, ValueList* b)
    {
        context->pushList(a);
        context->pushList(b);
        HSca_List::m_HSfx_3d3d(context, 1);
        return context->popBool();
    }

    void test_HSca_List(void)
    {
        context->pushList(NULL);
        HSca_List::m_HSfq_empty_0(context, 0);
        CPPUNIT_ASSERT_EQUAL(true, context->popBool());

        context->pushInt(1001);
        context->pushList(NULL);
        context->pushClass(HC_List);
        HSca_List::m_HSfa_cons(context, 2);
        ValueList* x = context->popList();
        CPPUNIT_ASSERT_EQUAL(1001, x->head().toInt());
        
        context->pushFloat(2002.25f);
        context->pushList(x);
        context->pushClass(HC_List);
        HSca_List::m_HSfa_cons(context, 2);
        x = context->popList();
        context->pushList(x);
        HSca_List::m_HSfa_head(context, 0);
        CPPUNIT_ASSERT_EQUAL(2002.25f, context->popFloat());

        context->pushList(x);
        HSca_List::m_HSfa_tail(context, 0);
        ValueList* y = context->popList();
        CPPUNIT_ASSERT_EQUAL(1001, y->head().toInt());
        
        // now x == '(2002.25, 1001)

        context->pushInt(567);
        context->pushList(NULL);
        context->pushClass(HC_List);
        HSca_List::m_HSfa_cons(context, 2);
        y = context->popList();
        context->pushInt(34);
        context->pushList(y);
        context->pushClass(HC_List);
        HSca_List::m_HSfa_cons(context, 2);
        y = context->popList();

        // now y == '(34, 567)

        context->pushList(x);
        context->pushList(y);
        HSca_List::m_HSfa_append_1(context, 1);
        ValueList* z = context->popList();
        CPPUNIT_ASSERT(z != x);
        CPPUNIT_ASSERT(z != y);
        CPPUNIT_ASSERT(!isEqual(context,z,x));
        CPPUNIT_ASSERT(!isEqual(context,z,y));

        // now z == '(34, 567, 2002.25, 1001)

        context->pushList(z);
        HSca_List::m_HSfa_clone_0(context, 0);
        y = context->popList();

        Value t[4];
        t[0] = Value::fromInt(34);
        t[1] = Value::fromInt(567);
        t[2] = Value::fromFloat(2002.25);
        t[3] = Value::fromInt(1001);
        int i = 0;
        while (z != NULL) {
            CPPUNIT_ASSERT(i < 4);
            CPPUNIT_ASSERT(y != NULL);
            CPPUNIT_ASSERT(y != z);
            CPPUNIT_ASSERT(isEqual(context,z,y));
            context->pushList(z);
            HSca_List::m_HSfa_head(context, 0);
            Value v = context->pop();
            CPPUNIT_ASSERT(v.type == t[i].type);
            CPPUNIT_ASSERT(v.data == t[i].data);
            context->pushList(z);
            HSca_List::m_HSfa_tail(context, 0);
            z = context->popList();
            context->pushList(y);
            HSca_List::m_HSfa_tail(context, 0);
            y = context->popList();
            ++i;
        }            
        CPPUNIT_ASSERT(y == NULL);

    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_HSca_List);
