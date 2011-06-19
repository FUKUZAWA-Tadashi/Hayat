/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCOperand.h"
#include "hyMemPool.h"
#include "hyEndian.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Compiler;

class Test_hyCOperand : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyCOperand);
    CPPUNIT_TEST(test_addCode);
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

    void test_addCode(void)
    {
        Bytecode b;
        OPR_INT opr1(-12345); opr1.addCodeTo(b);
        OPR_INT8 opr2(-8); opr2.addCodeTo(b);
        OPR_FLOAT opr3(1234.5f); opr3.addCodeTo(b);
        OPR_UINT8 opr4(8); opr4.addCodeTo(b);
        OPR_UINT16 opr5(16); opr5.addCodeTo(b);
        OPR_UINT32 opr6(3232); opr6.addCodeTo(b);
        OPR_SYMBOL opr7((SymbolID_t)4321); opr7.addCodeTo(b);
        OPR_METHOD opr8((SymbolID_t)33100); opr8.addCodeTo(b);
        OPR_CLASS opr9((SymbolID_t)1455); opr9.addCodeTo(b);
        OPR_LOCALVAR opr10(147); opr10.addCodeTo(b);
        OPR_MAYLOCALVAR opr11(258); opr11.addCodeTo(b);
        OPR_MEMBVAR opr12((SymbolID_t)369); opr12.addCodeTo(b);
        OPR_CLASSVAR opr13((SymbolID_t)642); opr13.addCodeTo(b);
        OPR_GLOBALVAR opr14((SymbolID_t)321); opr14.addCodeTo(b);
        OPR_CONSTVAR opr15((SymbolID_t)543); opr15.addCodeTo(b);
        OPR_RELATIVE opr16(-357); opr16.addCodeTo(b);
        const hyu8* p = b.getCodes(0);
        static const size_t SS = sizeof(SymbolID_t);
        CPPUNIT_ASSERT_EQUAL((hys32)-12345, Endian::unpack<hys32>(p));
        CPPUNIT_ASSERT_EQUAL((hys8)-8, (hys8)*(p+4));
        CPPUNIT_ASSERT_EQUAL(1234.5f, Endian::unpack<hyf32>(p+5));
        CPPUNIT_ASSERT_EQUAL((hyu8)8, *(p+9));
        CPPUNIT_ASSERT_EQUAL((hyu16)16, Endian::unpack<hyu16>(p+10));
        CPPUNIT_ASSERT_EQUAL((hyu32)3232, Endian::unpack<hyu32>(p+12));
        CPPUNIT_ASSERT_EQUAL((SymbolID_t)4321, Endian::unpack<SymbolID_t>(p+16));
        CPPUNIT_ASSERT_EQUAL((SymbolID_t)33100, Endian::unpack<SymbolID_t>(p+16+SS));
        CPPUNIT_ASSERT_EQUAL((SymbolID_t)1455, Endian::unpack<SymbolID_t>(p+16+SS*2));
        CPPUNIT_ASSERT_EQUAL((hys16)147, Endian::unpack<hys16>(p+16+SS*3));
        CPPUNIT_ASSERT_EQUAL((hys16)258, Endian::unpack<hys16>(p+18+SS*3));
        CPPUNIT_ASSERT_EQUAL((SymbolID_t)369, Endian::unpack<SymbolID_t>(p+20+SS*3));
        CPPUNIT_ASSERT_EQUAL((SymbolID_t)642, Endian::unpack<SymbolID_t>(p+22+SS*3));
        CPPUNIT_ASSERT_EQUAL((SymbolID_t)321, Endian::unpack<SymbolID_t>(p+24+SS*3));
        CPPUNIT_ASSERT_EQUAL((SymbolID_t)543, Endian::unpack<SymbolID_t>(p+26+SS*3));
        CPPUNIT_ASSERT_EQUAL((hys32)-357, Endian::unpack<hys32>(p+28+SS*3));
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyCOperand);
