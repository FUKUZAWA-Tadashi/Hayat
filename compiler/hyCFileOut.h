/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYFILEOUT_H_
#define m_HYFILEOUT_H_

#include <stdio.h>

namespace Hayat {
    namespace Compiler {

        void writeGenHead(FILE* fp);

        // ファイルに書いた内容が、元ファイルと同じ内容だった場合には
        // 更新せずにタイムスタンプを変更しないモード付きのファイルライタ。
        class DiffWrite {
        public:
            typedef enum {
                DW_DEFAULT,     // クラスのデフォルト設定を使う
                DW_OVERWRITE,   // 内容を比較せずに必ず上書き更新する
                DW_DIFFWRITE    // 内容が同じなら更新しない
            } DwMode_e;

            // クラスのデフォルト設定をセット
            static void setDefaultMode(DwMode_e);

        protected:
            static DwMode_e m_defaultMode;

        public:
            DiffWrite(void);
            DiffWrite(const char* path, DwMode_e mode = DW_DEFAULT);
            ~DiffWrite();

            FILE* open(const char* path, DwMode_e mode = DW_DEFAULT);
            void close(void);
            FILE* fp(void) { return m_fp; }

        protected:
            FILE*       m_fp;
            DwMode_e    m_mode;
            const char* m_orgPath;
            char*       m_tmpPath;
        };
    }
}

#endif /* m_HYFILEOUT_H_ */
