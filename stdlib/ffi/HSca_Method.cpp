/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Method.h"

FFI_DEFINITION_START {

DEEP_FFI_FUNC(initialize)
{
    HMD_ASSERT(numArgs == 2);
    Method* self = context->popCppObj<Method>(HSym_Method);
    SymbolID_t methodSym = context->popSymbol();
    Value val = context->pop();
    self->initialize(val, methodSym);
    context->pushObj(Object::fromCppObj(self));
}

DEEP_FFI_FUNC(call)
{
    Method* self = context->popCppObj<Method>(HSym_Method);
    self->call(context, numArgs);
}

DEEP_FFI_FUNC_X(2829)      // ()
{
    Method* self = context->popCppObj<Method>(HSym_Method);
    self->call(context, numArgs);
}

} FFI_DEFINITION_END
