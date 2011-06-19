/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Exception.h"

FFI_DEFINITION_START {


Exception* FFI_FUNC(initialize) (Value selfVal, Value val, SymbolID_t type)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    Exception* self = selfVal.toCppObj<Exception>(HSym_Exception);
    self->initialize(type, val);
    return self;
}

hys32 FFI_FUNC(hashCode) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return ((hyu32)selfVal.toCppObj<Exception>()) >> 2;
}

Value FFI_FUNC(getLabel) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    Exception* self = selfVal.toCppObj<Exception>(HSym_Exception);
    if (self->type() == HSym_goto) {
        return Value::fromSymbol(self->getLabel());
    }
    return NIL_VALUE;
}
                          

DEEP_FFI_FUNC(concatToStringBuffer)
{
    (void)numArgs;
    Exception* self = context->popCppObj<Exception>(HSym_Exception);
    hys32 mode = context->popInt();
    StringBuffer* sb = context->popCppObj<StringBuffer>(HSym_StringBuffer);
    sb->concat("((exception)");
    sb->concat(context, self->val(), -1);
    if (self->type() != HSym_exception) {
        sb->concat(",");
        sb->concat(context, Value::fromSymbol(self->type()));
    }
    sb->concat(")");
    if (mode < 0)
        sb->concat(":<Exception>");
    context->pushObj(sb->getObj());
}

DEEP_FFI_FUNC(backTraceStr)
{
    HMD_ASSERT(numArgs == 1);
    Exception* self = context->popCppObj<Exception>(HSym_Exception);
    hys32 i = context->popInt();
    
    if (i < 0 || i >= self->backTraceSize()) {
        context->pushNil();
        return;
    }

    const Exception::BackTraceInfo_st* inf = self->getBackTrace(i);
    HMD_ASSERT(inf != NULL);
    StringBuffer* sb = new StringBuffer();
    inf->ownerClass->bytecode()->debugGetInfo(sb, inf->ptr.addr());
    context->pushObj(sb->getObj());
}


FFI_GCMARK_FUNC()
{
    object->cppObj<Exception>(HSym_Exception)->m_GC_mark();
}

} FFI_DEFINITION_END
