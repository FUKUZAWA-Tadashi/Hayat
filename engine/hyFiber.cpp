/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyFiber.h"
#include "hyContext.h"
#include "hyVM.h"
#include "hyThread.h"
#include "hyException.h"
#include "hyClosure.h"
#include "hySymbolTable.h"
#include "hyDebug.h"

using namespace Hayat::Common;
using namespace Hayat::Engine;

void* Fiber::operator new(size_t size)
{
#ifndef HMD_DEBUG
    (void)size;
#endif
    HMD_DEBUG_ASSERT(size == sizeof(Fiber));
    Object* obj = Object::create(HC_Fiber, sizeof(Fiber) + sizeof(hyu32));
    return (void*) obj->cppObj<Fiber>();
}

void Fiber::operator delete(void* p)
{
    Object::fromCppObj(p)->destroy();
}



// context上で実行を開始したばかりのメソッドをFiberに変化させる
void Fiber::initialize(Context* context)
{
    m_callerContext = NULL;
    m_state = RUNNING;
    m_fiberContext = context->fork(0);
    m_fiberContext->fiber = this;
    m_mode = MODE_NONE;
    m_iterating_closure = NULL;
}

// context上でclosureを呼び出してそれをFiberに変化させる
void Fiber::initialize(Context* context, Closure* closure, hyu8 numArgs)
{
    closure->call(context, numArgs);
    initialize(context);
}

void Fiber::m_contextSwitch(Context* prevContext, Context* nextContext)
{
    if (prevContext->thread != NULL)
        prevContext->thread->switchContext(nextContext);
    nextContext->thread = prevContext->thread;
    VM::setContext(nextContext);
    GC::writeBarrier(Value::fromObj(prevContext->getObj()));
    //HMD__PRINTF_FK("Fiber::m_contextSwitch() %x -> %x\n",prevContext,nextContext);
}

// yieldまたはreturnまで実行
void Fiber::go(Context* context)
{
    if (context == m_fiberContext) {
        context->throwException(HSym_fiber_error, M_M("fiber loop"));
        return;
    }
    switch (m_state) {
    case YIELDING:
        context->push(TRUE_VALUE);
        break;

    case YIELD_JOIN:
        // yieldの戻り値に nil を与えて継続
        m_fiberContext->pushNil();
        m_state = RUNNING;
        // not break

    case RUNNING:
        m_mode = MODE_GO;
        m_callerContext = context;
        m_contextSwitch(m_callerContext, m_fiberContext);
        break;

    case RETURNED:
        context->push(FALSE_VALUE);
        break;

    case FINISHED:
    default:
        context->throwException(HSym_fiber_error, M_M("already finished"));
        break;
    }
}


void Fiber::generate(Context* context)
{
    // if (go()) {
    //   r =  getYieldVal()
    //   yieldResult(x)
    //   return r
    // } else {
    //   r = getReturnVal()
    //   throw :fiber_return, r
    // }				

    if (context == m_fiberContext) {
        context->throwException(HSym_fiber_error, M_M("generate() loop"));
        return;
    }
    switch (m_state) {
    case YIELDING:
        m_fiberContext->stack.cutPaste(context->stack, 1);
        m_state = YIELD_JOIN;
        return;

    case YIELD_JOIN:
        // yieldの戻り値に nil を与えて継続
        m_fiberContext->pushNil();
        m_state = RUNNING;
        // not break

    case RUNNING:
        m_mode = MODE_GENERATE;
        m_callerContext = context;
        m_contextSwitch(m_callerContext, m_fiberContext);
        return;

    case RETURNED:
        {
            Value retVal = m_fiberContext->stack.pop();
            context->throwException(HSym_fiber_return, retVal);
            m_fiberContext->thread = NULL;
            m_state = FINISHED;
        }
        return;

    default:
        context->throwException(HSym_fiber_error, M_M("fiber was finished"));
    }
}

void Fiber::yielding(void)
{
    HMD_ASSERT(m_state == RUNNING);
    //HMD__PRINTF_FK("YIELD : m_mode=%d\n", m_mode);
    switch (m_mode) {
    case MODE_GENERATE:
        m_fiberContext->stack.cutPaste(m_callerContext->stack, 1);
        m_state = YIELD_JOIN;
        break;
    case MODE_GO:
        m_callerContext->push(TRUE_VALUE);
        m_state = YIELDING;
        break;
    case MODE_ITERATE:
        m_state = YIELDING;
        iterate(m_callerContext, m_iterating_closure);
        break;
    default:
        HMD_FATAL_ERROR("unknown fiber mode %d", m_mode);
    }
    m_contextSwitch(m_fiberContext, m_callerContext);
    //HMD_PRINTF("YIELD : context sw %x to %x\n", m_fiberContext, m_callerContext);
}

