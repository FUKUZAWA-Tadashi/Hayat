/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Object.h"

FFI_DEFINITION_START {

// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
StringBuffer* FFI_FUNC(concatToStringBuffer) (Value selfVal, StringBuffer* sb, hys32 mode)
{
    sb->sprintf("(%x)", selfVal.data);
    if (mode < 0) {
        sb->sprintf(":<%s>", selfVal.getType()->name());
    }
    return sb;
}

const HClass* FFI_FUNC(getClass) (Value selfVal)
{
    return selfVal.getType();
}

SymbolID_t FFI_FUNC(classSymbol) (Value selfVal)
{
    return selfVal.getType()->getSymbol();
}

hys32 FFI_FUNC(hashCode) (Value selfVal)
{
    if (selfVal.getType()->isPrimitive())
        return (hys32) selfVal.data;

    Object* obj = selfVal.objPtr;
    size_t size = obj->size();
    int size_l = size & 3;      // サイズ下位2bit
    int size_h = size >> 2;
    hyu32* p = (hyu32*) obj->field(0);
    hyu32 h = *p;
    while (--size_h >= 0) {
        h = h * 0x1f1f1f1f + *++p;
    }
    hyu8* q = (hyu8*) p;
    while (--size_l >= 0) {
        h = h * 31 + *q++;
    }

    return (hys32)h;
}

bool FFI_FUNC_Q(sameObject) (Value selfVal, Value o)
{
    return (selfVal.data == o.data) && (selfVal.getType() == o.getType());
}

// "=="
bool FFI_FUNC_X(3d3d) (Value selfVal, Value o)
{
    if (selfVal.getType() != o.getType()) {
        return false;
    }
    if (selfVal.data == o.data) {
        return true;
    }
    if (selfVal.getType()->isPrimitive()) {
        return false;
    }
    Object* o1 = (Object*) selfVal.data;
    HMD_ASSERT(o1->type() == selfVal.getType());

    return o1->equals(VM::getContext(), (Object*)o.data);
}

// "<>"
DEEP_FFI_FUNC_X(3c3e) {
    HMD_ASSERT(numArgs == 1);
    context->execMethod(HSymx_3d3d, 1);
    context->pushBool(! context->popBool());
}

// "!="
DEEP_FFI_FUNC_X(213d) {
    m_HSfx_3c3e(context, numArgs);
}

bool FFI_FUNC_Q(haveMethod) (Value selfVal, SymbolID_t methodSymbol) {
    return selfVal.getScope()->haveMethod(methodSymbol);
}

} FFI_DEFINITION_END
