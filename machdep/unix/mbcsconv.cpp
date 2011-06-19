/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "mbcsconv.h"
#include <locale.h>


#ifdef __CYGWIN__
// define one of below
#undef CYGWIN_VER_1_5
#define CYGWIN_VER_1_7
#endif


#ifdef __CYGWIN__
# ifdef CYGWIN_VER_1_5
#  define HMD_DEFAULT_LOCALE  "C-SJIS"
# else
#  define HMD_DEFAULT_LOCALE  "ja_JP.SJIS"
# endif
#else
# define HMD_DEFAULT_LOCALE  "ja_JP.sjis"
#endif


using namespace Hayat::Parser;

iconv_t MBCSConv::m_iconv_desc = (iconv_t)-1;
#ifdef __CYGWIN__
iconv_t MBCSConv::m_iconv_mbwc_desc = (iconv_t)-1;
#endif
const char* MBCSConv::m_prevLocale = "C";
#ifdef AVOID_MBTOWC_SJIS_SANITIZE
bool MBCSConv::m_sjis = false;
#endif

static const char* charset2locale(const char* charset)
{
    if (charset == NULL)
        return HMD_DEFAULT_LOCALE;

    static struct m_LC_st {
        const char* a;
        const char* b;
    } m_clconv[] = {
#ifdef __CYGWIN__
        { "SJIS", "ja_JP.SJIS" },
        { "CP932", "ja_JP.SJIS" },
        { "EUCJP", "ja_JP.EUCJP" },
        { "UTF-8", "ja_JP.UTF-8" }
#else
        { "SJIS", "ja_JP.sjis" },
        { "CP932", "ja_JP.sjis" },
        { "EUCJP", "ja_JP.eucjp" },
        { "UTF-8", "ja_JP.utf8" }
#endif
    };

    const char* loc = charset;
    for (int i = 0; i < (int)(sizeof(m_clconv)/sizeof(m_clconv[0])); ++i) {
        if (strcasecmp(m_clconv[i].a, charset) == 0) {
            loc = m_clconv[i].b;
            break;
        }
    }
    return loc;
}


bool MBCSConv::initialize(const char* fromcode, const char* tocode)
{
    if (tocode == NULL) tocode = fromcode;
    const char* inLocale = charset2locale(fromcode);
#ifdef AVOID_MBTOWC_SJIS_SANITIZE
    if (strstr(inLocale, "SJIS") != NULL)
        m_sjis = true;
    else if (strstr(inLocale, "sjis") != NULL)
        m_sjis = true;
    else
        m_sjis = false;
#endif
    m_prevLocale = setlocale(LC_CTYPE, inLocale);
    //HMD_PRINTF("MBCSConv::initialize(%s,%s) locale=%s prevlocale=%s\n",fromcode,tocode,inLocale,m_prevLocale);
    HMD_DEBUG_ASSERT(m_prevLocale != NULL);
    HMD_DEBUG_ASSERT(m_iconv_desc == (iconv_t)-1);
#ifdef __CYGWIN__
    if (fromcode != NULL)
        m_iconv_mbwc_desc = iconv_open("UTF-16LE", fromcode);
#endif
    if (tocode != NULL && fromcode != NULL) {
        if (HMD_STRCMP(tocode, fromcode) == 0)
            return true;
        m_iconv_desc = iconv_open(tocode, fromcode);
        if (m_iconv_desc != (iconv_t)-1)
            return true;
    }
    return false;
}

void MBCSConv::finalize(void)
{
    if (m_iconv_desc != (iconv_t)-1) {
        iconv_close(m_iconv_desc);
    }
    m_iconv_desc = (iconv_t)-1;
#ifdef __CYGWIN__
    if (m_iconv_mbwc_desc != (iconv_t)-1) {
        iconv_close(m_iconv_mbwc_desc);
    }
    m_iconv_mbwc_desc = (iconv_t)-1;
#endif
    setlocale(LC_CTYPE, m_prevLocale);
}

#ifdef __CYGWIN__
size_t MBCSConv::cygwin_mbtowc(wchar_t* pwc, const char* s, size_t n)
{
    size_t len = mbtowc(pwc, s, n);
    if (len < 2)
        return len;
    if (m_iconv_mbwc_desc == (iconv_t)-1)
        return len;
    wchar_t wbuf[2];
    char* pwbuf = (char*)&wbuf[0];
    size_t wbufsize = sizeof(wchar_t)*2;
    size_t ilen = len;
#ifdef CYGWIN_VER_1_5
    size_t cn = iconv(m_iconv_mbwc_desc, &s, &ilen, &pwbuf, &wbufsize);
#else
    size_t cn = iconv(m_iconv_mbwc_desc, const_cast<char**>(&s), &ilen, &pwbuf, &wbufsize);
#endif
    if (cn == (size_t)-1)
        return 0;
    if (wbuf[0] == (wchar_t)0xfeff) {
        *pwc = wbuf[1];
    } else {
        *pwc = wbuf[0];
    }
    return len;
}
#endif

