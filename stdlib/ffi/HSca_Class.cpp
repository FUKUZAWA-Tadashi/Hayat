/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Class.h"
#include "hySymbolTable.h"

using namespace Hayat::Common;

FFI_DEFINITION_START {

StringBuffer* FFI_FUNC(concatToStringBuffer) (Value selfVal, StringBuffer* sb, hys32 mode)
{
    HMD_ASSERT(selfVal.type->symCheck(HSym_Class));
    HClass* self = (HClass*) selfVal.ptrData;
    sb->concat((const char*) gSymbolTable.id2str(self->getSymbol()));
    if (mode < 0)
        sb->concat(":<Class>");
    return sb;
}

SymbolID_t FFI_FUNC(classSymbol) (Value selfVal)
{
    return selfVal.toClass()->getSymbol();
}

hys32 FFI_FUNC(hashCode) (Value selfVal)
{
    return ((hys32)selfVal.toClass()) >> 2;
}

DEEP_FFI_FUNC_X(3d3d)
{
    HMD_ASSERT(numArgs == 1);
    Value self = context->pop();
    Value x = context->pop();
    context->pushBool(self == x);
}

bool FFI_FUNC_X(3c) (Value selfVal, Value x) // "<"
{
    if (x.type != HC_Class)
        return false;
    const HClass* self = (HClass*) selfVal.ptrData;
    const HClass* xClass = x.toClass();
    return (self->doesInherit(xClass));
}

bool FFI_FUNC_X(3c3d) (Value selfVal, Value x) // "<="
{
    if (x.type != HC_Class)
        return false;
    const HClass* self = (HClass*) selfVal.ptrData;
    const HClass* xClass = x.toClass();
    if (self == xClass)
        return true;
    return (self->doesInherit(xClass));
}

bool FFI_FUNC_X(3e) (Value selfVal, Value x) // ">"
{
    if (x.type != HC_Class)
        return false;
    const HClass* self = (HClass*) selfVal.ptrData;
    const HClass* xClass = x.toClass();
    return (xClass->doesInherit(self));
}

bool FFI_FUNC_X(3e3d) (Value selfVal, Value x) // ">="
{
    if (x.type != HC_Class)
        return false;
    const HClass* self = (HClass*) selfVal.ptrData;
    const HClass* xClass = x.toClass();
    if (self == xClass)
        return true;
    return (xClass->doesInherit(self));
}

} FFI_DEFINITION_END
