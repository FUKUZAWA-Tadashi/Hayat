/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hayat.h"
#include <cppunit/extensions/HelperMacros.h>


using namespace Hayat::Common;
using namespace Hayat::Engine;

class Test_hyStringBuffer : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyStringBuffer);
    CPPUNIT_TEST(test_hyStringBuffer);
    CPPUNIT_TEST(test_hyStringBuffer2);
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

    void test_hyStringBuffer(void)
    {
        StringBuffer* sb = new StringBuffer();
        hyu16 bufSize = sb->bufSize();
        CPPUNIT_ASSERT_EQUAL((hyu16)0, sb->length());
        CPPUNIT_ASSERT_EQUAL(bufSize, (hyu16)(sb->remain() + 1));

        HMD_STRNCPY(sb->bottom(), "asd ", 5);
        sb->concatenated();
        CPPUNIT_ASSERT_EQUAL((hyu16)4, sb->length());
        CPPUNIT_ASSERT_EQUAL(bufSize, (hyu16)(sb->remain() + 1 + 4));
        CPPUNIT_ASSERT_EQUAL('\0', *(sb->bottom()));

        HMD_STRNCPY(sb->bottom(), "qwe", 4);
        sb->concatenated();
        CPPUNIT_ASSERT_EQUAL((hyu16)7, sb->length());
        CPPUNIT_ASSERT_EQUAL(bufSize, (hyu16)(sb->remain() + 1 + 7));
        CPPUNIT_ASSERT_EQUAL('\0', *(sb->bottom()));

        int r = sb->remain();
        char* p = sb->bottom();
        while (--r >= 0) *p++ = '#';
        sb->concatenated();
        CPPUNIT_ASSERT_EQUAL(bufSize, (hyu16)(sb->length() + 1));
        CPPUNIT_ASSERT_EQUAL((hyu16)0, sb->remain());
        CPPUNIT_ASSERT_EQUAL('\0', *(sb->bottom()));

        *(sb->bottom()) = 'a';       // バッファオーバー
        HMD_ASSERT_HALT(sb->concatenated());

        sb->concat("hoge", 4);
        CPPUNIT_ASSERT(sb->bufSize() > bufSize);
    }



    static char tbuf[128];
    static int tbuflen;
    static void copytbuf1(const char* str) {
        HMD_STRNCPY(tbuf, str, HMD_STRLEN(str) + 1);
    }
    static void copytbuf2(const char* str, int len) {
        HMD_STRNCPY(tbuf, str, len+1);
        tbuflen = len;
    }
    void test_hyStringBuffer2(void)
    {
        gGlobalVar.initialize();

        Context* context = gCodeManager.createContext();

        StringBuffer* sb = new StringBuffer();
        CPPUNIT_ASSERT_EQUAL((hyu16)0, sb->length());
        sb->concat(context, Value::fromInt(12), 1);
        sb->concat(context, Value::fromBool(false));
        sb->concat(context, NIL_VALUE, 1);
        sb->concat(context, Value::fromFloat(58.75));
        sb->concat(context, Value(HC_String, (hyu32)"foo"), 1);
        StringBuffer* sb2 = new StringBuffer();
        sb2->concat("bar");
        sb->concat(context, Value::fromObj(sb2->getObj()), 1);

        CPPUNIT_ASSERT(!HMD_STRCMP("12false58.75foobar", sb->top()));
        CPPUNIT_ASSERT_EQUAL((hyu16)18, sb->length());


        StringBuffer::sendStr(context, Value::fromInt(23), copytbuf1);
        CPPUNIT_ASSERT(!HMD_STRCMP("23", tbuf));
        CPPUNIT_ASSERT_EQUAL((size_t)2, HMD_STRLEN(tbuf));
        StringBuffer::sendStr(context, Value::fromFloat(1.25), copytbuf1);
        CPPUNIT_ASSERT(!HMD_STRCMP("1.25", tbuf));
        CPPUNIT_ASSERT_EQUAL((size_t)4, HMD_STRLEN(tbuf));
        StringBuffer::sendStr(context, Value::fromBool(true), copytbuf1);
        CPPUNIT_ASSERT(!HMD_STRCMP("true", tbuf));
        CPPUNIT_ASSERT_EQUAL((size_t)4, HMD_STRLEN(tbuf));
        StringBuffer::sendStr(context, NIL_VALUE, copytbuf1);
        CPPUNIT_ASSERT_EQUAL('\0', tbuf[0]);
        StringBuffer::sendStr(context, Value(HC_String, (void*)"hoge"), copytbuf1);
        CPPUNIT_ASSERT(!HMD_STRCMP("hoge", tbuf));
        CPPUNIT_ASSERT_EQUAL((size_t)4, HMD_STRLEN(tbuf));
        sb->clear();
        sb->concat("fuga");
        StringBuffer::sendStr(context, Value::fromObj(sb->getObj()), copytbuf1);
        CPPUNIT_ASSERT(!HMD_STRCMP("fuga", tbuf));
        CPPUNIT_ASSERT_EQUAL((size_t)4, HMD_STRLEN(tbuf));
        

        StringBuffer::sendStr(context, Value::fromInt(987654), copytbuf2);
        CPPUNIT_ASSERT(!HMD_STRCMP("987654", tbuf));
        CPPUNIT_ASSERT_EQUAL(6, tbuflen);
        StringBuffer::sendStr(context, Value::fromFloat(284.625), copytbuf2);
        CPPUNIT_ASSERT(!HMD_STRCMP("284.625", tbuf));
        CPPUNIT_ASSERT_EQUAL(7, tbuflen);
        StringBuffer::sendStr(context, Value::fromBool(false), copytbuf2);
        CPPUNIT_ASSERT(!HMD_STRCMP("false", tbuf));
        CPPUNIT_ASSERT_EQUAL(5, tbuflen);
        StringBuffer::sendStr(context, NIL_VALUE, copytbuf2);
        CPPUNIT_ASSERT_EQUAL('\0', tbuf[0]);
        StringBuffer::sendStr(context, Value(HC_String, (void*)"honya"), copytbuf2);
        CPPUNIT_ASSERT(!HMD_STRCMP("honya", tbuf));
        CPPUNIT_ASSERT_EQUAL(5, tbuflen);
        sb->clear();
        sb->concat("asd");
        StringBuffer::sendStr(context, Value::fromObj(sb->getObj()), copytbuf2);
        CPPUNIT_ASSERT(!HMD_STRCMP("asd", tbuf));
        CPPUNIT_ASSERT_EQUAL(3, tbuflen);

        gCodeManager.releaseContext(context);
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyStringBuffer);


char Test_hyStringBuffer::tbuf[128];
int Test_hyStringBuffer::tbuflen;
