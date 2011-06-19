/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Thread.h"

FFI_DEFINITION_START {

// インスタンス初期化
DEEP_FFI_FUNC(initialize)
{
    HMD_ASSERT(numArgs == 1);
    Object* obj = context->popObj(HSym_Thread);
    Thread* pThread = obj->cppObj<Thread>();
    Context* newContext = new Context();
    pThread->initialize(newContext);
    Value startInstance = context->pop();
    if (! startInstance.type->symCheck(HSym_NilClass))
        pThread->startRun(startInstance);
    ThreadManager* mgr = NULL;
    if (context->thread != NULL)
        mgr = context->thread->threadManager();
    if (mgr == NULL)
        mgr = &gThreadManager;
    mgr->addThreadObj(obj);
    context->pushObj(obj);
}

void FFI_FUNC(kill) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    selfVal.toCppObj<Thread>()->terminate();
}

bool FFI_FUNC_Q(running) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    Thread* self = selfVal.toCppObj<Thread>();
    return (self->state() != Thread::TERMINATED) && (self->state() != Thread::BEFORE_INIT);
}

DEEP_FFI_FUNC(join)
{
    HMD_ASSERT(numArgs == 0);
    Thread* self = context->popCppObj<Thread>(HSym_Thread);
    context->thread->waitJoin(self);
    context->pushNil();
}

// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
StringBuffer* FFI_FUNC(concatToStringBuffer) (Value selfVal, StringBuffer* sb, hys32 mode)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    Thread* self = selfVal.toCppObj<Thread>(HSym_Thread);
    const char* stat;
    switch (self->state()) {
    case Thread::BEFORE_INIT: stat = "BEFORE_INIT"; break;
    case Thread::RUNNING: stat = "RUNNING"; break;
    case Thread::WAITING_TICK: stat = "WAITING_TICK"; break;
    case Thread::SLEEP: stat = "SLEEP"; break;
    case Thread::TERMINATED: stat = "TERMINATED"; break;
    default:
        stat = "UNKNOWN";
        break;
    }
    sb->sprintf("(%x-%s)", self, stat);
    if (mode < 0)
        sb->concat(":<Thread>");

    return sb;
}

hys32 FFI_FUNC(hashCode) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return ((hyu32)selfVal.data)>>2;
}

FFI_GCMARK_FUNC()
{
    object->cppObj<Thread>(HSym_Thread)->m_GC_mark();
}

} FFI_DEFINITION_END
