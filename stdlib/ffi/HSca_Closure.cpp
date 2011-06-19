/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyClosure.h"
#include "HSca_Closure.h"

FFI_DEFINITION_START {

// class method "*cppSize"
DEEP_FFI_FUNC_X(2a63707053697a65)
{
#ifndef HMD_DEBUG
    (void)numArgs;
#endif
    context->pop(); // self
    HMD_DEBUG_ASSERT(numArgs == 0);
    context->pushInt(Closure::needSize(0));
}

DEEP_FFI_FUNC(call)
{
    Closure* self = context->popCppObj<Closure>(HSym_Closure);
    self->call(context, numArgs, false);
}

DEEP_FFI_FUNC_X(2829)      // ()
{
    Closure* self = context->popCppObj<Closure>(HSym_Closure);
    self->call(context, numArgs, false);
}

DEEP_FFI_FUNC(callAway)
{
    Closure* self = context->popCppObj<Closure>(HSym_Closure);
    self->call(context, numArgs, true);
}

FFI_GCMARK_FUNC()
{
    object->cppObj<Closure>(HSym_Closure)->m_GC_mark();
}

} FFI_DEFINITION_END
