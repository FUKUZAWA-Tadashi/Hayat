/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyStringBuffer.h"
#include "hyClass.h"
#include "hyContext.h"
#include "hyBytecode.h"
#include "HSymbol.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;


// サイズ拡張する時の余裕分
static const hyu16 EXPAND_MARGIN = 32;
static const hyu16 EXPAND_MASK = ~31;


void* StringBuffer::operator new(size_t size)
{
#ifndef HMD_DEBUG
    (void)size;
#endif
    HMD_DEBUG_ASSERT(size == sizeof(StringBuffer));
    Object* obj = Object::create(HC_StringBuffer, sizeof(StringBuffer) + sizeof(hyu32));
    return (void*) obj->cppObj<StringBuffer>(HSym_StringBuffer);
}

void StringBuffer::operator delete(void* p)
{
    Object::fromCppObj(p)->destroy();
}

void StringBuffer::initialize(hyu16 bufSize)
{
    m_buffer = gMemPool->allocT<char>(bufSize, "STRB");
    m_bufSize = bufSize;
    clear();
}

void StringBuffer::finalize(void)
{
    gMemPool->free(m_buffer);
    m_buffer = NULL;
    m_bufSize = 0;
    m_length = 0;
}

void StringBuffer::clear(void)
{
    m_length = 0;
    *m_buffer = '\0';
}


void StringBuffer::reserve(hyu16 len)
{
    if (remain() < len || m_bufSize == 0) {
        hyu16 newSize = (m_length + len + EXPAND_MARGIN) & EXPAND_MASK;
        char* newBuf = gMemPool->reallocT<char>(m_buffer, newSize, m_length+1);
        if (newBuf == NULL) {
            HMD_FATAL_ERROR("StringBuffer::reserve : cannot realloc %d", newSize);
        }
        m_buffer = newBuf;
        m_bufSize = newSize;
    }
}
    

void StringBuffer::concat(const char* str, int len)
{
    if (len < 0)
        len = HMD_STRLEN(str);
    reserve(len);
    char* btm = bottom();
    memcpy(btm, str, len);
    *(btm + len) = '\0';
    concatenated();
}

void StringBuffer::concat(Context* context, Value d, int mode)
{
    context->pushObj(getObj());
    context->pushInt(mode);
    bool b = context->execMethod(d, HSym_concatToStringBuffer, 2);
    HMD_DEBUG_ASSERT(b);
    context->pop();
}

void StringBuffer::addBinary(const void* bin, hyu32 size)
{
    reserve(size);
    char* btm = bottom();
    memcpy(btm, bin, size);
    *(btm + size) = '\0';
    m_length += size;
}


void StringBuffer::sprintf(const char* fmt,...)
{
    va_list ap;
    va_start(ap, fmt);
    vsprintf(fmt, ap);
    va_end(ap);
}

void StringBuffer::vsprintf(const char* fmt, va_list ap)
{
    char tmp[1024];
    HMD_VSNPRINTF(tmp, 1024, fmt, ap);
    HMD_DEBUG_ASSERT(HMD_STRLEN(tmp) < 1024);
    concat(tmp, HMD_STRLEN(tmp));
}


void StringBuffer::concatenated(void)
{
    int rem = remain();
    char* p = bottom();
    while (*p != '\0' && rem > 0) {
        ++p;
        --rem;
    }
    HMD_ASSERTMSG((rem > 0 || *p == '\0'), M_M("StringBuffer overflow detected"));
    m_length = m_bufSize - rem - 1;
}

void StringBuffer::calcLength(void)
{
    int rem = m_bufSize - 1;
    char* p = top();
    while (*p != '\0' && rem > 0) {
        ++p;
        --rem;
    }
    HMD_ASSERTMSG((rem > 0 || *p == '\0'), M_M("StringBuffer overflow detected"));
    m_length = m_bufSize - rem - 1;
}

void StringBuffer::binaryAdded(const void* endAddr)
{
    HMD_ASSERTMSG((top() <= endAddr) && (endAddr <= top()+m_bufSize),
                  M_M("address out of StringBuffer"));
    int endPos = (const char*)endAddr - top();
    if (m_length < endPos)
        m_length = endPos;
}


void StringBuffer::sendStr(Context* context, Value d, void(*func)(const char*))
{
    switch (d.getType()->getSymbol()) {
    case HSym_NilClass:
        func("");
        break;
    case HSym_String:
        func((const char*) d.ptrData);
        break;
    case HSym_StringBuffer:
        {
            StringBuffer* sb = d.toObj()->cppObj<StringBuffer>();
            func(sb->top());
        }
        break;
    default:
        {
            StringBuffer* sb = new StringBuffer();
            sb->concat(context, d, 1);
            func(sb->top());
            delete sb;
        }
    }
}

void StringBuffer::sendStr(Context* context, Value d, void(*func)(const char*,int))
{
    switch (d.getType()->getSymbol()) {
    case HSym_NilClass:
        func("", 0);
        break;
    case HSym_String:
        func((const char*) d.ptrData, HMD_STRLEN((const char*) d.ptrData));
        break;
    case HSym_StringBuffer:
        {
            StringBuffer* sb = d.toObj()->cppObj<StringBuffer>();
            func(sb->top(), sb->length());
        }
        break;
    default:
        {
            StringBuffer* sb = new StringBuffer();
            sb->concat(context, d, 1);
            func(sb->top(), sb->length());
            delete sb;
        }
    }
}


// d1とd2を文字列比較する。
// d1とd2の型はStringとStringBufferのみ受け付け、そうでなければ
// falseを返す。
hys32 StringBuffer::compare(Value d1, Value d2)
{
    SymbolID_t d1sym = d1.getTypeSymbol();
    SymbolID_t d2sym = d2.getTypeSymbol();
    const char* p1;
    const char* p2;
    hyu32 len1;
    hyu32 len2;
    if (d1sym == HSym_String) {
        p1 = (const char*) d1.ptrData;
        len1 = HMD_STRLEN(p1);
    } else if (d1sym == HSym_StringBuffer) {
        StringBuffer* sb1 = d1.toObj()->cppObj<StringBuffer>();
        p1 = sb1->top();
        len1 = sb1->length();
    } else {
        return COMPARE_ERROR;
    }
    if (d2sym == HSym_String) {
        p2 = (const char*) d2.ptrData;
        len2 = HMD_STRLEN(p2);
    } else if (d2sym == HSym_StringBuffer) {
        StringBuffer* sb2 = d2.toObj()->cppObj<StringBuffer>();
        p2 = sb2->top();
        len2 = sb2->length();
    } else {
        return COMPARE_ERROR;
    }
    if (len1 != len2)
        return COMPARE_ERROR;
    return memcmp(p1, p2, len1+1);  // 最後の \0 も比較する
}


// 文字列からハッシュコードを計算
hyu32 StringBuffer::calcHashCode(const char* p)
{
    hyu32 h = 0;
    hyu8 c;
    while ((c = (hyu8)*p++) != '\0') {
        h = h * 31 + c;
    }
    return h;
}
