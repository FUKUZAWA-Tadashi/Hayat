/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_MBCSCONV_H_
#define m_MBCSCONV_H_

//
// 文字列の文字コード変換ルーチン for unix
// iconvを使用して変換する
//


// in case of Japanese Shift-JIS, linux mbtowc() converts
// backslash '\' to 165, but we need 92 (L'\\') .
// define this if you want to change single byte 165 to 92.
#define AVOID_MBTOWC_SJIS_SANITIZE


#include <iconv.h>
#include "machdep.h"


namespace Hayat {
    namespace Parser {

        class MBCSConv {
        public:
            static bool initialize(const char* fromcode, const char* tocode = NULL);
            static void finalize(void);

            // *pInStrにあるマルチバイト文字を1文字読む。読んだバイト数を
            // *pLenに(NULLでなければ)格納する。 *pInStr を読んだバイト数分
            // 進ませる。読めなかった場合は *pLen に 0 を格納する。
            static wchar_t getCharAt(const char** pInStr, hyu32* pLen = NULL);

            // inStrのマルチバイト文字列(文字コードfromcode)をoutBufに
            // 文字コードtocodeに変換して格納する。変換後の文字列長を返す。
            // エラーの場合は(hyu32)-1を返す。
            static hyu32 conv(const char* inStr, hyu32 inStrLen, char* outBuf, hyu32 outBufSize);

            // inStrのエスケープ文字 \n \r \x?? \? を展開した文字列をoutBufに
            // 格納する。扱う文字コードはfromcode。
            // bEolCareがtrueなら末尾のCRコードは削除し、末尾にLFコードを付加する。
            // 最終的に末尾に \0 を付加する。(戻り値では\0をカウントしない)
            // 戻り値: 展開後文字列長。 -1=読めない文字、-2=outBufオーバーフロー
            static hys32 unescape(const char* inStr, hyu32 inStrLen, char* outBuf, hyu32 outBufSize, bool bEolCare = false);

        protected:
            static iconv_t  m_iconv_desc;
            static const char* m_prevLocale;
#ifdef AVOID_MBTOWC_SJIS_SANITIZE
            static bool m_sjis;
#endif
#ifdef __CYGWIN__
            // cygwinのmbtowc()が正しい値を返さないので、iconv()を使用する
            static iconv_t m_iconv_mbwc_desc;
            static size_t cygwin_mbtowc(wchar_t* pmc, const char* s, size_t n);
#endif
        };
        
    }
}


#endif /* m_MBCSCONV_H_ */
