/*  -*- coding: sjis-dos -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyPack.h"
#include "hyStringBuffer.h"
#include "hyValueArray.h"
#include "hyEndian.h"
#include <string.h>

using namespace Hayat::Engine;


// n	big endian unsigned 16bit
// N	big endian 32bit
// v	little endian unsigned 16bit
// V	little endian 32bit
// c 	signed char 8bit
// C	unsigned char 8bit
// s	signed short 16bit
// S	unsigned short 16bit
// i	signed int 32bit
// f	float 32bit
// a	ナルバイト区切り文字列
// A	バイト数区切り文字列 : 最大65535バイトまで
// x	pack:ナルバイト / unpack:1byte読み飛ばし


static const char* TEMPL_CHARS = "nNvVcCsSifaAx";

static char getNextTemplChar(const char*& refTempl)
{
    char c = *refTempl;
    while (c != '\0') {
        ++refTempl;
        if (strchr(TEMPL_CHARS, c) != NULL) {
            return c;
        }
        c = *refTempl;
    }
    return '\0';
}

static int getTemplCount(const char*& refTempl)
{
    char c = *refTempl;
    if (c == '*') {
        ++refTempl;
        return -1;
    }    

    if (c < '0' || c > '9')
        return 1;

    int count = 0;
    while (c >= '0' && c <= '9') {
        count *= 10;
        count += c - '0';
        c = *++refTempl;
    }
    return count;
}


void Pack::pack(const char* templ, ValueArray* va, StringBuffer* sb)
{
    hyu8 buf[4];

    for (hyu32 aidx = 0; aidx < va->size(); ) {
        char c = getNextTemplChar(templ);
        if (c == '\0')
            return;
        int count = getTemplCount(templ);
        while ((count != 0) && (aidx < va->size())) {
            Value& val = va->nth(aidx);
            hys32 x;
            switch (c) {
            case 'n':
                x = val.toInt();
                Endian::packE<Endian::BIG, hyu16>(buf, (hyu16)x);
                sb->addBinary(buf, 2);
                break;

            case 'N':
                x = val.toInt();
                Endian::packE<Endian::BIG, hys32>(buf, (hys32)x);
                sb->addBinary(buf, 4);
                break;

            case 'v':
                x = val.toInt();
                Endian::packE<Endian::LITTLE, hyu16>(buf, (hyu16)x);
                sb->addBinary(buf, 2);
                break;

            case 'V':
                x = val.toInt();
                Endian::packE<Endian::LITTLE, hys32>(buf, (hys32)x);
                sb->addBinary(buf, 4);
                break;

            case 'c':
            case 'C':
                x = val.toInt();
                buf[0] = (hyu8)x;
                sb->addBinary(buf, 1);
                break;

            case 's':
                x = val.toInt();
                Endian::pack<hys16>(buf, (hys16)x);
                sb->addBinary(buf, 2);
                break;

            case 'S':
                x = val.toInt();
                Endian::pack<hyu16>(buf, (hyu16)x);
                sb->addBinary(buf, 2);
                break;

            case 'i':
                x = val.toInt();
                Endian::pack(buf, x);
                sb->addBinary(buf, 4);
                break;

            case 'f':
                {
                    union { hys32 i; hyf32 f; } u;
                    u.f = val.toFloat();
                    Endian::pack(buf, u.i);
                    sb->addBinary(buf, 4);
                }                    
                break;

            case 'a':
                {
                    SymbolID_t tsym = val.getTypeSymbol();
                    HMD_ASSERTMSG(tsym == HSym_String || tsym == HSym_StringBuffer,
                                  M_M("pack(\"a\") for non string value"));
                    const char* str = val.toString();
                    sb->addBinary(str, HMD_STRLEN(str)+1);
                }
                break;

            case 'A':
                {
                    const char* str;
                    hyu32 len;
                    if (val.getTypeSymbol() == HSym_String) {
                        str = val.toString();
                        len = HMD_STRLEN(str);
                    } else if (val.getTypeSymbol() == HSym_StringBuffer) {
                        StringBuffer* p = val.toCppObj<StringBuffer>();
                        str = p->top();
                        len = p->length();
                    } else {
                        HMD_FATAL_ERROR(M_M("pack(\"A\") for non string value"));
                    }
                    HMD_ASSERT(len <= 0xffff);
                    Endian::pack<hyu16>(buf, (hyu16)len);
                    sb->addBinary(buf, 2);
                    sb->addBinary(str, len);
                }
                break;


            case 'x':
                buf[0] = '\0';
                sb->addBinary(buf, 1);
                --aidx; // 後で実行される ++aidx を打ち消すため
                break;
                
            default:
                HMD_FATAL_ERROR(M_M("bug on Pack::pack()"));
            }
            if (count > 0) --count;
            ++aidx;
        }
    }
}

void Pack::unpack(const char* templ, ValueArray* va, StringBuffer* sb)
{
    int rest = sb->length();
    const hyu8* str = (const hyu8*) sb->top();

    while (rest > 0) {
        char c = getNextTemplChar(templ);
        if (c == '\0')
            return;
        int count = getTemplCount(templ);
        while (count != 0) {
            switch (c) {
            case 'n':
                {
                    if (rest < 2) return;
                    hyu16 x = Endian::unpackE<Endian::BIG, hyu16>(str);
                    va->add(Value::fromInt(x));
                    str += 2; rest -= 2;
                }
                break;

            case 'N':
                {
                    if (rest < 4) return;
                    hys32 x = Endian::unpackE<Endian::BIG, hys32>(str);
                    va->add(Value::fromInt(x));
                    str += 4; rest -= 4;
                }
                break;

            case 'v':
                {
                    if (rest < 2) return;
                    hyu16 x = Endian::unpackE<Endian::LITTLE, hyu16>(str);
                    va->add(Value::fromInt(x));
                    str += 2; rest -= 2;
                }
                break;

            case 'V':
                {
                    if (rest < 4) return;
                    hys32 x = Endian::unpackE<Endian::LITTLE, hys32>(str);
                    va->add(Value::fromInt(x));
                    str += 4; rest -= 4;
                }
                break;

            case 'c':
                {
                    if (rest < 1) return;
                    hys8 x = Endian::unpackP<hys8>(&str);
                    va->add(Value::fromInt(x));
                    rest -= 1;
                }
                break;

            case 'C':
                {
                    if (rest < 1) return;
                    hyu8 x = Endian::unpackP<hyu8>(&str);
                    va->add(Value::fromInt(x));
                    rest -= 1;
                }
                break;

            case 's':
                {
                    if (rest < 2) return;
                    hys16 x = Endian::unpackP<hys16>(&str);
                    va->add(Value::fromInt(x));
                    rest -= 2;
                }
                break;

            case 'S':
                {
                    if (rest < 2) return;
                    hyu16 x = Endian::unpackP<hyu16>(&str);
                    va->add(Value::fromInt(x));
                    rest -= 2;
                }
                break;

            case 'i':
                {
                    if (rest < 4) return;
                    hys32 x = Endian::unpackP<hys32>(&str);
                    va->add(Value::fromInt(x));
                    rest -= 4;
                }
                break;

            case 'f':
                {
                    if (rest < 4) return;
                    union { hys32 i; hyf32 f; } u;
                    u.i = Endian::unpackP<hys32>(&str);
                    va->add(Value::fromFloat(u.f));
                    rest -= 4;
                }
                break;


            case 'a':
                {
                    int len = HMD_STRLEN((const char*)str);
                    if (rest < len) return;
                    StringBuffer* p = new StringBuffer(len+1);
                    p->concat((const char*)str, len);
                    va->add(Value::fromObj(p->getObj()));
                    str += len+1;
                    rest -= len+1;
                }
                break;

            case 'A':
                {
                    if (rest < 2) return;
                    int len = Endian::unpackP<hyu16>(&str);
                    rest -= 2;
                    if (rest < len) return;
                    StringBuffer* p = new StringBuffer(len+1);
                    p->addBinary(str, len);
                    va->add(Value::fromObj(p->getObj()));
                    str += len;
                    rest -= len;
                }
                break;


            case 'x':
                if (rest < 1) return;
                ++str; --rest;
                break;

            default:
                HMD_FATAL_ERROR("bug on Pack::unpack()");
            }
            if (count > 0) --count;
        }
    }
}
