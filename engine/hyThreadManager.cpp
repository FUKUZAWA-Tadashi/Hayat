/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyThreadManager.h"
#include "hyThread.h"
#include "hyMemPool.h"
#include "hyObject.h"
#include "hyContext.h"
#include "hyDebug.h"
#include <string.h>


using namespace Hayat::Engine;


#ifdef HMD_DEBUG
int         ThreadManager::m_firstOfAllCalled = 0;
#endif
int              ThreadManager::m_numManagers = 0;
ThreadManager*   ThreadManager::m_managers[ThreadManager::MAX_MANAGERS];



HMD_DLL_EXPORT ThreadManager       Hayat::Engine::gThreadManager;


void ThreadManager::firstOfAll(void)
{
#ifdef HMD_DEBUG
    m_firstOfAllCalled = 0x0123feda;
#endif
    m_numManagers = 0;
}


ThreadManager::ThreadManager(void)
{
    m_tbl = (tbl_st*)NULL;
    m_tblSize = 0;
}

void ThreadManager::initialize(void)
{
    HMD_DEBUG_ASSERTMSG(m_firstOfAllCalled == 0x0123feda,
                        M_M("PROGRAM BUG :  ThreadManager::firstOfAll() must called first"));
    m_tbl = gMemPool->allocT<tbl_st>(INIT_TBLSIZE, "THRT");
    m_tblSize = INIT_TBLSIZE;
    m_lastId = 0;
    int i;
    for (i = 0; i < m_tblSize; i++) {
        m_tbl[i].threadObj = NULL;
        m_tbl[i].flag = FLAG_NONE;
        m_tbl[i].threadID = INVALID_THREAD_ID;
    }
    for (i = 0; i < m_numManagers; i++) {
        if (m_managers[i] == this)
            return;
    }
    HMD_ASSERT(m_numManagers < MAX_MANAGERS);
    if (m_numManagers < MAX_MANAGERS)
        m_managers[m_numManagers++] = this;
}
    

ThreadID_t ThreadManager::createThread(void)
{
    Object* obj = Object::create(HC_Thread, sizeof(Thread) + sizeof(hyu32));
    Thread* pThread = obj->cppObj<Thread>();
    pThread->m_pThreadManager = this;
    pThread->beforeInit();

    //HMD_PRINTF("Thread %x create\n",pThread);

    return addThreadObj(obj);
}
    
ThreadID_t ThreadManager::addThreadObj(Object* obj)
{
    HMD_DEBUG_ASSERT(obj->type()->symCheck(HSym_Thread));
    int i;
    for (i = 0; i < m_tblSize; i++) {
        if (m_tbl[i].threadObj == NULL)
            break;
    }
    if (i >= m_tblSize) {
        m_resize(m_tblSize + INIT_TBLSIZE);
        HMD_ASSERT(i < m_tblSize);
    }        

    m_nextId();
    m_tbl[i].threadObj = obj;
    m_tbl[i].flag = FLAG_ADDED;
    m_tbl[i].threadID = m_lastId;;
    return m_lastId;
}

void ThreadManager::m_nextId(void)
{
    int i;
    for (;;) {
        if (++m_lastId < 0)
            m_lastId = 1;
        for (i = 0; i < m_tblSize; i++) {
            if (m_tbl[i].threadObj == NULL)
                continue;
            if (m_tbl[i].threadID == m_lastId)
                break;  // IDがかぶった
        }
        if (i >= m_tblSize)
            return;
    }
}

void ThreadManager::m_resize(int newTblSize)
{
    HMD_ASSERT(newTblSize > m_tblSize);
    //HMD__PRINTF_FK("ThreadManager::m_resize  %d -> %d\n", m_tblSize, newTblSize);
    tbl_st* newTbl = gMemPool->allocT<tbl_st>(newTblSize, "THRT");
    memcpy(newTbl, m_tbl, m_tblSize * sizeof(tbl_st));
    for (int i = m_tblSize; i < newTblSize; ++i) {
        newTbl[i].threadObj = NULL;
        newTbl[i].flag = FLAG_NONE;
        newTbl[i].threadID = INVALID_THREAD_ID;
    }
    gMemPool->free(m_tbl);
    m_tbl = newTbl;
    m_tblSize = newTblSize;
}

void ThreadManager::m_removeThread(Thread* pThread)
{
    int i;
    for (i = 0; i < m_tblSize; i++) {
        if ((m_tbl[i].threadObj != NULL) && (m_tbl[i].threadObj->cppObj<Thread>() == pThread)) {
            m_tbl[i].threadObj = NULL;
            m_tbl[i].flag = FLAG_REMOVED;
            m_tbl[i].threadID = INVALID_THREAD_ID;
            return;
        }
    }
    HMD_ERRPRINTF("hy::Thread %x is not in manager %x\n", pThread, this);
    HMD_ERRPRINTF("Thread state = %d\n", pThread->state());
}
    
Thread* ThreadManager::id2thread(ThreadID_t tid)
{
    if (tid == INVALID_THREAD_ID)
        return NULL;
    int i;
    for (i = 0; i < m_tblSize; i++) {
        if (m_tbl[i].threadID == tid) {
            return m_tbl[i].threadObj->cppObj<Thread>();
        }
    }
    return NULL;
}
    
ThreadID_t ThreadManager::thread2id(Thread* pThread)
{
    if ( pThread == NULL ) return INVALID_THREAD_ID;
    int i;
    for (i = 0; i < m_tblSize; i++) {
        if (m_tbl[i].threadObj == NULL) continue;
        
        if (m_tbl[i].threadObj->cppObj<Thread>() == pThread) {
            return m_tbl[i].threadID;
        }
    }
    return INVALID_THREAD_ID;
}
    

