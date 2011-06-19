/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_MBCSCONV_H_
#define m_MBCSCONV_H_

//
// 文字コード変換ルーチン for Visual C++
//


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

            // fromcodeがUTF8かどうか
            static bool is_utf8_in(void);

        protected:
            static hyu16  m_inCodePage;
            static hyu16  m_outCodePage;
        };
        
    }
}



#endif /* m_MBCSCONV_H_ */
