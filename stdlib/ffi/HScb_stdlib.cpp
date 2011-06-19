/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HScb_stdlib.h"

FFI_DEFINITION_START {


void FFI_FUNC(assert)  (Value, bool cond) {
    if (!cond)
        VM::getContext()->throwError(M_M("assertion failed"));
}

DEEP_FFI_FUNC(assertMsg) {
    context->pop(); // self
    HMD_ASSERT(numArgs > 0);
    bool x = context->stack.getNth(numArgs).toBool();
    if (!x) {
        context->debugPrintValues(numArgs-1, 0, "");
        HMD_PRINTF("\n");
        context->throwError(M_M("assertion failed"));
    } else {
        context->stack.drop(numArgs);
        context->pushNil();
    }
}

DEEP_FFI_FUNC(warning) {
    context->pop(); // self
    HMD_ASSERT(numArgs > 0);
    context->debugPrintValues(numArgs, 0, "");
    HMD_PRINTF("\n");
    context->pushNil();
}

Value FFI_FUNC(getRunningThread) (Value)
{
    Context* context = VM::getContext();
    if (context->thread == NULL)
        return NIL_VALUE;
    return Value::fromObj(Object::fromCppObj(context->thread));
}

void FFI_FUNC(sleep) (Value)
{
    Context* context = VM::getContext();
    if (context->thread == NULL) {
        context->throwException(HSym_exception, M_M("sleep() called out of thread"));
    } else {
        context->thread->sleep();
    }
}



} FFI_DEFINITION_END
