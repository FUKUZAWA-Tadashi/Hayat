/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyThread.h"
#include "hyThreadManager.h"
#include "hyVM.h"
#include "hyBytecode.h"
#include "hyCodeManager.h"
#include "hyObject.h"
#include "hyMemPool.h"
#include "hyEndian.h"
#include "HSymbol.h"
#include "opcode_def.h"
#include <stdarg.h>
#include "hyStringBuffer.h"
#include <stdio.h>
#include <string.h>


#ifdef HMD_DEBUG
extern struct m_op_d_st { const char* name; const char* type; } m_HY_OPCODE_DEBUG_TABLE_[];
#endif


using namespace Hayat::Common;
using namespace Hayat::Engine;

Thread::Thread(ThreadManager* pManager)
{
    m_pThreadManager = pManager;
    beforeInit();
}

Thread::~Thread()
{
    //HMD_PRINTF("Thread %x destructor\n",this);
    if (m_pThreadManager != NULL) {
        m_pThreadManager->m_removeThread(this);
        finalize();
    }
}


void Thread::beforeInit(void) {
    m_state = BEFORE_INIT;
    m_context = NULL;
}

void Thread::initialize(Context* context)
{
    HMD_ASSERT(! isActive());
    m_context = context;
    HMD_ASSERT(context->thread == NULL || context->thread == this);
    context->thread = this;
    m_state = INITIALIZED;
    m_waitingCount = 0;
}

void Thread::finalize(void)
{
    //HMD_PRINTF("Thread %x finalize\n",this);
    m_state = BEFORE_INIT;
    if (m_context != NULL) {
        if (m_context->thread != NULL) {
            HMD_ASSERT(m_context->thread == this);
            m_context->thread = NULL;
        }
    }
    m_context = NULL;
}

// instance の run() メソッドからスレッドスタート
void Thread::startRun(Value instance)
{
    m_context->cleanup();
    m_context->push(instance);
    m_context->methodCall(HSym_run, 0);
    start();
}

// スレッド乗っ取り
void Thread::takeover(Value instance, SymbolID_t methodSym)
{
    m_context->cleanup();
    m_context->push(instance);
    m_context->methodCall(methodSym, 0);
    start();
}

void Thread::start(void)
{
    HMD_ASSERT(! isActive());
    m_state = RUNNING;
}

void Thread::terminate(void)
{
    m_state = TERMINATED;
}


void Thread::waitTicks(hys32 ticks)
{
    if (hmd_isNoWait())
        m_waitingCount = 0;
    else if (m_state == RUNNING) {
        m_state = WAITING_TICK;
        m_waitingCount = ticks;
    }
}

void Thread::waitJoin(const Thread* joinThread)
{
    HMD_ASSERT(m_state == RUNNING);
    m_state = WAITING_JOIN;
    m_joinThread = joinThread;
}

void Thread::sleep(void)
{
    HMD_ASSERT(m_state == RUNNING);
    m_state = SLEEP;
}
    
void Thread::wakeup(bool allowRunning)
{
    if ( allowRunning && m_state == TERMINATED )
        return; // スレッドが死んでいるので何もしない

    HMD_ASSERT((m_state == SLEEP) || (m_state == WAITING_TICK) || (allowRunning && (m_state == RUNNING)));
    m_state = RUNNING;
}

void Thread::exec1tick(void)
{
    VM::setContext(m_context);
    do {
        switch (m_state) {
                
        case WAITING_TICK:
            if (hmd_isNoWait()) {
                m_waitingCount = 0;
                m_state = RUNNING;
            } else if (--m_waitingCount <= 0)
                m_state = RUNNING;
            break;

        case RUNNING:
            if (! VM::haveCode())
                m_state = TERMINATED;
            else
                VM::exec1step();
            break;

        case WAITING_JOIN:
            if (m_joinThread->state() == TERMINATED)
                m_state = RUNNING;
            break;
                
        default:
            //case BEFORE_INIT:
            //case INITIALIZE:
            //case SLEEP:
            //case TERMINATED:
            break;
        }
    } while (m_state == RUNNING);
}



void Thread::m_GC_mark(void)
{
    //HMD_PRINTF("thread gc mark state=%d context=%x\n",m_state,m_context);
    if (m_state == TERMINATED)
        return;
    if (m_context != NULL) {
        Debug::incMarkTreeLevel();
        GC::markObj(m_context->getObj());
        Debug::decMarkTreeLevel();
    }
}



#ifdef HMD_DEBUG
void Thread::printStackTrace(void)
{
    static const char* stat_str[] = {
        "BEFORE_INIT",
        "INITIALIZED",
        "RUNNING",
        "WAITING_TICK",
        "WAITING_JOIN",
        "SLEEP",
        "TERMINATED"
    };
    HMD_PRINTF("state = %s\n", stat_str[m_state]);
    m_context->debugStackTrace();
    m_context->debugPrintStack();
}
#endif
