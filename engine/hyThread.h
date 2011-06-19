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
            friend class HClass;            // �f�X�g���N�^�Ăяo��
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
            // ������ ThreadManager::createThread() ���g�����B
            void* operator new(size_t) { HMD_HALT(); return (void*)3; }
            // �j���� terminate() ���Ăׂ΁A��� GC ���s�Ȃ��B
            void operator delete(void*) { HMD_HALT(); }

        public:
            Thread(ThreadManager* pManager = &gThreadManager);
            ~Thread();


            // ��b�������F��������s������Ԃł͂܂��������s���œ���s��
            void    beforeInit(void);

            // ������
            void    initialize(Context* context);

            // �I������
            void    finalize(void);

            Context* context(void) { return m_context; }
            void    detachContext(Context* context) { if (m_context == context) m_context = NULL; }
            void    switchContext(Context* context) {
                //HMD__PRINTF_FK("Thread(%x)::switchContext(%x)\n",this,context);
                m_context = context; }

            State_e state(void) const { return m_state; }
            bool    isActive(void) const { return (m_state != BEFORE_INIT) && (m_state != INITIALIZED) && (m_state != TERMINATED); }
            bool    isRunning(void) const { return (m_state == RUNNING); }

            void    start(void);    // ��~��Ԃ̃X���b�h���X�^�[�g������
            void    waitTicks(hys32 ticks);
            void    waitJoin(const Thread* joinThread);
            void    sleep(void);    // �x�~������
            // �x�~��Ԃ��I���A���s���ĊJ�Btrue��n���ƁARUNNING��Ԃ̎��ɌĂ�ł�assert�ŗ����Ȃ��Ȃ�B
            void    wakeup(bool allowRunning = true);
            void    terminate(void);        // �����I��

            // instance �� run() ���\�b�h����X���b�h�X�^�[�g
            void    startRun(Value instance);
            // �X���b�h������F���s���̃R�[�h���̂Ăčŏ�����C���X�^���X���\�b�h�����s���Ȃ���
            // ���\�b�h�̈����͖���
            void    takeover(Value instance, SymbolID_t methodSymbol);

            // waitTick����܂Ŏ��s����
            void    exec1tick(void);

#ifdef HMD_DEBUG
            void    printStackTrace(void);
#else
            void    printStackTrace(void) {}
#endif
            ThreadManager*  threadManager(void) { return m_pThreadManager; }

        protected:

            // �w��X�^�b�N�t���[���ȏ�͈̔͂ŁA���s
            void    m_exec(hyu32 frameSp);

            // ���\�b�h�Ăяo�������s
            // �Ăяo���ɐ���������true��Ԃ�
            // m_state��ύX���Ȃ�
            bool    m_methodExec(SymbolID_t methodSymbol, hyu8 numArgs);


        protected:
            // �o�^���Ă���X���b�h�}�l�[�W��
            ThreadManager*  m_pThreadManager;

            // ���s�R���e�L�X�g
            Context*        m_context;

            // �X���b�h�̏��
            State_e         m_state;

            // WAITING_TICK �̎��̎c�҂�����
            hys32             m_waitingCount;
            // WAITING_JOIN ���鎞�̏I����҂ΏۃX���b�h
            const Thread*   m_joinThread;

        public:
            void    m_GC_mark(void);

        };

    }
}
#endif /* m_HYTHREAD_H_ */
