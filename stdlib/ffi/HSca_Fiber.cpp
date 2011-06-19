/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Fiber.h"
#include "hyClosure.h"

FFI_DEFINITION_START {

DEEP_FFI_FUNC(initialize)
{
    HMD_ASSERT(numArgs == 1);
    Fiber* self = context->popCppObj<Fiber>(HSym_Fiber);
    Closure* closure = context->popCppObj<Closure>(HSym_Closure);
    self->initialize(context, closure, 0);
    context->pushObj(Object::fromCppObj(self));
}

DEEP_FFI_FUNC(go)
{
    HMD_ASSERT(numArgs == 0);
    Fiber* self = context->popCppObj<Fiber>(HSym_Fiber);
    self->go(context);
}

DEEP_FFI_FUNC(getYieldVal)
{
    HMD_ASSERT(numArgs == 0);
    Fiber* self = context->popCppObj<Fiber>(HSym_Fiber);
    self->getYieldVal(context);
}

DEEP_FFI_FUNC(getReturnVal)
{
    HMD_ASSERT(numArgs == 0);
    Fiber* self = context->popCppObj<Fiber>(HSym_Fiber);
    self->getReturnVal(context);
}

DEEP_FFI_FUNC(generate)
{
    HMD_ASSERT(numArgs == 0);
    Fiber* self = context->popCppObj<Fiber>(HSym_Fiber);
    self->generate(context);
}

DEEP_FFI_FUNC(iterate)
{
    HMD_ASSERT(numArgs == 1);
    Value selfVal = context->pop();
    Value v = context->pop();
    if (v == NIL_VALUE) {
        context->push(selfVal);
        return;
    }
    if (v.getTypeSymbol() != HSym_Closure) {
        context->throwException(HSym_type_mismatch, M_M("Closure required"));
        return;
    }
    Fiber* self = selfVal.toCppObj<Fiber>(HSym_Fiber);
    Closure* closure = v.toCppObj<Closure>();
    self->iterate(context, closure);
}


DEEP_FFI_FUNC(yieldResult)
{
    Fiber* self = context->popCppObj<Fiber>(HSym_Fiber);
    self->yieldResult(context, numArgs);
    context->pushNil();
}

DEEP_FFI_FUNC(thrown)
{
    HMD_ASSERT(numArgs == 1);
    Fiber* self = context->popCppObj<Fiber>(HSym_Fiber);
    Exception* e = context->popCppObj<Exception>(HSym_Exception);
    self->thrown(context, e);
    // self->switchToYieldContext();
}

DEEP_FFI_FUNC(stakeCall)
{
    Fiber* self = context->popCppObj<Fiber>(HSym_Fiber);
    context->stack.rot(numArgs);
    Closure* closure = context->popCppObj<Closure>(HSym_Closure);
    self->stakeCall(context, closure, numArgs-1);
}

bool FFI_FUNC_Q(finished) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toCppObj<Fiber>()->isFinished();
}


FFI_GCMARK_FUNC()
{
    object->cppObj<Fiber>(HSym_Fiber)->m_GC_mark();
}

} FFI_DEFINITION_END
