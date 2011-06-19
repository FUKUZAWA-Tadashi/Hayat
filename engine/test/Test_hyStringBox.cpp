/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hayat.h"
#include "hyStringBox.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;

class Test_StringBox : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_StringBox);
    CPPUNIT_TEST(test_stringBox);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(100000);
        initMemory(hayatMemory, 100000);
        initStdlib();
    }

    void tearDown(void)
    {
        finalizeAll();
        HMD_FREE(hayatMemory);
    }

    void test_stringBox(void)
    {
        StringBox strBox;

        CPPUNIT_ASSERT_EQUAL((hyu32)0, strBox.m_strStorage.size());
        CPPUNIT_ASSERT_EQUAL(0, strBox.m_markFlags.size());

        strBox.unmark();
        const char* p1 = strBox.store("foo");
        CPPUNIT_ASSERT(HMD_STRCMP(p1, "foo") == 0);
        const char* p2 = strBox.store("bar");
        CPPUNIT_ASSERT(HMD_STRCMP(p2, "bar") == 0);
        CPPUNIT_ASSERT_EQUAL((hyu32)2, strBox.m_strStorage.size());
        CPPUNIT_ASSERT_EQUAL(2, strBox.m_markFlags.size());
        strBox.sweep();
        CPPUNIT_ASSERT_EQUAL((hyu32)2, strBox.m_strStorage.size());
        CPPUNIT_ASSERT_EQUAL(0, strBox.m_markFlags.size());

        strBox.unmark();
        CPPUNIT_ASSERT_EQUAL(2, strBox.m_markFlags.size());
        CPPUNIT_ASSERT_EQUAL(false, strBox.m_markFlags.getAt(0));
        CPPUNIT_ASSERT_EQUAL(false, strBox.m_markFlags.getAt(1));
        strBox.mark(p2);
        CPPUNIT_ASSERT_EQUAL(true, strBox.m_markFlags.getAt(0));
        CPPUNIT_ASSERT_EQUAL(false, strBox.m_markFlags.getAt(1));

        const char* p3 = strBox.store("asd");
        CPPUNIT_ASSERT(HMD_STRCMP(p3, "asd") == 0);
        CPPUNIT_ASSERT_EQUAL((hyu32)3, strBox.m_strStorage.size());
        CPPUNIT_ASSERT_EQUAL(3, strBox.m_markFlags.size());
        strBox.sweep(); // p1 swept
        CPPUNIT_ASSERT_EQUAL((hyu32)2, strBox.m_strStorage.size());
        CPPUNIT_ASSERT_EQUAL(0, strBox.m_markFlags.size());
        CPPUNIT_ASSERT_EQUAL(p3, strBox.m_strStorage[0]);
        CPPUNIT_ASSERT_EQUAL(p2, strBox.m_strStorage[1]);

        const char* p4 = strBox.store("bar");
        CPPUNIT_ASSERT_EQUAL(p2, p4);   // same string same pointer
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_StringBox);
