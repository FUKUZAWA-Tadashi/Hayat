/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYMETHOD_H_
#define m_HYMETHOD_H_

#include "hyValue.h"


/*
  Methodクラス

  オブジェクトと、メソッド名のペアを保持する。

  call() によって、オーバーロード、オーバーライドを考慮した適切な
  メソッドを呼び出す。
*/


namespace Hayat {
    namespace Engine {

        class Method {
        protected:
            Value       m_self;
            SymbolID_t  m_methodSym;

        public:
            Method(Value& self, SymbolID_t methodSym);
            ~Method() {}

            void initialize(Value& self, SymbolID_t methodSym);

            void call(Context* context, hyu8 numArgs);

        public:
            void m_GC_mark(void);

        };

    }
}


#endif /* m_HYMETHOD_H_ */
