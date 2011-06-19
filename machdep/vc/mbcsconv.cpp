/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include <windows.h>
#include <winnls.h>
#include "mbcsconv.h"
#include "hyEndian.h"

#define DEFAULT_CODEPAGE        65001


using namespace Hayat::Parser;
using namespace Hayat::Common;


hyu16 MBCSConv::m_inCodePage;
hyu16 MBCSConv::m_outCodePage;


static hyu16 cpscan(const char* codeStr)
{
    static struct m_CP_st {
        const char* a;
        hyu16 cp;
    } m_cpconv[] = {
        { "SJIS", 932 },
        { "EUCJP", 20932 },
        { "UTF-8", 65001 },
        { "CP932", 932 },
        { "CP20932", 20932 },
        { "CP65001", 65001 },
        { "UTF-16BE", 1201 },
        { "UTF-16LE", 1200 },
    };
    
    if (codeStr == NULL)
        return DEFAULT_CODEPAGE;

    hyu16 codePage = 0;

    size_t len = strlen(codeStr) + 1;
    for (int i = 0; i < sizeof(m_cpconv)/sizeof(m_cpconv[0]); ++i) {
        if (_strnicmp(m_cpconv[i].a, codeStr, len) == 0) {
            codePage = m_cpconv[i].cp;
            break;
        }
    }
    if (codePage == 0) {
        codePage = atoi(codeStr);
    }

    return codePage;
}


bool MBCSConv::initialize(const char* fromcode, const char* tocode)
{
    if (tocode == NULL) tocode = fromcode;
    m_inCodePage = cpscan(fromcode);
    m_outCodePage = cpscan(tocode);
    if (m_inCodePage == 0 || m_outCodePage == 0)
        return false;
    return true;
}

void MBCSConv::finalize(void)
{
}

wchar_t MBCSConv::getCharAt(const char** pInStr, hyu32* pLen)
{
    const char* next;
    size_t len;
    if (m_inCodePage == 65001) {
        // UTF-8
        hyu8 b = (hyu8)**pInStr;
        if (b >= 0xf0) {
            len = 4;
        } else if (b >= 0xe0) {
            len = 3;
        } else if (b >= 0xc0) {
            len = 2;
        //} else if (b >= 0x80) { 
            // 2文字目、3文字目
        } else {
            len = 1;
        }
        next = *pInStr + len;
    } else {
        next = CharNextExA(m_inCodePage, *pInStr, 0);
        len = next - *pInStr;
    }
    if (pLen != NULL)
        *pLen = len;
    if (len == 0) {
        return (wchar_t)*pInStr;
    }
    wchar_t wbuf[2];
    MultiByteToWideChar(m_inCodePage, 0, *pInStr, len, wbuf, 2);
    *pInStr = next;
    return wbuf[0];
}

hyu32 MBCSConv::conv(const char* inStr, hyu32 inStrLen, char* outBuf, hyu32 outBufSize)
{
    int len = MultiByteToWideChar(m_inCodePage, 0, inStr, inStrLen, NULL, 0);
    if (len == 0) {
        *outBuf = '\0';
        HMD_DEBUG_ASSERT(outBufSize >= 2);
        outBuf[1] = '\0';
        return 0;
    }
    wchar_t* wbuf = (wchar_t*) HMD_ALLOCA(sizeof(wchar_t) * (len+1));
    MultiByteToWideChar(m_inCodePage, 0, inStr, inStrLen, wbuf, len);    

    hyu32 mlen;

    if (m_outCodePage == 1200) {
        // UTF-16LE
        hyu8* p = (hyu8*)outBuf;
        for (int i = 0; i < len; ++i) {
            Endian::packE<Endian::LITTLE,hyu16>(p, wbuf[i]);
            p += 2;
        }
        mlen = ((char*)p) - outBuf;

    } else if (m_outCodePage == 1201) {
        // UTF-16BE
        hyu8* p = (hyu8*)outBuf;
        for (int i = 0; i < len; ++i) {
            Endian::packE<Endian::BIG,hyu16>(p, wbuf[i]);
            p += 2;
        }
        mlen = ((char*)p) - outBuf;

    } else {
        mlen = WideCharToMultiByte(m_outCodePage, 0, wbuf, len, outBuf, outBufSize, NULL, NULL);
    }

    if (mlen > outBufSize - 2)
        mlen = 0;
    else if (mlen > 0) {
        outBuf[mlen] = '\0';
        outBuf[mlen+1] = '\0';
    }

    return mlen;
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

bool MBCSConv::is_utf8_in(void)
{
    return m_inCodePage == 65001;
}
