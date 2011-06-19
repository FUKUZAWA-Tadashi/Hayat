/*  -*- coding: sjis-dos -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef h_HYSTRINGREADER_H_
#define h_HYSTRINGREADER_H_

#include "hyObject.h"


namespace Hayat {
    namespace Engine {
        
        class StringBuffer;

        class StringReader {

        protected:
            // m_sbObj もしくは m_str のどちらかを使用
            Object* m_sbObj;  // StringBufferのObject
            const char* m_str;  // String
            hys32 m_strlen; // m_strの文字列長

            hys32 m_pos;    // 読み込み位置

            const char* m_addr(hys32 pos);

        public:
            StringReader(void);
            void initialize(Object* sbObj);
            void initialize(const char* str);

            // 現在位置を返す。
            hys32   getPos(void) { return m_pos; }
            // 現在位置を変更する。posが有効ならposを返す。無効なら-1
            hys32   setPos(hys32 pos);
            // 全体サイズを返す。終端位置と同じ。
            hys32   length(void);
            // 1バイト取得。文字列がもう無ければ-1を返す。
            hys32   getc(void);
            // delimに含まれる文字を終端とする文字列を取得。
            // 文字列がもう無ければNULLを返す。
            StringBuffer*   gets(const char* delim = "\n");
            // searchCharsに含まれる文字を発見したら、そこに pos を移動して
            // true を返す。発見できず文字列が終了したら false を返す。
            bool    search(const char* searchChars);
            // skipCharsに含まれる文字である間、posを進める。
            hys32   skip(const char* skipChars);
            // chである間、posを進める。
            hys32   skip(hys32 ch) { return skip(ch, ch); }
            // c1 <= c <= c2 である間、posを進める。
            hys32   skip(hys32 c1, hys32 c2);
            // start <= p < end の範囲の文字列を取得
            StringBuffer*   substr(hys32 start, hys32 end);
            // 現在位置からlenバイト分の文字列を取得
            StringBuffer*   read(hys32 len);

            // 整数を読む。読めなかったらposは移動しない
            hys32   getInt(void);
            // 浮動小数を読む。読めなかったらposは移動しない
            hyf32   getFloat(void);

            void m_GC_mark(void);
        };

    }
}

#endif /* h_HYSTRINGREADER_H_ */