Thread* ThreadManager::createTmpThread(void)
{
    return id2thread(createThread());
}
    
void ThreadManager::destroyTmpThread(Thread* pThread)
{
    HMD_DEBUG_ASSERT(pThread != NULL);
    m_removeThread(pThread);
    pThread->finalize();
    Object::fromCppObj(pThread)->destroy();
}



void ThreadManager::exec1tick(void)
{
    int i;
    for (i = 0; i < m_tblSize; i++) {
        m_tbl[i].flag = FLAG_NONE;
    }
    // 頭から実行
    for (i = 0; i < m_tblSize; i++) {
        if (m_tbl[i].threadObj != NULL) {
            if (m_tbl[i].flag == FLAG_NONE) {
                m_tbl[i].flag = FLAG_RUNNING;
                m_tbl[i].threadObj->cppObj<Thread>()->exec1tick();
                m_tbl[i].flag = FLAG_END;
            }
        }
    }
}
    
bool ThreadManager::isThreadRunning(void)
{
    int i;
    for (i = 0; i < m_tblSize; i++) {
        if (m_tbl[i].threadObj != NULL) {
            Thread::State_e state = m_tbl[i].threadObj->cppObj<Thread>()->state();
            if ((state != Thread::BEFORE_INIT) && (state != Thread::TERMINATED))
                return true;
        }
    }
    return false;
}

void ThreadManager::finalize(void)
{
    int i;
    for (i = 0; i < m_tblSize; i++) {
        if (m_tbl[i].threadObj != NULL) {
            m_tbl[i].threadObj->cppObj<Thread>()->finalize();
            m_tbl[i].threadObj = NULL;
            m_tbl[i].threadID = INVALID_THREAD_ID;
        }
    }
    gMemPool->free(m_tbl);
    m_tbl = NULL;
    m_tblSize = 0;
}

void ThreadManager::m_GC_mark(void)
{
    Debug::incMarkTreeLevel();
    for (int i = 0; i < m_numManagers; i++) {
        for (int j = 0; j < m_managers[i]->m_tblSize; j++) {
            Object* obj = m_managers[i]->m_tbl[j].threadObj;
            if ((obj != NULL) && (obj->cppObj<Thread>()->state() != Thread::TERMINATED)){
                //HMD_PRINTF("mark thread obj %x\n",obj);
                obj->m_unmark();
                GC::markObj(obj);
            }
        }
    }
    Debug::decMarkTreeLevel();
}


void ThreadManager::startMarkIncremental(void)
{
    GC::m_arrIdx = m_numManagers - 1;
    if (GC::m_arrIdx < 0) {
        GC::m_arrIdx_2 = -1;
    } else {
        GC::m_arrIdx_2 = m_managers[GC::m_arrIdx]->m_tblSize - 1;
    }
    Debug::incMarkTreeLevel();
}

void ThreadManager::markIncremental(void)
{
    bool done = false;
    for (;;) {
        for ( ; GC::m_arrIdx_2 >= 0; --GC::m_arrIdx_2) {
            if (done)
                return;
            Object* obj = m_managers[GC::m_arrIdx]->m_tbl[GC::m_arrIdx_2].threadObj;
            if ((obj != NULL) && (obj->cppObj<Thread>()->state() != Thread::TERMINATED)){
                //HMD_PRINTF("mark thread obj %x (idx %d)\n",obj,GC::m_arrIdx_2);
                obj->m_unmark();
                GC::markObj(obj);
                done = true;
            }
        }
        if (--GC::m_arrIdx >= 0) {
            GC::m_arrIdx_2 = m_managers[GC::m_arrIdx]->m_tblSize - 1;
        } else {
            break;
        }
    }
    GC::m_subPhase_end = true;
    Debug::decMarkTreeLevel();
}

void ThreadManager::markAllStack(void)
{
    for (int i = 0; i < m_numManagers; ++i) {
        ThreadManager* mgr = m_managers[i];
        int n = mgr->m_tblSize;
        for (int j = 0; j < n; ++j) {
            Object* obj = mgr->m_tbl[j].threadObj;
            if (obj == NULL)
                continue;
            Thread* th = obj->cppObj<Thread>();
            if (th->state() == Thread::TERMINATED)
                continue;

            th->context()->m_GC_mark_stack();
        }
    }
}


// terminatedなスレッドを削除
void ThreadManager::m_sweep(void)
{
    for (int i = 0; i < m_numManagers; i++) {
        for (int j = 0; j < m_managers[i]->m_tblSize; j++) {
            Object* obj = m_managers[i]->m_tbl[j].threadObj;
            if ((obj != NULL) && (! obj->isMarked())){
                Thread* pThread = obj->cppObj<Thread>();
                if (pThread->state() == Thread::TERMINATED) {
                    //HMD__PRINTF_FK("thread %x sweeped\n",pThread);
                    m_managers[i]->m_tbl[j].threadObj = NULL;
                    m_managers[i]->m_tbl[j].flag = FLAG_REMOVED;
                    m_managers[i]->m_tbl[j].threadID = INVALID_THREAD_ID;
                    pThread->finalize();
                    obj->m_unmark();
                }
            }
        }
    }
}

#ifdef HMD_DEBUG
void ThreadManager::printAllStackTrace(void)
{
    for (int i = 0; i < m_tblSize; i++) {
        if (m_tbl[i].threadObj != NULL) {
            Thread* th = m_tbl[i].threadObj->cppObj<Thread>();
            HMD_PRINTF("---- thread %x id=%d ----\n", th, m_tbl[i].threadID);
            th->printStackTrace();
        }
    }
}
#endif
