/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCFileOut.h"
#include "machdep.h"
#include <cppunit/extensions/HelperMacros.h>
#include <errno.h>
#include <sys/stat.h>


using namespace Hayat::Compiler;

class Test_hyCFileOut : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyCFileOut);
    CPPUNIT_TEST(test_diffWrite);
    CPPUNIT_TEST_SUITE_END();
public:

    void test_diffWrite(void)
    {
        const char* FNAME = "./m_diff_write_test";
        hmd_rm_file(FNAME);

        struct stat stbuf;
        CPPUNIT_ASSERT(stat(FNAME, &stbuf) != 0);
        CPPUNIT_ASSERT_EQUAL(ENOENT, errno);

        DiffWrite dw(FNAME);
        FILE* fp = dw.fp();
        fprintf(fp, "Test_hyCFileOut\n");
        dw.close();
        CPPUNIT_ASSERT(stat(FNAME, &stbuf) == 0);
        time_t mtime = stbuf.st_mtime;

        hmd_sleep(2);
        DiffWrite odw;
        fp = odw.open(FNAME);
        CPPUNIT_ASSERT(stat(FNAME, &stbuf) == 0);
        CPPUNIT_ASSERT_EQUAL(mtime, stbuf.st_mtime);    // closeëO:èëÇ´ïœÇÌÇ¡ÇƒÇ¢Ç»Ç¢
        fprintf(fp, "Test_hyCFileOut\n");        // ì‡óeìØÇ∂
        odw.close();
        CPPUNIT_ASSERT(stat(FNAME, &stbuf) == 0);
        CPPUNIT_ASSERT_EQUAL(mtime, stbuf.st_mtime);    // èëÇ´ïœÇÌÇ¡ÇƒÇ¢Ç»Ç¢

        fp = odw.open(FNAME);
        fprintf(fp, "Test_hyCFileOut 2\n");      // ì‡óeà·Ç§
        odw.close();
        CPPUNIT_ASSERT(stat(FNAME, &stbuf) == 0);
        CPPUNIT_ASSERT(mtime != stbuf.st_mtime);    // èëÇ´ïœÇÌÇ¡ÇΩ
        
        hmd_rm_file(FNAME);
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyCFileOut);
