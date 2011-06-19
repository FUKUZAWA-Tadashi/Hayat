/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "machdep.h"
#include <cppunit/extensions/HelperMacros.h>


static const char* TEST_LOCK_FILE = "_test_lock_file";

class Test_filelock : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_filelock);
    CPPUNIT_TEST(test_lock);
    CPPUNIT_TEST_SUITE_END();
public:

    void setUp(void)
    {
    }

    void tearDown(void)
    {
        hmd_rm_file(TEST_LOCK_FILE);
    }

    void test_lock(void)
    {
        CPPUNIT_ASSERT_EQUAL(true, hmd_lockFile(TEST_LOCK_FILE));
        CPPUNIT_ASSERT_EQUAL(false, hmd_lockFile(TEST_LOCK_FILE));
        hmd_unlockFile();
        CPPUNIT_ASSERT_EQUAL(true, hmd_lockFile(TEST_LOCK_FILE));
        CPPUNIT_ASSERT_EQUAL(false, hmd_lockFile(TEST_LOCK_FILE));
        hmd_unlockFile();
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_filelock);
