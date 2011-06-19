/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCBytecode.h"
#include "hyMemPool.h"
#include "hyEndian.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Compiler;

class Test_hyCBytecode : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyCBytecode);
    CPPUNIT_TEST(test_accessor);
    CPPUNIT_TEST(test_codes);
    CPPUNIT_TEST(test_addCode);
    CPPUNIT_TEST(test_removeLastCode);
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

    void test_accessor(void)
    {
        Bytecode b;
        b.setNumLocalAlloc(1);
        CPPUNIT_ASSERT_EQUAL((hyu8)1, b.m_numLocalAlloc);
        b.setNumLocalAlloc(10);
        CPPUNIT_ASSERT_EQUAL((hyu8)10, b.m_numLocalAlloc);
        b.setOffset(10);
        CPPUNIT_ASSERT_EQUAL((hyu32)10, b.getOffset());
        b.setOffset(10000);
        CPPUNIT_ASSERT_EQUAL((hyu32)10000, b.getOffset());
    }

    void test_codes(void)
    {
        Bytecode b;
        hyu8 buf[20] = { 10,11,12,13,14,15,16,17,18,19,
                       120,121,122,123,124,125,126,127,128,129 };
        CPPUNIT_ASSERT_EQUAL((hyu32)0, b.getSize());
        b.addCodes(buf, 10);
        CPPUNIT_ASSERT_EQUAL((hyu32)10, b.getSize());
        CPPUNIT_ASSERT_EQUAL((hyu8)15, *(b.getCodes(5)));
        b.replaceCodes(3, &buf[10], 5);
        CPPUNIT_ASSERT_EQUAL((hyu8)10, *(b.getCodes(0)));
        CPPUNIT_ASSERT_EQUAL((hyu8)12, *(b.getCodes(2)));
        CPPUNIT_ASSERT_EQUAL((hyu8)120, *(b.getCodes(3)));
        CPPUNIT_ASSERT_EQUAL((hyu8)124, *(b.getCodes(7)));
        CPPUNIT_ASSERT_EQUAL((hyu8)18, *(b.getCodes(8)));
        CPPUNIT_ASSERT_EQUAL((hyu8)19, *(b.getCodes(9)));
        b.addCodes(&buf[16], 3);
        CPPUNIT_ASSERT_EQUAL((hyu32)13, b.getSize());
        CPPUNIT_ASSERT_EQUAL((hyu8)126, *(b.getCodes(10)));
        CPPUNIT_ASSERT_EQUAL((hyu8)128, *(b.getCodes(12)));
    }

    void test_addCode(void)
    {
        Bytecode b;
        CPPUNIT_ASSERT_EQUAL((hyu32)0, b.getSize());
        b.addCode<hyu8>(8);
        b.addCode<hyu16>(16);
        b.addCode<hyu32>(32);
        b.addCode<hys8>(-8);
        b.addCode<hys16>(-16);
        b.addCode<hys32>(-32);
        b.addCode<hyf32>(100.5f);
        b.addCode<SymbolID_t>((SymbolID_t) 12345);
        CPPUNIT_ASSERT_EQUAL((hyu32)(18+sizeof(SymbolID_t)), b.getSize());
        const hyu8* p = b.getCodes(0);
        CPPUNIT_ASSERT_EQUAL((hyu8)8, *p);
        CPPUNIT_ASSERT_EQUAL((hyu16)16, Endian::unpack<hyu16>(p+1));
        CPPUNIT_ASSERT_EQUAL((hyu32)32, Endian::unpack<hyu32>(p+3));
        CPPUNIT_ASSERT_EQUAL((hys8)-8, (hys8)*(p+7));
        CPPUNIT_ASSERT_EQUAL((hys16)-16, Endian::unpack<hys16>(p+8));
        CPPUNIT_ASSERT_EQUAL((hys32)-32, Endian::unpack<hys32>(p+10));
        CPPUNIT_ASSERT_EQUAL((hyf32)100.5f, Endian::unpack<hyf32>(p+14));
        CPPUNIT_ASSERT_EQUAL((SymbolID_t)12345, Endian::unpack<SymbolID_t>(p+18));
    }

    void test_removeLastCode(void)
    {
        Bytecode b;
        CPPUNIT_ASSERT_EQUAL((hyu32)0, b.getSize());
        b.addCode<hyu8>(1);
        CPPUNIT_ASSERT_EQUAL(false, b.removeLastCode(99));
        CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
        b.addCode<hyu8>(22);
        CPPUNIT_ASSERT_EQUAL(false, b.removeLastCode(99));
        CPPUNIT_ASSERT_EQUAL((hyu32)2, b.getSize());
        b.addCode<hyu8>(99);
        CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
        CPPUNIT_ASSERT_EQUAL(true, b.removeLastCode(99));
        CPPUNIT_ASSERT_EQUAL((hyu32)2, b.getSize());
        CPPUNIT_ASSERT_EQUAL(false, b.removeLastCode(99));
        CPPUNIT_ASSERT_EQUAL((hyu32)2, b.getSize());
        b.addCode<hyu8>(99);
        b.addCode<hyu8>(99);
        CPPUNIT_ASSERT_EQUAL((hyu32)4, b.getSize());
        CPPUNIT_ASSERT_EQUAL(true, b.removeLastCode(99));
        CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
        CPPUNIT_ASSERT_EQUAL(true, b.removeLastCode(99));
        CPPUNIT_ASSERT_EQUAL((hyu32)2, b.getSize());
        CPPUNIT_ASSERT_EQUAL(false, b.removeLastCode(99));
        CPPUNIT_ASSERT_EQUAL((hyu32)2, b.getSize());
        CPPUNIT_ASSERT_EQUAL(true, b.removeLastCode(22));
        CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyCBytecode);
