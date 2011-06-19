/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYFIBER_H_
#define m_HYFIBER_H_

#include "hyValue.h"

/*
  Fiberクラス

  yield文で値を生成する。


  generate()
    yieldによって作られた値を取得する。まだ作られていなければ
    yieldするまで実行する。
    returnが実行されると、Exception(:fiber_return)を発生させる。


  defFiber times (n) {
    x = 0
    for (i = 0; i < n; i += 1) {
      x += yield i
    }
    return x
  }

  gen = times(3)    // n=3 で Fiberインスタンスが生成される
  gen.generate()    // => 0  // yield i まで実行、0 を取得
  gen.yieldResult(10) // yield文の戻り値として 10 を与える
  gen.generate()    // => 1  // yield i まで実行、1 を取得
  gen.yieldResult(20) // yield文の戻り値として 20 を与える
  gen.generate()    // => 2  // yield i まで実行、2 を取得
  gen.yieldResult(30) // yield文の戻り値として 30 を与える
  gen.generate()    // => return x まで実行、Exceptionが投げられる val()は 60



  go()
    yield または return 文までを実行する。
    yield を実行した場合は true を、 return を実行した場合には false を返す。
    yieldによって作られた値は getYieldVal() で取得できる。
    returnによって返された値は getReturnVal() で取得できる。
*/

namespace Hayat {
    namespace Engine {

        class Exception;
        class Closure;

        class Fiber {
#ifdef HMD_DEBUG
            friend class Context;   // for Context::debugStackTrace()
#endif

        protected:
            Context*        m_fiberContext; // yieldを実行しているContext
            Context*        m_callerContext; // go()またはgenerate()を呼び出したContext

            enum State_e {
                YIELDING,           // yieldが実行された直後
                YIELD_JOIN,         // yield戻り値待ち
                RUNNING,            // 通常部分を実行中
                RETURNED,           // returnが実行された直後
                FINISHED            // 終了状態
            } m_state;


            static const hyu8 MODE_NONE = 0;
            static const hyu8 MODE_GO = 1; // go()実行
            static const hyu8 MODE_GENERATE = 2; // generate()実行
            static const hyu8 MODE_ITERATE = 3; // iterate()実行
            hyu8            m_mode;
            Closure*        m_iterating_closure; // iterate中のclosure

        public:

            static void* operator new(size_t);
            static void operator delete(void*);
        
            Fiber(Context* context) { initialize(context); }
            ~Fiber() {}

            // context上で実行を開始したばかりのメソッドをFiberに変化させる
            void initialize(Context* context);

            // context上でclosureを呼び出してそれをFiberに変化させる
            void initialize(Context* context, Closure* closure, hyu8 numArgs);

            void yielding(void); // m_fiberContext上でyieldを実行
            void fiberReturn(void);         // fiberからのreturnを実行
            void fiberThrow(Exception* e);  // fiberから例外で抜ける
            void go(Context* context);     // yieldまたはreturnまで実行
            void getYieldVal(Context* context); // yield値を取得
            void getReturnVal(Context* context); // return値を取得
            void yieldResult(Context* context, hyu8 numArgs); // yieldの戻り値を与える
            void generate(Context* context); // yield値をcontextに持ってくる

            // 繰り返し処理
            void iterate(Context* context, Closure* closure);
            void iterate_affi(Context* context);
            // 繰り返し中での例外の処理
            bool iterate_exception_catcher(Context* context, Exception* e);

            // yield中(m_status==YIELD_JOIN)ならyieldに対して例外を投げ、
            // そうでなければ普通に例外を投げる
            void thrown(Context* context, Exception* e);
            // 実行コンテキストをm_fiberContextに変更
            void switchToFiberContext(void);
            // closure上で例外が発生したらそれをthrown()に渡すようにした状態で
            // context上でclosureを呼び出す
            void stakeCall(Context* context, Closure* closure, hyu8 numArgs);


            bool isFinished(void) { return m_state >= FINISHED; }

        public:
            void m_GC_mark(void);

        protected:
            static void m_contextSwitch(Context* prevContext, Context* nextContext);

        };

    }
}

#endif /* m_HYFIBER_H_ */
