/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "mbcsconv.h"
#include <cppunit/extensions/HelperMacros.h>


using namespace Hayat::Parser;


class MBCS_Test : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(MBCS_Test);
    CPPUNIT_TEST(test_unescape);
    CPPUNIT_TEST(test_conv);
    CPPUNIT_TEST_SUITE_END();
public:


    void test_unescape(void)
    {
        char buf[64];
        MBCSConv::initialize("C");

        hys32 x = MBCSConv::unescape("123", 3, buf, 3, false);
        CPPUNIT_ASSERT_EQUAL(-2, x);    // buffer over
        x = MBCSConv::unescape("123", 3, buf, 4, false);
        CPPUNIT_ASSERT_EQUAL(3, x);
        x = MBCSConv::unescape("123", 3, buf, 4, true);
        CPPUNIT_ASSERT_EQUAL(-2, x);    // buffer over
        x = MBCSConv::unescape("123", 3, buf, 5, true);
        CPPUNIT_ASSERT_EQUAL(4, x);

#define UESC(str,e) MBCSConv::unescape(str, HMD_STRLEN(str), buf, 64, e)

        x = UESC("\\", false);
        CPPUNIT_ASSERT_EQUAL(1, x);
        CPPUNIT_ASSERT(! HMD_STRCMP("\\", buf));

        x = UESC("\\x", true);
        CPPUNIT_ASSERT_EQUAL(3, x);
        CPPUNIT_ASSERT(! HMD_STRCMP("\\x\n", buf));

        x = UESC("\\x6", false);
        CPPUNIT_ASSERT_EQUAL(3, x);
        CPPUNIT_ASSERT(! HMD_STRCMP("\\x6", buf));

        x = UESC("\\x61", true);
        CPPUNIT_ASSERT_EQUAL(2, x);
        CPPUNIT_ASSERT(! HMD_STRCMP("a\n", buf));

        x = UESC("\\\n\\\r\\xz\\x0z", true);
        CPPUNIT_ASSERT_EQUAL(10, x);
        CPPUNIT_ASSERT(! HMD_STRCMP("\n\r\\xz\\x0z\n", buf));

        x = UESC("foo\\nbar\\rasd\\x61qwe\\x65", false);
        CPPUNIT_ASSERT_EQUAL(16, x);
        CPPUNIT_ASSERT(! HMD_STRCMP("foo\nbar\rasdaqwee", buf));

#undef UESC

        MBCSConv::finalize();
    }

    void test_conv(void)
    {
        // Š¿Žš‚ a‚ñz //
        const char s_utf8[] = {(char)0xe6, (char)0xbc, (char)0xa2, (char)0xe5,
                               (char)0xad, (char)0x97, (char)0xe3, (char)0x81,
                               (char)0x82, (char)0x61, (char)0xe3, (char)0x82,
                               (char)0x93, (char)0x7a, 0};
        const hyu32 len_utf8 = 15;
        const char s_sjis[] = {(char)0x8a, (char)0xbf, (char)0x8e, (char)0x9a,
                               (char)0x82, (char)0xa0, (char)0x61, (char)0x82,
                               (char)0xf1, (char)0x7a, 0};
        const hyu32 len_sjis = 11;
        const char s_euc[] = {(char)0xb4, (char)0xc1, (char)0xbb, (char)0xfa,
                              (char)0xa4, (char)0xa2, (char)0x61, (char)0xa4,
                              (char)0xf3, (char)0x7a, 0};
        const hyu32 len_euc = 11;
        const char s_utf16be[] = {(char)0x6f,(char)0x22, (char)0x5b,(char)0x57,
                                  (char)0x30,(char)0x42, (char)0x00,(char)0x61,
                                  (char)0x30,(char)0x93, (char)0x00,(char)0x7a,
                                  0, 0};
        const hyu32 len_utf16be = 14;
        const char s_utf16le[] = {(char)0x22,(char)0x6f, (char)0x57,(char)0x5b,
                                  (char)0x42,(char)0x30, (char)0x61,(char)0x00,
                                  (char)0x93,(char)0x30, (char)0x7a,(char)0x00,
                                  0, 0};
        const hyu32 len_utf16le = 14;
            
        const char* cn_sjis = "SJIS";
        const char* cn_euc = "EUCJP";
        const char* cn_utf8 = "UTF-8";
        const char* cn_utf16be = "UTF-16BE";
        const char* cn_utf16le = "UTF-16LE";
        // on Visual C++, these locales are converted to CodePage
        // in machdep/vc/mbcsconv.cpp .

        char buf[64];
        hyu32 x;


#define CONV_TEST(FROM,TO) \
        CPPUNIT_ASSERT(MBCSConv::initialize(cn_##FROM, cn_##TO));   \
        x = MBCSConv::conv(s_##FROM, len_##FROM, buf, 64);          \
        CPPUNIT_ASSERT_EQUAL(len_##TO, x);                          \
        CPPUNIT_ASSERT(HMD_MEMCMP(s_##TO, buf, x) == 0);            \
        MBCSConv::finalize()


        CONV_TEST(euc,euc);
        CONV_TEST(euc,sjis);
        CONV_TEST(euc,utf8);
        CONV_TEST(euc,utf16be);
        CONV_TEST(euc,utf16le);

        CONV_TEST(sjis,euc);
        CONV_TEST(sjis,sjis);
        CONV_TEST(sjis,utf8);
        CONV_TEST(sjis,utf16be);
        CONV_TEST(sjis,utf16le);

        CONV_TEST(utf8,euc);
        CONV_TEST(utf8,sjis);
        CONV_TEST(utf8,utf8);
        CONV_TEST(utf8,utf16be);
        CONV_TEST(utf8,utf16le);

#undef CONV_TEST
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(MBCS_Test);
