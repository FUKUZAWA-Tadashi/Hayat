/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCSymbolTable.h"
#include "hyMemPool.h"
#include "machdep.h"
#include "hyEndian.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Compiler;

static const char* TEST_SYM_FILENAME = "_test_sym_file";

class Test_hyCSymbolTable : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyCSymbolTable);
    CPPUNIT_TEST(test_symLabel);
    CPPUNIT_TEST(test_symbolID);
    CPPUNIT_TEST(test_file);
    CPPUNIT_TEST(test_merge);
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

    void test_symLabel(void)
    {
        char tmp[128];

        SymbolTable::m_symLabel(tmp, 10, "foobarasd");
        CPPUNIT_ASSERT_EQUAL('\0', tmp[0]);
        SymbolTable::m_symLabel(tmp, 10, "#####");
        CPPUNIT_ASSERT_EQUAL('\0', tmp[0]);
        SymbolTable::m_symLabel(tmp, 20, "#####");
        CPPUNIT_ASSERT(tmp[0] != '\0');

        SymbolTable::m_symLabel(tmp, 128, "asd");
        CPPUNIT_ASSERT(!HMD_STRCMP("HSym_asd", tmp));
        SymbolTable::m_symLabel(tmp, 128, "*asd");
        CPPUNIT_ASSERT(!HMD_STRCMP("HSymR_asd", tmp));
        SymbolTable::m_symLabel(tmp, 128, "asd*");
        CPPUNIT_ASSERT(!HMD_STRCMP("HSymx_6173642a", tmp));
        SymbolTable::m_symLabel(tmp, 128, "foo!");
        CPPUNIT_ASSERT(!HMD_STRCMP("HSymE_foo", tmp));
        SymbolTable::m_symLabel(tmp, 128, "bar?");
        CPPUNIT_ASSERT(!HMD_STRCMP("HSymQ_bar", tmp));
        SymbolTable::m_symLabel(tmp, 128, "*qwe!");
        CPPUNIT_ASSERT(!HMD_STRCMP("HSymRE_qwe", tmp));
        SymbolTable::m_symLabel(tmp, 128, "*qwe?");
        CPPUNIT_ASSERT(!HMD_STRCMP("HSymRQ_qwe", tmp));
        SymbolTable::m_symLabel(tmp, 128, "[]");
        CPPUNIT_ASSERT(!HMD_STRCMP("HSymx_5b5d", tmp));
        SymbolTable::m_symLabel(tmp, 128, "<=>");
        CPPUNIT_ASSERT(!HMD_STRCMP("HSymx_3c3d3e", tmp));
    }

    void test_symbolID(void)
    {
        SymbolTable symTbl;
        symTbl.initialize(NULL, 2, 10);
        // ó\ñÒÉVÉìÉ{ÉãÇìoò^Ç∑ÇÈÇÃÇ≈èâä˙ílÇÊÇËëÂÇ´Ç≠Ç»ÇÈ
        CPPUNIT_ASSERT_EQUAL((hyu32)0, symTbl.m_numSymbols);
        CPPUNIT_ASSERT_EQUAL((hyu32)2, symTbl.m_symOffsSize);
        CPPUNIT_ASSERT_EQUAL((hyu32)10, symTbl.m_symbolBufSize);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, (hyu32)symTbl.symbolID("new_sym0"));
        CPPUNIT_ASSERT_EQUAL((hyu32)0, (hyu32)symTbl.symbolID("new_sym0"));
        CPPUNIT_ASSERT_EQUAL((hyu32)1, (hyu32)symTbl.symbolID("new_sym1"));
        CPPUNIT_ASSERT_EQUAL((hyu32)2, (hyu32)symTbl.symbolID("new_sym2"));
        CPPUNIT_ASSERT_EQUAL((hyu32)1, (hyu32)symTbl.symbolID("new_sym1"));
        CPPUNIT_ASSERT_EQUAL((hyu32)0, (hyu32)symTbl.symbolID("new_sym0"));
        CPPUNIT_ASSERT_EQUAL((hyu32)2, (hyu32)symTbl.symbolID("new_sym2"));
    }

    void test_file(void)
    {
        SymbolTable symTbl;
        SymbolID_t ids[20];
        static const char* syms[] = {
            "123", "456", "789",
            "foo", "bar", "asd",
            "*bar", "*baz", "hoge_hoge", "fuga_fuga",
            "++", "--", "[]", "<--", "-->",
        };
        symTbl.initialize(NULL);
        for (hyu32 i = 0; i < sizeof(syms)/sizeof(syms[0]); i++) {
            ids[i] = symTbl.symbolID(syms[i]);
        }
        hmd_rm_file(TEST_SYM_FILENAME);
        symTbl.writeFile(TEST_SYM_FILENAME);

        SymbolTable symTbl2;
        symTbl2.readFile(TEST_SYM_FILENAME);
        for (hyu32 i = 0; i < sizeof(syms)/sizeof(syms[0]); i++) {
            CPPUNIT_ASSERT(!HMD_STRCMP(syms[i], symTbl2.id2str(ids[i])));
        }        

        hmd_rm_file(TEST_SYM_FILENAME);
    }

    

    void test_merge(void)
    {
        static const char* syms[] = {
            "123", "456", "789",
            "foo", "bar", "asd",
            "*bar", "*baz", "hoge_hoge",
            "fuga_fuga", "++", "--",
            "[]", "<--", "-->",
        };
        SymbolTable tbl1;
        for (int i = 0; i < 6; ++i) tbl1.symbolID(syms[i]);
        SymbolTable tbl2;
        for (int i = 0; i < 9; ++i) tbl2.symbolID(syms[i]);
        SymbolTable tbl3;
        for (int i = 0; i < 12; ++i) tbl3.symbolID(syms[i]);

        hyu8* buf = gMemPool->allocT<hyu8>(8 + tbl1.m_symOffs[tbl1.m_numSymbols]);
        Endian::pack<hyu32>(buf, (hyu32)tbl1.m_numSymbols);
        Endian::pack<hyu32>(buf + 4, (hyu32)(8 + tbl1.m_symOffs[tbl1.m_numSymbols]));
        memcpy(buf + 8, tbl1.m_symbolBuf, tbl1.m_symOffs[tbl1.m_numSymbols]);
        const hyu8* p = tbl2.mergeTable(buf, 8 + tbl1.m_symOffs[tbl1.m_numSymbols]);
        CPPUNIT_ASSERT_EQUAL(9, (int)tbl2.m_numSymbols);
        CPPUNIT_ASSERT_EQUAL(buf + 8 + tbl1.m_symOffs[tbl1.m_numSymbols], (hyu8*)p);

        buf = gMemPool->reallocT<hyu8>(buf, 8 + tbl3.m_symOffs[tbl3.m_numSymbols]);
        Endian::pack<hyu32>(buf, (hyu32)tbl3.m_numSymbols);
        Endian::pack<hyu32>(buf + 4, (hyu32)(8 + tbl3.m_symOffs[tbl3.m_numSymbols]));
        memcpy(buf + 8, tbl3.m_symbolBuf, tbl3.m_symOffs[tbl3.m_numSymbols]);
        p = tbl2.mergeTable(buf, 8 + tbl3.m_symOffs[tbl3.m_numSymbols]);
        CPPUNIT_ASSERT_EQUAL(12, (int)tbl2.m_numSymbols);
        CPPUNIT_ASSERT_EQUAL(buf + 8 + tbl3.m_symOffs[tbl3.m_numSymbols], (hyu8*)p);

        SymbolTable tbl4;
        for (int i = 1; i < 5; ++i) tbl4.symbolID(syms[i]);
        buf = gMemPool->reallocT<hyu8>(buf, 8 + tbl4.m_symOffs[tbl4.m_numSymbols]);
        Endian::pack<hyu32>(buf, (hyu32)tbl4.m_numSymbols);
        Endian::pack<hyu32>(buf + 4, (hyu32)(8 + tbl4.m_symOffs[tbl4.m_numSymbols]));
        memcpy(buf + 8, tbl4.m_symbolBuf, tbl4.m_symOffs[tbl4.m_numSymbols]);

        p = tbl3.mergeTable(buf, 8 + tbl4.m_symOffs[tbl4.m_numSymbols]);
        CPPUNIT_ASSERT_EQUAL((const hyu8*)NULL, p);

        gMemPool->free(buf);
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyCSymbolTable);
