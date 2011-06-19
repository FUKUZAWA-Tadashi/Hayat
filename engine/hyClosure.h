/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCLOSURE_H_
#define m_HYCLOSURE_H_

#include "hyCodePtr.h"
#include "hyObject.h"

/*
 クロージャクラス

  Object としてメモリ管理されている事が前提。それ以外の使い方をすると
  メモリ破壊が起きるのでしてはいけない。

  +----------------
  | MethodPtr m_methodPtr
  +----------------
  | hyu8 m_numSharedLocal
  +----------------
  | hyu8 m_numSharedSet
  +----------------
  | hyu16 m_creatorMethodID
  +----------------
  | SharedLocal_st m_shareInfo[0]
  +----------------
  | SharedLocal_st m_shareInfo[1]
  +----------------
  | ...
*/

namespace Hayat {
    namespace Engine {

        class Closure {
            friend class HSca_Closure;

        protected:
            struct SharedLocal_st {
                hyu8          localIdx;
                Object*     sharedLocalVar;
            };

            MethodPtr m_methodPtr;
            hyu8      m_numSharedLocal;
            hyu8      m_numSharedSet;
            hyu16         m_creatorMethodID;
            SharedLocal_st  m_shareInfo[1];  // m_shareInfo[m_numSharedLocal]
        

        private:
            // new, delete は使用禁止。 create() を使用する事。
            static void* operator new(size_t);
            static void operator delete(void*);

        public:
            Closure(void) {}

            static Closure* create(hyu8 numSharedLocal);
            Object* getObj(void) { return Object::fromCppObj(this); }
            static size_t needSize(hys32 numSharedLocal);

            void initialize(const MethodPtr& methodPtr,
                            hyu8 numSharedlocal,
                            hyu16 creatorMethodID);
            void setSharedLocal(hyu8 localIdx, Object* sharedLocalVar);

            // クロージャ呼び出し
            // bAway が true の時、クロージャ中の return 文は
            // クロージャを生成したルーチンからのリターン動作となる
            void call(Context* context, hyu8 numArgs, bool bAway = false);


        public:
            void m_GC_mark(void);


        };
    
    }
}

#endif /* m_HYCLOSURE_H_ */
