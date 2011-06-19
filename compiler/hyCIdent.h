/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCIDENT_H_
#define m_HYCIDENT_H_

#include "machdep.h"

// Hayatで使う識別子を、C++で使える識別子に変換する
//
// name に [a-zA-Z0-9_] 以外の文字が含まれていない場合
//   → a_name
//
// name に [a-bzA-Z0-9_] 以外の文字が含まれている場合
//   末尾が ? で、他は [a-bzA-Z0-9_]
//   → q_name
//
//   末尾が ! で、他は [a-bzA-Z0-9_]
//   → e_name
//
//   末尾が = で、他は [a-bzA-Z0-9_]
//   → s_name
//
//   末尾が .hyb で、他は [a-bzA-Z0-9_]
//   → b_name
//
//   それ以外
//   → x_16進
//

namespace Hayat {
    namespace Compiler {

        class Ident {
        public:
            // nameをエンコードしてbufに入れる。入りきらなかったらfalseを返す
            static bool encode(const char* name, char* buf, hyu32 bufSize);

            // nameをエンコードして返す。先頭に head をつける。
            // 長すぎてエンコード失敗時にはNULLを返す。
            // 戻り値は gMemPool からallocしたバッファに入るので、
            // 不要となったらfreeしなければならない
            static char* encode(const char* name, const char* head = "", hyu32 extraMem = 0);

        };

    }
}

#endif /* m_HYCIDENT_H_ */
