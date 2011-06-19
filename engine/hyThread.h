/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYTHREAD_H_
#define m_HYTHREAD_H_


#include "hyValue.h"


namespace Hayat {
    namespace Engine {
        class Object;
        class ThreadManager;
        class Context;
        HMD_EXTERN_DECL ThreadManager gThreadManager;

        typedef hys16     ThreadID_t;
        static const ThreadID_t INVALID_THREAD_ID = (ThreadID_t)-1;

        class Thread {
            friend class ThreadManager;
            friend class GC;
            friend class HSca_Thread;        // for GC
            friend class HClass;            // デストラクタ呼び出し
        public:
            static const hyu32 INITIAL_STACK_SIZE = 100;
            static const hyu32 MAX_SUBROUTINE_LEVEL = 40;

            typedef enum {
                BEFORE_INIT,
                INITIALIZED,
                RUNNING,
                WAITING_TICK,
                WAITING_JOIN,
                SLEEP,
                TERMINATED
            } State_e;

        private:
            // 生成は ThreadManager::createThread() を使う事。
            void* operator new(size_t) { HMD_HALT(); return (void*)3; }
            // 破棄は terminate() を呼べば、後は GC が行なう。
            void operator delete(void*) { HMD_HALT(); }

        public:
            Thread(ThreadManager* pManager = &gThreadManager);
            ~Thread();


            // 基礎初期化：これを実行した状態ではまだ初期化不足で動作不可
            void    beforeInit(void);

            // 初期化
            void    initialize(Context* context);

            // 終了処理
            void    finalize(void);

            Context* context(void) { return m_context; }
            void    detachContext(Context* context) { if (m_context == context) m_context = NULL; }
            void    switchContext(Context* context) {
                //HMD__PRINTF_FK("Thread(%x)::switchContext(%x)\n",this,context);
                m_context = context; }

            State_e state(void) const { return m_state; }
            bool    isActive(void) const { return (m_state != BEFORE_INIT) && (m_state != INITIALIZED) && (m_state != TERMINATED); }
            bool    isRunning(void) const { return (m_state == RUNNING); }

            void    start(void);    // 停止状態のスレッドをスタートさせる
            void    waitTicks(hys32 ticks);
            void    waitJoin(const Thread* joinThread);
            void    sleep(void);    // 休止させる
            // 休止状態を終了、実行を再開。trueを渡すと、RUNNING状態の時に呼んでもassertで落ちなくなる。
            void    wakeup(bool allowRunning = true);
            void    terminate(void);        // 強制終了

            // instance の run() メソッドからスレッドスタート
            void    startRun(Value instance);
            // スレッド乗っ取り：実行中のコードを捨てて最初からインスタンスメソッドを実行しなおす
            // メソッドの引数は無し
            void    takeover(Value instance, SymbolID_t methodSymbol);

            // waitTickするまで実行する
            void    exec1tick(void);

#ifdef HMD_DEBUG
            void    printStackTrace(void);
#else
            void    printStackTrace(void) {}
#endif
            ThreadManager*  threadManager(void) { return m_pThreadManager; }

        protected:

            // 指定スタックフレーム以上の範囲で、実行
            void    m_exec(hyu32 frameSp);

            // メソッド呼び出し即実行
            // 呼び出しに成功したらtrueを返す
            // m_stateを変更しない
            bool    m_methodExec(SymbolID_t methodSymbol, hyu8 numArgs);


        protected:
            // 登録しているスレッドマネージャ
            ThreadManager*  m_pThreadManager;

            // 実行コンテキスト
            Context*        m_context;

            // スレッドの状態
            State_e         m_state;

            // WAITING_TICK の時の残待ち時間
            hys32             m_waitingCount;
            // WAITING_JOIN する時の終了を待つ対象スレッド
            const Thread*   m_joinThread;

        public:
            void    m_GC_mark(void);

        };

    }
}
#endif /* m_HYTHREAD_H_ */
