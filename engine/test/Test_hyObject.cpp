/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyObject.h"
#include "hyValue.h"
#include "hyMemPool.h"
#include "hyThreadManager.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;

class Test_hyObject : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyObject);
    CPPUNIT_TEST(test_Object);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(20480);
        MemPool::initGMemPool(hayatMemory, 20480);
        ThreadManager::firstOfAll();
        gThreadManager.initialize();
    }

    void tearDown(void)
    {
        gThreadManager.finalize();
        HMD_FREE(hayatMemory);
    }


    void test_Object(void)
    {
        Object* obj = Object::create((HClass*)100, 200);
        hyu32* p = (hyu32*) obj->field();
        *p = 0x3456789a;
        p[200/sizeof(hyu32)-1] = 0x87654321;
#ifdef m_MEMCELL_JUST_ARENA_SIZE_
        CPPUNIT_ASSERT((size_t)200 <= obj->size());
#else
        CPPUNIT_ASSERT_EQUAL((size_t)200, obj->size());
#endif
        CPPUNIT_ASSERT_EQUAL((const HClass*)100, obj->type());

        HMD_ASSERT_NO_HALT(obj->destroy());
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyObject);
