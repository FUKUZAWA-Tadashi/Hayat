/*  -*- coding: sjis-dos -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


#include "hyStringReader.h"
#include "hyStringBuffer.h"
#include "hyDebug.h"
#include <stdlib.h>

using namespace Hayat::Engine;


StringReader::StringReader(void)
    : m_sbObj(NULL), m_str(NULL), m_strlen(0), m_pos(0)
{
}

void StringReader::initialize(Object* sbObj)
{
    m_sbObj = sbObj;
    m_str = NULL;
    m_strlen = 0;
    m_pos = 0;
}

void StringReader::initialize(const char* str)
{
    m_sbObj = NULL;
    m_str = str;
    m_strlen = HMD_STRLEN(str);
    m_pos = 0;
}

const char* StringReader::m_addr(hys32 pos)
{
    if (m_sbObj != NULL)
        return m_sbObj->cppObj<StringBuffer>()->top() + pos;
    return m_str + pos;
    
}

hys32 StringReader::setPos(hys32 pos)
{
    if (pos < 0 || pos > length())
        return -1;
    m_pos = pos;
    return pos;
}

hys32 StringReader::length(void)
{
    if (m_sbObj != NULL)
        return m_sbObj->cppObj<StringBuffer>()->length();
    return m_strlen;
}

hys32 StringReader::getc(void)
{
    if (m_pos >= length())
        return -1;
    return (hys32)(hyu8)*m_addr(m_pos++);
}

StringBuffer* StringReader::gets(const char* delim)
{
    hys32 len = length();
    if (m_pos >= len)
        return NULL;
    hys32 head = m_pos;
    if (search(delim))
        ++m_pos;
    if (head == m_pos)
        return NULL;
    return substr(head, m_pos);
}

bool StringReader::search(const char* searchChars)
{
    hys32 len = length();
    const char* p = m_addr(m_pos);
    hys32 savedPos = m_pos;
    while (m_pos < len) {
        if (*searchChars == *p)     // searchChars = "" ‚Å‚àOK
            return true;
        if (strchr(searchChars, *p) != NULL)  // searchChars‚ª•¡”•¶Žš‚Ìê‡
            return true;
        ++m_pos;
        ++p;
    }
    m_pos = savedPos;
    return false;
}

hys32 StringReader::skip(const char* skipChars)
{
    hys32 len = length();
    const char* p = m_addr(m_pos);
    while (m_pos < len) {
        if (strchr(skipChars, *p) == NULL)
            break;
        ++m_pos;
        ++p;
    }
    return m_pos;
}

hys32 StringReader::skip(hys32 c1, hys32 c2)
{
    hys32 len = length();
    const char* p = m_addr(m_pos);
    while (m_pos < len) {
        hys32 c = (hyu8)*p;
        if ((c < c1) || (c > c2))
            break;
        ++m_pos;
        ++p;
    }
    return m_pos;
}

StringBuffer* StringReader::substr(hys32 start, hys32 end)
{
    HMD_ASSERT(end <= length());
    HMD_ASSERT(start <= end);
    HMD_ASSERT(0 <= start);
    hys32 len = end - start;
    StringBuffer* sb = new StringBuffer(len+1);
    sb->addBinary(m_addr(start), len);
    return sb;
}

StringBuffer* StringReader::read(hys32 len)
{
    hys32 start = m_pos;
    hys32 mlen = length();
    if (len < 0)
        m_pos = mlen;
    else {
        m_pos += + len;
        if (m_pos >= mlen)
            m_pos = mlen;
    }
    return substr(start, m_pos);
}


hys32 StringReader::getInt(void)
{
    const char* p = m_addr(m_pos);
    const char* lastp = m_addr(length());
    if (p >= lastp)
        return 0;

    hys32 sign = 1;
    hys32 radix = 10;

    char c;
    for ( ; p < lastp; ++p) {
        c = *p;
        if (c == '+')
            ;
        else if (c == '-')
            sign *= -1;
        else
            break;
    }
    if (c == '0') {
        c = p[1];
        if ((c == 'b') || (c == 'B')) {
            p += 2;
            radix = 2;
        } else if ((c == 'o') || (c == 'O')) {
            p += 2;
            radix = 8;
        } else if ((c == 'x') || (c == 'X')) {
            p += 2;
            radix = 16;
        }
    }
    if ((c == '+') || (c == '-')) {
        // ‚±‚±‚Å +- ‚ÍA 0+, 0- ‚µ‚©‚ ‚è‚¦‚È‚¢
        m_pos = p + 1 - m_addr(0);
        return 0;
    }

    char* endp;
    hys32 val = (hys32) strtol(p, &endp, radix);
    if (p == endp)
        return 0;
    if (endp > lastp)
        return 0;
    m_pos = endp - m_addr(0);
    if (sign == -1)
        val = -val;
    return val;
}

hyf32 StringReader::getFloat(void)
{
    const char* p = m_addr(m_pos);
    char* endp;

    hyf32 val = (hyf32) strtod(p, &endp);
    if (p == endp)
        return 0.0f;
    hys32 endPos = endp - m_addr(0);
    if (endPos > length())
        return 0.0f;
    m_pos = endPos;
    return val;
}

void StringReader::m_GC_mark(void)
{
    Debug::incMarkTreeLevel();
    if (m_sbObj == NULL)
        return;
    GC::markObjP(&m_sbObj);
    Debug::decMarkTreeLevel();
}
