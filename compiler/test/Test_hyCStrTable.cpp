/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCStrTable.h"
#include "hyMemPool.h"
#include "hyEndian.h"
#include "hpInputBuffer.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Compiler;
using namespace Hayat::Parser;

static const char* TEST_STRTABLE_FILENAME = "_test_strtable_file";

class Test_hyCStrTable : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyCStrTable);
    CPPUNIT_TEST(test_str);
    CPPUNIT_TEST(test_str_utf16);
    CPPUNIT_TEST(test_bytes);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(20480);
        MemPool::initGMemPool(hayatMemory, 20480);
    }

    void tearDown(void)
    {
        HMD_FREE(hayatMemory);
    }

    void test_str(void)
    {
        StrTable tbl;
        tbl.initialize(2, 10);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, tbl.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, tbl.addStr("foo"));
        CPPUNIT_ASSERT_EQUAL((hyu32)1, tbl.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu32)4, tbl.getOffs("hoge"));
        CPPUNIT_ASSERT_EQUAL((hyu32)2, tbl.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu32)9, tbl.addStr("asd qwe"));
        CPPUNIT_ASSERT_EQUAL((hyu32)3, tbl.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu32)17, tbl.m_strOffs[3]);
        CPPUNIT_ASSERT_EQUAL((hyu32)4, tbl.getOffs("hoge"));
        CPPUNIT_ASSERT_EQUAL(StrTable::NOT_FOUND, tbl.checkOffs("foo bar"));
        CPPUNIT_ASSERT_EQUAL((hyu32)9, tbl.getOffs("asd qwe"));

        hmd_rm_file(TEST_STRTABLE_FILENAME);
        FILE* fp = hmd_fopen(TEST_STRTABLE_FILENAME, "wb");
        CPPUNIT_ASSERT(fp != NULL);
        tbl.writeFile(fp);
        fclose(fp);

        fp = hmd_fopen(TEST_STRTABLE_FILENAME, "rb");
        CPPUNIT_ASSERT(fp != NULL);
        StrTable tbl2;
        tbl2.initialize();
        tbl2.readFile(fp);
        fclose(fp);

        CPPUNIT_ASSERT_EQUAL((hyu32)3, tbl2.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu32)17, tbl2.m_strOffs[3]);
        CPPUNIT_ASSERT_EQUAL((hyu32)4, tbl2.getOffs("hoge"));
        CPPUNIT_ASSERT_EQUAL(StrTable::NOT_FOUND, tbl2.checkOffs("foo bar"));
        CPPUNIT_ASSERT_EQUAL((hyu32)9, tbl2.checkOffs("asd qwe"));
        CPPUNIT_ASSERT_EQUAL((hyu32)0, tbl2.getOffs("foo"));
        CPPUNIT_ASSERT(! HMD_STRCMP("hoge", tbl2.getStr(4)));
        CPPUNIT_ASSERT(! HMD_STRCMP("asd qwe", tbl2.getStr(9)));

        hmd_rm_file(TEST_STRTABLE_FILENAME);
    }

    void test_str_utf16(void)
    {
        StrTable tbl;
        tbl.initialize(2, 10);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, tbl.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, tbl.addStr("a\0\0\0", 4));
        CPPUNIT_ASSERT_EQUAL((hyu32)1, tbl.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu32)4, tbl.getOffs("\0b\0\0", 4));
        CPPUNIT_ASSERT_EQUAL((hyu32)2, tbl.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu32)8, tbl.addStr("\x12\x34\x56\x78\0\0", 6));
        CPPUNIT_ASSERT_EQUAL((hyu32)3, tbl.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu32)14, tbl.m_strOffs[3]);
        CPPUNIT_ASSERT_EQUAL((hyu32)4, tbl.getOffs("\0b\0\0", 4));
        CPPUNIT_ASSERT_EQUAL(StrTable::NOT_FOUND, tbl.checkOffs("\x56\x78\0\0", 4));
        CPPUNIT_ASSERT_EQUAL((hyu32)8, tbl.getOffs("\x12\x34\x56\x78\0\0", 6));
    }


    void test_bytes(void)
    {
        StrTable tbl;
        tbl.initialize(2, 10);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, tbl.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu16)0, tbl.addBytes((const hyu8*)"\x11\xff\x00\x12", 4));
        CPPUNIT_ASSERT_EQUAL((hyu32)1, tbl.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu16)1, tbl.getIdBytes((const hyu8*)"\x00\x12\x34\x56", 4));
        CPPUNIT_ASSERT_EQUAL((hyu32)2, tbl.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu16)2, tbl.addBytes((const hyu8*)"\x11\xff", 2));
        CPPUNIT_ASSERT_EQUAL((hyu32)3, tbl.m_numStr);
        CPPUNIT_ASSERT_EQUAL((hyu32)10, tbl.m_strOffs[3]);
        CPPUNIT_ASSERT_EQUAL((hyu16)1, tbl.getIdBytes((const hyu8*)"\x00\x12\x34\x56", 4));
        CPPUNIT_ASSERT_EQUAL(StrTable::NOT_FOUND_ID, tbl.checkIdBytes((const hyu8*)"foo", 3));
        CPPUNIT_ASSERT_EQUAL((hyu16)0, tbl.getIdBytes((const hyu8*)"\x11\xff\x00\x12", 4));
        CPPUNIT_ASSERT_EQUAL((hyu16)2, tbl.getIdBytes((const hyu8*)"\x11\xff", 2));

        hyu32 len;
        const hyu8* p = tbl.getBytes(0, &len);
        CPPUNIT_ASSERT_EQUAL((hyu32)4, len);
        CPPUNIT_ASSERT(memcmp("\x11\xff\x00\x12", p, 4) == 0);
        p = tbl.getBytes(1, &len);
        CPPUNIT_ASSERT_EQUAL((hyu32)4, len);
        CPPUNIT_ASSERT(memcmp("\x00\x12\x34\x56", p, 4) == 0);
        p = tbl.getBytes(2, &len);
        CPPUNIT_ASSERT_EQUAL((hyu32)2, len);
        CPPUNIT_ASSERT(memcmp("\x11\xff", p, 2) == 0);


        TArray<hyu8> out(64);
        tbl.writeBytes(&out);
        CPPUNIT_ASSERT_EQUAL((hyu32)32, out.size());
        
        StrTable rtbl;
        rtbl.initialize(2, 10);
        const hyu8* inp = out.top();
        rtbl.readBytes(&inp);
        size_t nread = inp - out.top();
        CPPUNIT_ASSERT_EQUAL((size_t)32, nread);

        inp = out.top();
        CPPUNIT_ASSERT_EQUAL((hyu32)3, Endian::unpack<hyu32>(inp));
        CPPUNIT_ASSERT_EQUAL((hyu32)0, Endian::unpack<hyu32>(inp+4));
        CPPUNIT_ASSERT_EQUAL((hyu32)4, Endian::unpack<hyu32>(inp+8));
        CPPUNIT_ASSERT_EQUAL((hyu32)8, Endian::unpack<hyu32>(inp+12));
        CPPUNIT_ASSERT_EQUAL((hyu32)10, Endian::unpack<hyu32>(inp+16));
        CPPUNIT_ASSERT_EQUAL(0, memcmp("\x11\xff\x00\x12\x00\x12\x34\x56\x11\xff", inp+20, 10));
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyCStrTable);
