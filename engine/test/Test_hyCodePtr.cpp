/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyBytecode.h"
#include "hyCodePtr.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Engine;


class Mock_Bytecode : public Bytecode {
public:
    void mockInit(const hyu8* start, const hyu8* end)
    {
        m_bytecodeBuf = start;
        m_bytecodeEndAddr = end;
        m_bytecodeSize = end - start;
    }
};


class Test_hyCodePtr : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyCodePtr);
    CPPUNIT_TEST(test_hyCodePtr);
    CPPUNIT_TEST_SUITE_END();
public:

    void test_hyCodePtr(void)
    {
        const hyu8  data[] = { 0,1,2,3,4,5,6,7,8,9 };
        Mock_Bytecode mockBytecode;
        CodePtr ptr;
        mockBytecode.mockInit(data, data+10);
        ptr.initialize(data);

        CPPUNIT_ASSERT_EQUAL((hyu8)0, *ptr);
        ptr += 1;
        CPPUNIT_ASSERT_EQUAL(&data[1], ptr.addr());
        ptr += 1;
        CPPUNIT_ASSERT_EQUAL((hyu8)2, *ptr);
        ptr += 4;
        CPPUNIT_ASSERT_EQUAL((hyu8)6, *ptr);
        ptr.advance(-2);
        CPPUNIT_ASSERT_EQUAL((hyu8)4, *ptr);
        ptr.jump(&data[8]);
        CPPUNIT_ASSERT_EQUAL((hyu8)8, *ptr);
        CPPUNIT_ASSERT_EQUAL(&data[8], ptr.addr());
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyCodePtr);
