/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Bool.h"

FFI_DEFINITION_START {

// �f�o�b�O�\��
StringBuffer* FFI_FUNC(concatToStringBuffer) (Value selfVal, StringBuffer* sb, hys32 mode)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    bool self = selfVal.toBool();
    sb->concat(self ? "true" : "false");
    if (mode < 0)
        sb->concat(":<Bool>");
    return sb;
}

hys32 FFI_FUNC(hashCode) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toBool() ? 1 : 0;
}

// "!" �_���ے� 
bool FFI_FUNC_X(21) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return ! selfVal.toBool();
}

// "==" ��r 
bool FFI_FUNC_X(3d3d) (Value selfVal, Value x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    if (x.type->symCheck(HSym_Bool))
        return selfVal.toBool() == x.toBool();
    else
        return false;
}

// "||" �_���a
DEEP_FFI_FUNC_X(7c7c)
{
    bool self = context->popBool();
    Value x = context->pop();
    if (self) {
        context->push(TRUE_VALUE);
        return;
    }
    SymbolID_t xsym = x.type->getSymbol();
    bool v;
    if (xsym == HSym_NilClass)
        v = false;
    else if (xsym == HSym_Bool && x.data == 0)
        v = false;
    else
        v = true;
    context->pushBool(v);
}

// "&&" �_����
DEEP_FFI_FUNC_X(2626)
{
    bool self = context->popBool();
    Value x = context->pop();
    if (! self) {
        context->push(FALSE_VALUE);
        return;
    }
    SymbolID_t xsym = x.type->getSymbol();
    bool v;
    if (xsym == HSym_NilClass)
        v = false;
    else if (xsym == HSym_Bool && x.data == 0)
        v = false;
    else
        v = true;
    context->pushBool(v);
}

} FFI_DEFINITION_END