void Fiber::fiberReturn(void)
{
    HMD_ASSERT(m_state == RUNNING);
    //HMD_PRINTF("fiberReturn : m_mode=%d\n", m_mode);
    m_contextSwitch(m_fiberContext, m_callerContext);
    switch (m_mode) {
    case MODE_GENERATE:
        {
            Value retVal = m_fiberContext->stack.pop();
            m_callerContext->throwException(HSym_fiber_return, retVal);
            m_fiberContext->thread = NULL;
            m_state = FINISHED;
        }
        break;
    case MODE_GO:
        m_callerContext->push(FALSE_VALUE);
        m_state = RETURNED;
        break;
    case MODE_ITERATE:
        m_fiberContext->stack.cutPaste(m_callerContext->stack, 1);
        m_fiberContext->thread = NULL;
        m_state = FINISHED;
#if 0
        HMD_PRINTF("caller %x stack trace :\n",m_callerContext);
        m_callerContext->debugStackTrace();
        HMD_PRINTF("fiber %x stack trace :\n", m_fiberContext);
        m_fiberContext->debugStackTrace();
        HMD_PRINTF(";\n");
#endif
        break;
    default:
        HMD_FATAL_ERROR("unknown fiber mode %d", m_mode);
    }
}

void Fiber::fiberThrow(Exception* e)
{
#if 0
    HMD_PRINTF("fiberThrow: m_fiberContext = %x, m_callerContext = %x\n", m_fiberContext, m_callerContext);
#endif
    m_contextSwitch(m_fiberContext, m_callerContext);
    m_fiberContext->thread = NULL;
    //m_mode = MODE_NONE;
    m_state = FINISHED;
    m_callerContext->throwException(e);
}

void Fiber::getYieldVal(Context* context)
{
    if (m_state != YIELDING) {
        context->throwException(HSym_fiber_error, M_M("getYieldVal() called but not yielding"));
        return;
    }
    m_fiberContext->stack.cutPaste(context->stack, 1);
    m_state = YIELD_JOIN;
}

void Fiber::getReturnVal(Context* context)
{
    if (m_state != RETURNED) {
        context->throwException(HSym_fiber_error, M_M("getReturnVal() called but not returned"));
        return;
    }
    m_fiberContext->stack.cutPaste(context->stack, 1);
    m_fiberContext->thread = NULL;
    m_state = FINISHED;
}


void Fiber::yieldResult(Context* context, hyu8 numArgs)
{
    if (m_state != YIELD_JOIN) {
        context->throwException(HSym_fiber_error, M_M("yieldResult() called but not yielded"));
        return;
    }
    context->stack.cutPaste(m_fiberContext->stack, numArgs);
    m_state = RUNNING;
}

void Fiber::thrown(Context* context, Exception* e)
{
    HMD_ASSERT(context == m_callerContext);
#if 0
    HMD_PRINTF("thrown context=");
    context->debugPrintCurPos();
    HMD_PRINTF(" fiber context=");
    m_fiberContext->debugPrintCurPos();
    HMD_PRINTF("\n ");
    e->debugPrint(context);
    HMD_PRINTF(" state=%d\n",m_state);
#endif

    if (m_state == YIELD_JOIN) {
        //HMD__PRINTF_FK("-- yield join\n");


        if (m_mode == MODE_ITERATE) {
            // iterate()中

            if (e->type() == HSymR_jump_control) {
                if (e->getLabel() == HSymR_next) {
                    // nextなら、stakeCallの戻り値をセットしてiterateの続き
                    m_fiberContext->push(e->val());
                    m_state = RUNNING;
                    iterate(context, m_iterating_closure);
                    return;
                }

                if (e->getLabel() == HSymR_break) {
                    // breakなら、iterate終了
                    context->push(e->val());
                    m_fiberContext->thread = NULL;
                    m_state = FINISHED;
                    context->ffiFinish();
                    return;
                }

                HMD_FATAL_ERROR("bad jumpControl label: %s", gSymbolTable.id2str(e->getLabel()));
            }

            if (e->type() == HSym_goto) {
                // gotoなら、iterate終了してその外側でgoto実行
                m_fiberContext->thread = NULL;
                m_state = FINISHED;
                context->push(e->val());
                context->ffiFinish();
                context->throwException(e);
                return;
            }
            
            // その他の例外

            // yieldに対して投げる
            m_state = RUNNING;
            m_fiberContext->throwException(e);

            // yield中で例外が捕捉されたなら、RUNNINGのまま。捕捉されずに
            // こぼれたら、Fiber::fiberThrowが呼ばれてFINISHEDになっている
            if (m_state == RUNNING) {
                // iterate() のループは go() の所から再開
                iterate(context, m_iterating_closure);
            }
            //HMD__PRINTF_FK("state after fiber throw = %d\n", m_state);
        }
        else {
            // iterate()中ではない

            if (e->type() == HSymR_jump_control) {
                if (e->getLabel() == HSymR_next) {
                    // nextなら、stakeCallの戻り値をセットして続き
                    context->push(e->val());
                    return;
                }
                if (e->getLabel() == HSymR_break) {
                    // breakなら、ループ終了
                    context->throwException(e);
                    return;
                }

                HMD_FATAL_ERROR("bad jumpControl label: %s", gSymbolTable.id2str(e->getLabel()));
            }

            if (e->type() == HSym_goto) {
                // gotoなら、ここでそのままgoto実行
                context->throwException(e);
                return;
            }

            // その他の例外

            // yieldに対して投げる
            m_state = RUNNING;
            m_fiberContext->throwException(e);

            // yield中で例外が捕捉されたなら、RUNNINGのまま。捕捉されずに
            // こぼれたら、Fiber::fiberThrowが呼ばれてFINISHEDになっている
            if (m_state == RUNNING) {
                // ここのループのyieldResultを呼ばず、
                // nextへジャンプさせる
                context->throwJumpCtlException(HSymR_next, e->val());
            }

        }

    } else {
        //HMD__PRINTF_FK("-- not join\n");
        context->throwException(e);
    }
}