wchar_t MBCSConv::getCharAt(const char** pInStr, hyu32* pLen)
{
    const char* uptr = *pInStr;
    wchar_t wc;
#ifdef __CYGWIN__
    size_t len = cygwin_mbtowc(&wc, uptr, 6);  // max 6 bytes char
#else
    size_t len = mbtowc(&wc, uptr, 6);  // max 6 bytes char
#endif
    if (len == 0) {
        wc = (wchar_t) (hyu8)*uptr;
    } else if (len == (size_t)-1) {
        // ERROR
        // not change *pInStr
        if (pLen != NULL) *pLen = 0;
        return (wchar_t)uptr[0];
    }
#ifdef AVOID_MBTOWC_SJIS_SANITIZE
    else if (m_sjis && wc == (wchar_t)165) wc = L'\\';
    else if (m_sjis && wc == (wchar_t)0x203e) wc = L'~';
#endif
    *pInStr += len;
    if (pLen != NULL) *pLen = len;
    return wc;
}


hyu32 MBCSConv::conv(const char* inStr, hyu32 inStrLen, char* outBuf, hyu32 outBufSize)
{
    if (m_iconv_desc == (iconv_t)-1) {
        // not initialized, so not convert
        if (outBufSize < inStrLen)
            return (hyu32)-1;
        HMD_STRNCPY(outBuf, inStr, inStrLen+1);
        return inStrLen;
    }

    if (outBufSize < 2)
        return 0;
    outBufSize -= 2;

#ifdef CYGWIN_VER_1_5
    const char* strp = (const char*)inStr;
#else
    char* strp = (char*)inStr;
#endif
    char* p = outBuf;
    size_t cn = iconv(m_iconv_desc, &strp, &inStrLen,  &p, &outBufSize);
    if (cn == (size_t)-1)
        return (hyu32)-1;      // error

    *p = '\0';
    p[1] = '\0';
    return p - outBuf;
}


static int m_hex1(wchar_t c)
{
    if (c >= L'0' && c <= L'9')
        return c - L'0';
    if (c >= L'A' && c <= L'F')
        return c - L'A' + 10;
    if (c >= L'a' && c <= L'f')
        return c - L'a' + 10;
    return -1;
}

// inStrのエスケープ文字 \n \r \x?? \? を展開した文字列をoutBufに格納する。
// 扱う文字コードはfromcode。
// bEolCareがtrueなら末尾のCRコードは削除し、末尾にLFコードを付加する。
// 最終的に末尾に \0 を付加する。(戻り値では\0をカウントしない)
// 戻り値: 展開後文字列長。 -1=読めない文字、-2=outBufオーバーフロー
hys32 MBCSConv::unescape(const char* inStr, hyu32 inStrLen, char* outBuf, hyu32 outBufSize, bool bEolCare)
{
    const char* inStrEnd = inStr + inStrLen;
    char* outBufEnd = outBuf + outBufSize;
    char* outBufTop = outBuf;

    while (inStr < inStrEnd) {
        if (outBuf >= outBufEnd)
            return -2;

        const char* pi = inStr;
        hyu32 len;
        wchar_t wc = getCharAt(&inStr, &len);
        if (len == 0)
            // 読めない文字コード
            return -1;
        if (wc == L'\\' && inStr < inStrEnd) {
            const char* pi2 = inStr;
            wc = getCharAt(&inStr, &len);
            if (len == 0)
                wc = (wchar_t) *inStr++;
            if (wc == L'x' || wc == L'X') {
                if (inStr < inStrEnd) {
                    wc = getCharAt(&inStr, &len);
                    if (len == 0)
                        return -1;
                    int x1 = m_hex1(wc);
                    if (x1 >= 0 && inStr < inStrEnd) {
                        wc = getCharAt(&inStr, &len);
                        if (len == 0)
                            return -1;
                        int x2 = m_hex1(wc);
                        if (x2 >= 0) {
                            *outBuf++ = x1 * 16 + x2;
                            continue;
                        }
                    }
                }
                // 16進2桁取得できず
                len = inStr - pi;
                if (outBuf + len >= outBufEnd)
                    return -2;
                memcpy(outBuf, pi, len);
                outBuf += len;
            } else if (wc == L'n') {
                *outBuf++ = '\n';
            } else if (wc == L'r') {
                *outBuf++ = '\r';
            } else {
                len = inStr - pi2;
                if (outBuf + len <= outBufEnd) {
                    memcpy(outBuf, pi2, len);
                    outBuf += len;
                } else
                    return -2;
            }
        } else if (bEolCare && wc == L'\r' && inStr >= inStrEnd) {
            // bEolCareがtrueなら文字列最後の \r は無視する
        } else {
            len = inStr - pi;
            if (outBuf + len <= outBufEnd) {
                memcpy(outBuf, pi, len);
                outBuf += len;
            } else
                return -2;
        }
    }        

    if (bEolCare) {
        if (outBuf < outBufEnd)
            *outBuf++ = '\n';
        else
            return -2;
    }
    if (outBuf < outBufEnd) {
        *outBuf = '\0';
        return outBuf - outBufTop;
    }
    return -2;
}
