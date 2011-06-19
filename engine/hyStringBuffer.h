/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYSTRINGBUFFER_H_
#define m_HYSTRINGBUFFER_H_

#include "hyValue.h"
#include "hyObject.h"
#include <stdarg.h>

/*
  文字列バッファクラス

  文字列の終端には必ず '\0' が入る。
  バッファサイズは '\0' を含むサイズ。

  this
  +--------------+
  | hyu16 m_bufSize
  +--------------+
  | hyu16 m_length
  +--------------+
  | char* m_buffer
  +--------------+

  m_buffer
  +------------+
  |            |
  |  buffer    |
  |            |
  |          \0|
  +------------+
 */

namespace Hayat {
    namespace Engine {
        class Thread;

        // 型がStringBufferであるObjectを操作するクラス
        class StringBuffer {
        protected:
            hyu16     m_bufSize;
            hyu16     m_length;
            char*   m_buffer;


        public:
            static const hyu16 DEFAULT_BUFSIZE = 64;

            static void* operator new(size_t);
            static void operator delete(void*);

            StringBuffer(hyu16 bufSize = DEFAULT_BUFSIZE) { initialize(bufSize); }
            ~StringBuffer() { finalize(); }

            void    initialize(hyu16 bufSize = DEFAULT_BUFSIZE);
            void    finalize(void);


            /// 対応Objectを返す
            Object* getObj(void) { return Object::fromCppObj(this); }

            /// バッファをクリア
            void    clear(void);
            /// 文字列先頭アドレス
            char*   top(void) { return m_buffer; }
            /// バッファサイズ ('\0'を含むサイズ)
            hyu16     bufSize(void) { return m_bufSize; }
            /// 文字列長
            hyu16     length(void) { return m_length; }

            /// バッファ拡張無しで追加可能な文字数
            hyu16     remain(void) { return m_bufSize - m_length - 1; }
            /// lenバイトの文字列を追加できるように、必要ならバッファを拡張
            void    reserve(hyu16 len);
            /// 文字列を追加：バッファが不足していたら拡張する
            void    concat(const char* str, int len = -1);
            /// 値をconcatToStringBufferメソッドで文字列化して、末尾に追加する。
            void    concat(Context* context, Value d, int mode = 1);
            /// バイナリデータを追加：中に \0 が含まれていても良い
            /// バッファが不足していたら拡張する
            void    addBinary(const void* bin, hyu32 size);
            /// printfの結果を末尾に追加する。
            void    sprintf(const char* fmt, ...);
            void    vsprintf(const char* fmt, va_list);

            /// 文字列終端アドレス ('\0'の位置)
            char*   bottom(void) { return m_buffer + m_length; }
            /// 後ろに文字列を追加したら、これを呼んで文字列長を再計算する
            /// 追加前の終端位置から、 \0 の位置をサーチする
            void    concatenated(void);
            /// バッファが操作されて、文字列長が短かくなった可能性がある場合には、
            /// これを呼んで文字列長を再計算する
            /// バッファ先頭位置から、 \0 の位置をサーチする
            void    calcLength(void);
            /// バイナリデータがこのアドレスまで読まれた：終端位置をここにセット
            void    binaryAdded(const void* endAddr);

            /// dを文字列化してfuncに渡し、文字列化したバッファを消して終了
            /// dがnilだったら空文字列をfuncに渡す。
            static void sendStr(Context* context, Value d, void(*func)(const char*));
            static void sendStr(Context* context, Value d, void(*func)(const char*,int));
            /// d1とd2を文字列比較する。
            /// d1とd2の型はStringとStringBufferのみ受け付け、strcmpと同じ値を
            /// 返す。そうでなければ、 StringBuffer::COMPARE_ERROR を返す。
            static hys32 compare(Value d1, Value d2);
            static const hys32 COMPARE_ERROR = -99999999;
            // 文字列からハッシュコードを計算
            static hyu32 calcHashCode(const char*);
        };

    }
}
#endif /* m_HYSTRINGBUFFER_H_ */
