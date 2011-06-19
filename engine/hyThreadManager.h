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
                FLAG_RUNNING,       // exec1tick ���s��
                FLAG_END,           // exec1tick ���s��
                FLAG_REMOVED,       // �폜��
                FLAG_ADDED          // create���ꂽ�΂���
            };
        
            struct tbl_st {
                Object*     threadObj;
                ThreadID_t  threadID;
                hyu8          flag;
            };

            ThreadID_t      m_lastId;
            tbl_st*         m_tbl;           // �Ǘ��e�[�u��
            int             m_tblSize;       // �Ǘ��e�[�u���T�C�Y


        public:
            // ��ԍŏ��̏�����
            static void     firstOfAll(void);

            ThreadManager(void);
            void    initialize(void);
            void    finalize(void);

            // �X���b�h�쐬 �F ���s������ INVALID_THREAD_ID ��Ԃ�
            ThreadID_t createThread(void);
            // Thread�I�u�W�F�N�g�̓o�^ : createThread() �� HSca_Thread::HSfa_initialize() �p
            ThreadID_t addThreadObj(Object* obj);
            // �X���b�hID����AThread*���擾 : �擾�����|�C���^�́A����GC�܂ŗL��
            Thread* id2thread(ThreadID_t tid);
            ThreadID_t thread2id(Thread* pThread);
            void    exec1tick(void);
            bool    isThreadRunning(void);

            // �ꎞ�I�ȃX���b�h�𐶐��B
            // �g�p�� destroyTmpThread ���Ȃ���΂Ȃ�Ȃ�
            Thread* createTmpThread(void);
            // �ꎞ�I�ȃX���b�h��j���B
            void destroyTmpThread(Thread*);

#ifdef HMD_DEBUG
            void printAllStackTrace(void);
#else
            void printAllStackTrace(void) {}
#endif

        protected:
            // ���̃X���b�hID�������̂��̂Ƃ��Ԃ�Ȃ��悤�ɐ���: ���ʂ� m_lastId
            void        m_nextId(void);
            // �Ǘ��e�[�u���̃T�C�Y�𑝂₷
            void            m_resize(int newTblSize);
            // �X���b�h�Ǘ�����O��
            void            m_removeThread(Thread* pThread);
            // terminated�ȃX���b�h���폜
            static void     m_sweep(void);

        public:
            // �X���b�h�̎����Ă���Object���}�[�N
            static void     m_GC_mark(void);

            // GC��mark�t�F�[�Y�̃C���N�������^�����s
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
