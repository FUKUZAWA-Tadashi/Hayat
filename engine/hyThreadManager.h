/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYTHREADMANAGER_H_
#define m_HYTHREADMANAGER_H_

#include "hyThread.h"



class Test_hyThreadManager;

namespace Hayat {
    namespace Engine {

        class ThreadManager {
            friend class Thread;
            friend class GC;
            friend class ::Test_hyThreadManager;  // for unittest

        protected:
            static const int INIT_TBLSIZE = 4;
            enum {
                FLAG_NONE,
                FLAG_RUNNING,       // exec1tick 実行中
                FLAG_END,           // exec1tick 実行後
                FLAG_REMOVED,       // 削除後
                FLAG_ADDED          // createされたばかり
            };
        
            struct tbl_st {
                Object*     threadObj;
                ThreadID_t  threadID;
                hyu8          flag;
            };

            ThreadID_t      m_lastId;
            tbl_st*         m_tbl;           // 管理テーブル
            int             m_tblSize;       // 管理テーブルサイズ


        public:
            // 一番最初の初期化
            static void     firstOfAll(void);

            ThreadManager(void);
            void    initialize(void);
            void    finalize(void);

            // スレッド作成 ： 失敗したら INVALID_THREAD_ID を返す
            ThreadID_t createThread(void);
            // Threadオブジェクトの登録 : createThread() と HSca_Thread::HSfa_initialize() 用
            ThreadID_t addThreadObj(Object* obj);
            // スレッドIDから、Thread*を取得 : 取得したポインタは、次のGCまで有効
            Thread* id2thread(ThreadID_t tid);
            ThreadID_t thread2id(Thread* pThread);
            void    exec1tick(void);
            bool    isThreadRunning(void);

            // 一時的なスレッドを生成。
            // 使用後 destroyTmpThread しなければならない
            Thread* createTmpThread(void);
            // 一時的なスレッドを破棄。
            void destroyTmpThread(Thread*);

#ifdef HMD_DEBUG
            void printAllStackTrace(void);
#else
            void printAllStackTrace(void) {}
#endif

        protected:
            // 次のスレッドIDを現存のものとかぶらないように生成: 結果は m_lastId
            void        m_nextId(void);
            // 管理テーブルのサイズを増やす
            void            m_resize(int newTblSize);
            // スレッド管理から外す
            void            m_removeThread(Thread* pThread);
            // terminatedなスレッドを削除
            static void     m_sweep(void);

        public:
            // スレッドの持っているObjectをマーク
            static void     m_GC_mark(void);

            // GCのmarkフェーズのインクリメンタル実行
            static void     startMarkIncremental(void);
            static void     markIncremental(void);
            static void     markAllStack(void);


        protected:
            static const int        MAX_MANAGERS = 3;
            static int              m_numManagers;
            static ThreadManager*   m_managers[MAX_MANAGERS];
#ifdef HMD_DEBUG
            static int              m_firstOfAllCalled;
#endif
        };

        HMD_EXTERN_DECL ThreadManager        gThreadManager;
    }
}
#endif /* m_HYTHREADMANAGER_H_ */
