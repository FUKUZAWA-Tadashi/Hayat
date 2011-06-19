/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "../stdlib/ffiout/HSca_Array.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Ffi::HScb_stdlib;

// ../stdlib/HSca_Array.cpp でも同じマクロを使用しているので注意
//{
#define DEFAULT_ARRAYSIZE 16
//}
#define IDXPOS(idx) (idx * 8 + 4)


class Test_HSca_Array : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_HSca_Array);
    CPPUNIT_TEST(test_HSca_Array);
    CPPUNIT_TEST(test_HSca_ArrayUtils);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;
    Context* context;
    
    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(100000);
        initMemory(hayatMemory, 100000);
        initStdlib();
        context = gCodeManager.createContext();
    }

    void tearDown(void)
    {
        finalizeAll();
        HMD_FREE(hayatMemory);
    }

    void test_HSca_Array(void)
    {
        //HClass HC_body_Array(HSym_Array);
        //Object* obj = Object::create(&HC_body_Array, sizeof(ValueArray) + sizeof(hyu32));

        Object* obj = Object::create(HC_Array, sizeof(ValueArray) + sizeof(hyu32));
        ValueArray* varr = obj->cppObj<ValueArray>();
        CPPUNIT_ASSERT_EQUAL(HSym_Array, obj->type()->getSymbol());
        context->pushInt(4);
        context->pushObj(obj);
        HSca_Array::m_HSfa_initialize_1(context, 1);

        HSca_Array::m_HSfa_size(context, 0);
        CPPUNIT_ASSERT_EQUAL(0, context->popInt());

        context->pushInt(10);
        context->pushInt(1);
        context->pushObj(obj);
        HSca_Array::m_HSfx_5b5d3d(context, 2); // []=
        CPPUNIT_ASSERT_EQUAL(10, context->popInt());
    
        context->pushObj(obj);
        HSca_Array::m_HSfa_size(context, 0);
        CPPUNIT_ASSERT_EQUAL(2, context->popInt());

        context->pushInt(0);
        context->pushObj(obj);
        HSca_Array::m_HSfx_5b5d(context, 1); // [0]
        Value v = context->pop();
        CPPUNIT_ASSERT_EQUAL(NIL_VALUE, v);

        context->pushInt(1);
        context->pushObj(obj);
        HSca_Array::m_HSfx_5b5d(context, 1); // [1]
        CPPUNIT_ASSERT_EQUAL(10, context->popInt());

        context->pushInt(20);
        context->pushInt(6);
        context->pushObj(obj);
        hyu32 prevCapa = varr->m_capacity;
        HSca_Array::m_HSfx_5b5d3d(context, 2); // []=
        CPPUNIT_ASSERT_EQUAL(20, context->popInt());
        // サイズ拡張された
        CPPUNIT_ASSERT(prevCapa != varr->m_capacity);

        context->pushObj(obj);
        HSca_Array::m_HSfa_size(context, 0);
        CPPUNIT_ASSERT_EQUAL(7, context->popInt());

        context->pushInt(6);
        context->pushObj(obj);
        HSca_Array::m_HSfx_5b5d(context, 1); // [6]
        CPPUNIT_ASSERT_EQUAL(20, context->popInt());

        context->pushInt(-6);
        context->pushObj(obj);
        HSca_Array::m_HSfx_5b5d(context, 1); // [7-6]
        CPPUNIT_ASSERT_EQUAL(10, context->popInt());

        /* frameの初期化をしないとthrowのテストはできない
        context->pushInt(30);
        context->pushInt(-10);
        context->pushObj(obj);
        HMD_ASSERT_HALT(HSca_Array::m_HSfx_5b5d3d(context, 2)); // [7-10]= -> error
        */

    }


    void test_HSca_ArrayUtils(void)
    {
        ValueArray* arr1 = new ValueArray(5);
        CPPUNIT_ASSERT_EQUAL((size_t)0, arr1->size());

        arr1->subst(2, TRUE_VALUE, NIL_VALUE);
        CPPUNIT_ASSERT_EQUAL((size_t)3, arr1->size());
        
        context->pushObj(arr1->getObj());
        ValueArray* p = context->popCppObj<ValueArray>(HSym_Array);
        CPPUNIT_ASSERT(p == arr1);

        Value* prevContents = arr1->m_contents;
        arr1->subst(100, FALSE_VALUE, NIL_VALUE);
        CPPUNIT_ASSERT(prevContents != arr1->m_contents);
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_HSca_Array);
