/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_HyTest_ffi.h"

using namespace Hayat::Common;

FFI_DEFINITION_START {

FFI_S_MEMBER_GETTER(memb,hys32,Membtest);
FFI_S_MEMBER_SETTER(memb,hys32,Membtest,hys32);

bool FFI_FUNC(paramCheck) (Value, hys32 x1, hyf32 x2, ValueArray* x3)
{
    return ((x1 == x3->nth(0).toInt()) && (x2 == x3->nth(1).toFloat()));
}

DEEP_FFI_FUNC(myThrow)
{
    HMD_ASSERT(numArgs == 1);
    context->pop();     // self
    context->throwException(HSym_exception, context->pop());
    // –ß‚è’l‚Ípush‚µ‚È‚¢
}

ValueArray* FFI_FUNC(fac) (Value, hys32 x)
{
    ValueArray* arr = new ValueArray(x);
    hys32 f = 1;
    for (hys32 i = 0; i < x; ++i) {
        f *= i+1;
        arr->subst(i, Value::fromInt(f), NIL_VALUE);
    }
    return arr;
}

const char* FFI_FUNC(toStringTest) (Value, const char* x)
{
    return x;
}

StringBuffer* FFI_FUNC(upcase) (Value, StringBuffer* x)
{
    hyu16 len = x->length();
    StringBuffer* sb = new StringBuffer(len + 1);
    sb->concat(x->top(), len);
    char* p = sb->top();
    for (hyu16 i = 0; i < len; ++i) {
        char c = *p;
        if ((c >= 'a') && (c <= 'z'))
            *p = c - 'a' + 'A';
        ++p;
    }
    return sb;
}

hys32 FFI_FUNC(defval1) (Value, Value x1, Value x2, Value x3)
{
    return x1.toInt() + x2.toInt() + x3.toInt() + 1;
}

hyf32 FFI_FUNC(defval2) (Value, hys32 x1, hys32 x2, hyf32 x3)
{
    return x1 + x2 + x3 + 1.0f;
}

hys32 FFI_FUNC(over) (Value, hys32 x)
{
    return 100 + x;
}

hys32 FFI_FUNC(over) (Value, hys32 x1, hys32 x2)
{
    return 200 + x1 + x2;
}

hys32 FFI_FUNC(over) (Value, hys32 x1, hys32 x2, hyf32 x3)
{
    return (hys32)((300 + x1 + x2) * x3 / 100.0f);
}

} FFI_DEFINITION_END
