/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hySymbolTable.h"
#include "hyException.h"
#include "hyEndian.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;

class Test_hySymbol : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hySymbol);
    CPPUNIT_TEST(test_hySymbol);
    CPPUNIT_TEST_SUITE_END();
public:
    void test_hySymbol(void)
    {
        MemPool* orgMemPool = gMemPool;
        void* tmp = HMD_ALLOC(2048);
        MemPool::initGMemPool(tmp, 2048);

        hyu8 a[4+4+37+10] = {
            9,0,0,0,                // 9ŒÂ
            37+10,0,0,0,               // 37+10ƒoƒCƒg
            'a','b','c','d','\0',
            'f','o','o','\0',
            'b','a','r','\0',
            'a','s','d','\0',
            'q','w','e','\0',
            'h','o','g','e','\0',
            'f','u','g','a','\0',
            '*','*','\0',
            '|','|','\0',
            '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0',
        };
        if (Endian::MY_ENDIAN == Endian::BIG) {
            hyu8 b[8] = { 0,0,0,9, 0,0,0,37+10 };
            memcpy(a, b, 8);
        }
        SymbolTable tbl;
        CPPUNIT_ASSERT_EQUAL(0, tbl.numSymbols());

        HMD_ASSERT_NO_HALT(tbl.readTable(a));
        CPPUNIT_ASSERT_EQUAL(9, tbl.numSymbols());
        CPPUNIT_ASSERT(memcmp("qwe", tbl.id2str(4), 4) == 0);
        CPPUNIT_ASSERT(memcmp("||", tbl.id2str(8), 3) == 0);
        CPPUNIT_ASSERT(memcmp("{9}", tbl.id2str(9), 4) == 0);

        CPPUNIT_ASSERT_EQUAL((SymbolID_t)9, tbl.addSymbol((hyu8*)"hy"));
        CPPUNIT_ASSERT(memcmp("hy", tbl.id2str(9), 3) == 0);
        CPPUNIT_ASSERT_EQUAL(SYMBOL_ID_ERROR, tbl.addSymbol((hyu8*)"too_long_symbol_name_to_add"));
        CPPUNIT_ASSERT_EQUAL(10, tbl.numSymbols());

        gMemPool = orgMemPool;
        HMD_FREE(tmp);
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hySymbol);
