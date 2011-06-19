/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyEndian.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;

class Test_hyEndian : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyEndian);
    CPPUNIT_TEST(test_unpack);
    CPPUNIT_TEST(test_pack);
    CPPUNIT_TEST(test_pack_unpack);
    CPPUNIT_TEST(test_rewrite);
    CPPUNIT_TEST(test_align);
    CPPUNIT_TEST_SUITE_END();
public:

    void test_unpack(void)
    {
        hyu8 a[] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0 };
    
        CPPUNIT_ASSERT_EQUAL((hys32)0x78563412, (Endian::unpackE<Endian::LITTLE, hys32>(&a[0])));
        CPPUNIT_ASSERT_EQUAL((hyu32)0xdebc9a78, (Endian::unpackE<Endian::LITTLE, hyu32>(&a[3])));
        CPPUNIT_ASSERT_EQUAL((hys16)0x5634, (Endian::unpackE<Endian::LITTLE, hys16>(&a[1])));
        CPPUNIT_ASSERT_EQUAL((hyu16)0xf0de, (Endian::unpackE<Endian::LITTLE, hyu16>(&a[6])));

        CPPUNIT_ASSERT_EQUAL((hys32)0x9abcdef0, (Endian::unpackE<Endian::BIG, hys32>(&a[4])));
        CPPUNIT_ASSERT_EQUAL((hyu32)0x3456789a, (Endian::unpackE<Endian::BIG, hyu32>(&a[1])));
        CPPUNIT_ASSERT_EQUAL((hys16)0x5678, (Endian::unpackE<Endian::BIG, hys16>(&a[2])));
        CPPUNIT_ASSERT_EQUAL((hyu16)0xbcde, (Endian::unpackE<Endian::BIG, hyu16>(&a[5])));

        Endian::packE<Endian::LITTLE, hyf32>(a, 123.45f);
        a[4] = a[3];
        a[5] = a[2];
        a[6] = a[1];
        a[7] = a[0];

        if (Endian::MY_ENDIAN == Endian::LITTLE) {
            CPPUNIT_ASSERT_EQUAL(123.45f, Endian::unpack<hyf32>(&a[0]));
            CPPUNIT_ASSERT_EQUAL(123.45f, (Endian::unpackE<Endian::BIG, hyf32>(&a[4])));
        } else {
            CPPUNIT_ASSERT_EQUAL(123.45f, Endian::unpack<hyf32>(&a[4]));
            CPPUNIT_ASSERT_EQUAL(123.45f, (Endian::unpackE<Endian::LITTLE, hyf32>(&a[0])));
        }

    }


    void test_pack(void)
    {
        hyu8 buf[8];
        Endian::packE<Endian::LITTLE>(buf, (hys32)0x3456789a);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x9a, buf[0]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x78, buf[1]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x56, buf[2]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x34, buf[3]);
        Endian::packE<Endian::LITTLE>(buf, (hyu32)0xba987654);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x54, buf[0]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x76, buf[1]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x98, buf[2]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0xba, buf[3]);
        Endian::packE<Endian::LITTLE>(buf, (hyu16)0x1357);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x57, buf[0]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x13, buf[1]);
        Endian::packE<Endian::LITTLE>(buf, (hys16)0xfedc);
        CPPUNIT_ASSERT_EQUAL((hyu8)0xdc, buf[0]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0xfe, buf[1]);

        Endian::packE<Endian::BIG>(buf, (hys32)0x3456789a);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x34, buf[0]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x56, buf[1]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x78, buf[2]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x9a, buf[3]);
        Endian::packE<Endian::BIG>(buf, (hyu32)0xba987654);
        CPPUNIT_ASSERT_EQUAL((hyu8)0xba, buf[0]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x98, buf[1]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x76, buf[2]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x54, buf[3]);
        Endian::packE<Endian::BIG>(buf, (hyu16)0x1357);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x13, buf[0]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0x57, buf[1]);
        Endian::packE<Endian::BIG>(buf, (hys16)0xfedc);
        CPPUNIT_ASSERT_EQUAL((hyu8)0xfe, buf[0]);
        CPPUNIT_ASSERT_EQUAL((hyu8)0xdc, buf[1]);

        Endian::packE<Endian::LITTLE>(buf, (hyf32)123.456f);
        Endian::packE<Endian::BIG>(&buf[4], (hyf32)123.456f);
        CPPUNIT_ASSERT_EQUAL(buf[0], buf[7]);
        CPPUNIT_ASSERT_EQUAL(buf[1], buf[6]);
        CPPUNIT_ASSERT_EQUAL(buf[2], buf[5]);
        CPPUNIT_ASSERT_EQUAL(buf[3], buf[4]);
    }

    void test_pack_unpack(void)
    {
        hyu8 buf[4];
        Endian::pack(buf, (hys32)-12345678);
        hys32 v_hys32 = Endian::unpack<hys32>(buf);
        CPPUNIT_ASSERT_EQUAL((hys32)-12345678, v_hys32);

        Endian::pack(buf, (hyu32)87654321);
        hyu32 v_hyu32 = Endian::unpack<hyu32>(buf);
        CPPUNIT_ASSERT_EQUAL((hyu32)87654321, v_hyu32);

        Endian::pack(buf, (hys16)-32109);
        hys16 v_hys16 = Endian::unpack<hys16>(buf);
        CPPUNIT_ASSERT_EQUAL((hys16)-32109, v_hys16);

        Endian::pack(buf, (hyu16)65432);
        hyu16 v_hyu16 = Endian::unpack<hyu16>(buf);
        CPPUNIT_ASSERT_EQUAL((hyu16)65432, v_hyu16);

        Endian::pack(buf, (hyf32)3.14159265f);
        hyf32 v_hyf32 = Endian::unpack<hyf32>(buf);
        CPPUNIT_ASSERT_EQUAL((hyf32)3.14159265f, v_hyf32);

        if (sizeof(SymbolID_t) == 2) {
            Endian::pack(buf, (SymbolID_t)0x2468);
            SymbolID_t v_sym = Endian::unpack<SymbolID_t>(buf);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)0x2468, v_sym);
        } else {
            Endian::pack(buf, (SymbolID_t)0x24681357);
            SymbolID_t v_sym = Endian::unpack<SymbolID_t>(buf);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)0x24681357, v_sym);
        }
    }

    void test_rewrite(void)
    {
        hyu8 buf[4];
        Endian::packE<Endian::LITTLE, hyu32>(buf, (hyu32)0x12345678);
        Endian::rewriteE<Endian::LITTLE, hyu32>((hyu32*)buf);
        CPPUNIT_ASSERT_EQUAL((hyu32)0x12345678, *(hyu32*)buf);
        Endian::packE<Endian::BIG, hyu32>(buf, (hyu32)0x23456789);
        Endian::rewriteE<Endian::BIG, hyu32>((hyu32*)buf);
        CPPUNIT_ASSERT_EQUAL((hyu32)0x23456789, *(hyu32*)buf);
    }

    void test_align(void)
    {
        CPPUNIT_ASSERT_EQUAL(0x234, alignInt<2>(0x234));
        CPPUNIT_ASSERT_EQUAL(0x236, alignInt<2>(0x235));
        CPPUNIT_ASSERT_EQUAL(0x236, alignInt<2>(0x236));
        CPPUNIT_ASSERT_EQUAL(0x238, alignInt<2>(0x237));
        CPPUNIT_ASSERT_EQUAL(0x238, alignInt<4>(0x238));
        CPPUNIT_ASSERT_EQUAL(0x23c, alignInt<4>(0x239));
        CPPUNIT_ASSERT_EQUAL(0x23c, alignInt<4>(0x23a));
        CPPUNIT_ASSERT_EQUAL(0x23c, alignInt<4>(0x23b));
        CPPUNIT_ASSERT_EQUAL(0x23c, alignInt<4>(0x23c));
        CPPUNIT_ASSERT_EQUAL(0x240, alignInt<4>(0x23d));
        CPPUNIT_ASSERT_EQUAL(0x240, alignInt<8>(0x23e));
        CPPUNIT_ASSERT_EQUAL(0x248, alignInt<8>(0x241));
        CPPUNIT_ASSERT_EQUAL(0x250, alignInt<8>(0x249));

        CPPUNIT_ASSERT_EQUAL(3, alignInt<3>(2));
        CPPUNIT_ASSERT_EQUAL(3, alignInt<3>(3));
        CPPUNIT_ASSERT_EQUAL(6, alignInt<3>(4));
        CPPUNIT_ASSERT_EQUAL(6, alignInt<3>(5));


        hyu8* p = (hyu8*)0x0120;
        alignPtr<2>(&p);
        CPPUNIT_ASSERT_EQUAL(0x0120, (int)p);
        p = (hyu8*)0x0121;
        alignPtr<2>(&p);
        CPPUNIT_ASSERT_EQUAL(0x0122, (int)p);
        p = (hyu8*)0x0122;
        alignPtr<2>(&p);
        CPPUNIT_ASSERT_EQUAL(0x0122, (int)p);
        p = (hyu8*)0x0123;
        alignPtr<2>(&p);
        CPPUNIT_ASSERT_EQUAL(0x0124, (int)p);
        p = (hyu8*)0x0120;
        alignPtr<4>(&p);
        CPPUNIT_ASSERT_EQUAL(0x0120, (int)p);
        p = (hyu8*)0x0121;
        alignPtr<4>(&p);
        CPPUNIT_ASSERT_EQUAL(0x0124, (int)p);
        p = (hyu8*)0x0122;
        alignPtr<4>(&p);
        CPPUNIT_ASSERT_EQUAL(0x0124, (int)p);
        p = (hyu8*)0x0123;
        alignPtr<4>(&p);
        CPPUNIT_ASSERT_EQUAL(0x0124, (int)p);
        p = (hyu8*)0x0125;
        alignPtr<4>(&p);
        CPPUNIT_ASSERT_EQUAL(0x0128, (int)p);
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyEndian);
