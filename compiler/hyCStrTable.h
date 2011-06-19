/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCSTRTABLE_H_
#define m_HYCSTRTABLE_H_

#include "machdep.h"
#include <stdio.h>
#include "hpSubstr.h"
#include "hyArray.h"

using namespace Hayat::Common;
using namespace Hayat::Parser;

class Test_hyCStrTable;

namespace Hayat {
    namespace Compiler {


        // 文字列テーブル or バイト列テーブル
        //
        // 重複チェックをしつつ、文字列を格納する。
        // またはバイト列を格納する。
        // 文字列とバイト列は同居できない。
        // 文字列の終端は '\0' でターミネートされている。
        // バイト列は中身に '\0' が含まれていても良く、終端はサイズで示す。
        class StrTable {
            friend class ::Test_hyCStrTable;    // unittest 

        protected:
            static const hyu32 m_INIT_STR_OFFS_SIZE = 256;
            static const hyu32 m_INIT_STR_BUF_SIZE = 5120;

        public:
            StrTable(void);
            ~StrTable();

            static const hyu32 NOT_FOUND = (hyu32)-1;
            static const hyu16 NOT_FOUND_ID = (hyu16)-1;


            void        initialize(hyu32 initStrOffsSize = m_INIT_STR_OFFS_SIZE, hyu32 initStrBufSize = m_INIT_STR_BUF_SIZE);
            void        finalize(void);

            // 重複チェックして(もし必要なら文字列を格納し)そのオフセットを返す
            hyu32         getOffs(const char* str, hyu32 len = 0);

            // 重複チェックせずに文字列を格納し、そのオフセットを返す
            hyu32         addStr(const char* str, hyu32 len = 0);

            // strと同じ文字列が既に格納されていたら、そのオフセットを返す
            // 格納されていなかったら、NOT_FOUND を返す
            hyu32         checkOffs(const char* str, hyu32 len = 0);

            // オフセットに対応する文字列を返す
            const char* getStr(hyu32 offset);
            

            // 重複チェックして(もし必要ならバイト列を格納し)その番号を返す
            hyu16         getIdBytes(const hyu8* bp, hyu32 len);
            // 重複チェックせずにバイト列を格納し、その番号を返す
            hyu16         addBytes(const hyu8* bp, hyu32 len);
            // bpと同じバイト列が既に格納されていたら、その番号を返す
            // 格納されていなかったら、NOT_FOUND_ID を返す
            hyu16         checkIdBytes(const hyu8* bp, hyu32 len);
            // 番号に対応するバイト列を返す
            const hyu8*   getBytes(hyu16 id, hyu32* pLen = NULL);

            // 文字列、バイト列の個数を返す
            hyu32         numStr(void) { return m_numStr; }


            // FILE* から文字列テーブルを読み込む
            void        readFile(FILE* fp);
            // 文字列テーブルを FILE* に出力
            // UTF16の文字列テーブルを出力した場合、readFileでは読めない
            void        writeFile(FILE* fp);

            // バイト列テーブルを出力
            void        writeBytes(TArray<hyu8>* out);
            // バイト列テーブルを読み込む
            void        readBytes(const hyu8** pinp);

#ifdef HMD_DEBUG
            void        dumpByteTable(void);
#endif

        protected:

            hyu32         m_numStr;
            hyu32*        m_strOffs;
            hyu32         m_strOffsSize;
            char*       m_strBuf;
            hyu32         m_strBufSize;

            //  文字列を追加する準備: m_strBufのサイズが不足するならrealloc
            void        m_reserve(hyu32 len);
            // m_strOffsに1つ追加する準備: m_strOffsのサイズが不足するならrealloc
            void        m_expandStrOffs(void);

        };

    }
}
#endif /* m_HYCSTRTABLE_H_ */