void Fiber::switchToFiberContext(void)
{
    m_contextSwitch(m_callerContext, m_fiberContext);
}

void Fiber::stakeCall(Context* context, Closure* closure, hyu8 numArgs)
{
    closure->call(context, numArgs, true);
    context->exceptionToFiber(this);
}




namespace Hayat {
    namespace Engine {
        void i_iterate_affi(Context* context)
        {
            Fiber* fiber = (Fiber*) context->getAfterFfiParam();
            fiber->iterate_affi(context);
        }

        bool i_iterate_ex_catcher(Context* context, Exception* e)
        {
            Fiber* fiber = (Fiber*) context->getAfterFfiParam();
            return fiber->iterate_exception_catcher(context, e);
        }
    }
}

void Fiber::iterate(Context* context, Closure* closure)
{
    // loop {
    //   if (! go()) return getReturnVal()
    //   yieldResult(stakeCall(closure, getYieldVal()))
    // }

    //HMD_PRINTF("iterate. state=%d,mode=%d\n",m_state,m_mode);
    if (context == m_fiberContext) {
        context->throwException(HSym_fiber_error, M_M("fiber loop"));
        return;
    }
    switch (m_state) {
    case YIELDING:
        // getYieldVal()
        m_fiberContext->stack.cutPaste(context->stack, 1);
        m_state = YIELD_JOIN;

        context->setAfterFfiParam(this);
        context->setAfterFfiFunc(i_iterate_affi);
        context->setFfiExceptionCatcher(i_iterate_ex_catcher);

        // stakeCall
        closure->call(context, 1, true);
        m_iterating_closure = closure;
        context->exceptionToFiber(this);
        break;

    case YIELD_JOIN:
        // yieldの戻り値に nil を与えて継続
        m_fiberContext->pushNil();
        m_state = RUNNING;
        // not break

    case RUNNING:
        context->setAfterFfiParam(this);
        context->setAfterFfiFunc(i_iterate_affi);
        m_mode = MODE_ITERATE;
        m_callerContext = context;
        m_iterating_closure = closure;
        m_contextSwitch(m_callerContext, m_fiberContext);
        break;

    case FINISHED:
        context->throwException(HSym_fiber_error, M_M("already finished"));
        break;

    default:
        HMD_FATAL_ERROR("Fiber::iterate() called in bad state %d", m_state);
    }
}

void Fiber::iterate_affi(Context* context)
{
#if 0
    HMD__PRINTF_FK("iterate_affi m_state=%d m_mode=%d\n",m_state,m_mode);
#endif
    if (m_state == YIELD_JOIN) {
        // yieldResult
        context->stack.cutPaste(m_fiberContext->stack, 1);
        m_state = RUNNING;
        // loop
        m_callerContext = context;
        m_contextSwitch(m_callerContext, m_fiberContext);
    } else if (m_state == RUNNING) {
        // exception
#if 0
        HMD_PRINTF("callerContext = "); m_callerContext->debugPrintCurPos();
        HMD_PRINTF(", fiberContext = "); m_fiberContext->debugPrintCurPos();
        HMD_PRINTF("\n");
#endif
        m_contextSwitch(m_callerContext, m_fiberContext);
    } else if (m_state == FINISHED) {
        context->ffiFinish();
    }
}

bool Fiber::iterate_exception_catcher(Context* context, Exception* e)
{
#if 0
    HMD__PRINTF_FK("iterate_exception_catcher state=%d mode=%d\n",m_state,m_mode);
    e->debugPrint(context);
#endif
    HMD_DEBUG_ASSERT(m_callerContext == context);
    if (m_state != RUNNING)
        return false;
    if (e->type() != HSymR_jump_control)
        return false;

    SymbolID_t label = e->getLabel();
    if (label == HSymR_break) {
        m_callerContext->push(e->val());
        m_fiberContext->thread = NULL;
        m_state = FINISHED;
        return true;
    }
    if (label != HSymR_next)
        return false;

    // next --> loop
    //HMD__PRINTF_FK(" --> next\n");
    //m_state = RUNNING;
    //m_callerContext = context;
    return true;
}


void Fiber::m_GC_mark(void)
{
    if (m_state < FINISHED) {
        Debug::incMarkTreeLevel();
        if (m_fiberContext != NULL)
            GC::markObj(Object::fromCppObj(m_fiberContext));
        if (m_callerContext != NULL)
            GC::markObj(Object::fromCppObj(m_callerContext));
        Debug::decMarkTreeLevel();
    }
}
