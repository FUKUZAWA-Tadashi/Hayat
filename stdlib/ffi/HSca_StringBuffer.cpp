/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_StringBuffer.h"
#include "hyPack.h"


FFI_DEFINITION_START {

// インスタンス初期化
StringBuffer* FFI_FUNC(initialize) (Value selfVal, hys32 bufSize)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    StringBuffer* self = selfVal.toCppObj<StringBuffer>(HSym_StringBuffer);
    self->initialize(bufSize);
    return self;
}

hys32 FFI_FUNC(hashCode) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return StringBuffer::calcHashCode(selfVal.toCppObj<StringBuffer>()->top());
}


// 引数のStringBufferの末尾に文字列を追加
StringBuffer* FFI_FUNC(concatToStringBuffer) (Value selfVal, StringBuffer* sb, hys32 mode)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    StringBuffer* self = selfVal.toCppObj<StringBuffer>(HSym_StringBuffer);
    if (mode < 0)
        sb->sprintf("\"%s\":<StringBuffer>", self->top());
    else
        sb->concat(self->top(), self->length());
    return sb;
}

// 引数オブジェクトを文字列化して末尾に追加
DEEP_FFI_FUNC(concat)
{
    StringBuffer* self = context->popCppObj<StringBuffer>(HSym_StringBuffer);
    int n = numArgs;
    while (--n >= 0) {
        Value d = context->stack.getNth(n+1);
        self->concat(context, d, 0);
    }
    context->stack.drop(numArgs);
    context->pushObj(self->getObj());
}

// バッファを空にする
StringBuffer* FFI_FUNC(clear) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    StringBuffer* self = selfVal.toCppObj<StringBuffer>();
    self->clear();
    return self;
}

// 比較
bool FFI_FUNC_X(3d3d) (Value selfVal, Value o)
{
    return StringBuffer::compare(selfVal, o) == 0;
}

DEEP_FFI_FUNC_X(3c3d3e)
{
    HMD_ASSERT(numArgs == 1);
    Value self = context->pop();
    Value o = context->pop();
    hys32 result = StringBuffer::compare(self, o);
    if (result == StringBuffer::COMPARE_ERROR) {
        context->throwException(HSym_type_mismatch, NIL_VALUE);
        return;
    }
    context->pushInt(result);
}

StringBuffer* FFI_FUNC(sayCommandStart) (Value selfVal, hys32, hys32)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toCppObj<StringBuffer>();
}

DEEP_FFI_FUNC(sayCommand)
{
    HMD_ASSERT(numArgs == 1);
    StringBuffer* self = context->popCppObj<StringBuffer>(HSym_StringBuffer);
    Value d = context->pop();
    self->concat(context, d, 1);
    context->pushObj(self->getObj());
}

StringBuffer* FFI_FUNC(sayCommandEnd) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toCppObj<StringBuffer>();
}

ValueArray* FFI_FUNC(unpack) (Value selfVal, const char* templ)
{
    StringBuffer* self = selfVal.toCppObj<StringBuffer>();
    ValueArray* va = new ValueArray(0);
    Pack::unpack(templ, va, self);
    return va;
}

} FFI_DEFINITION_END
