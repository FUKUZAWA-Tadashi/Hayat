/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_String.h"

FFI_DEFINITION_START {

// StringBuffer‚Ì––”ö‚É•¶Žš—ñ‚ð’Ç‰Á
StringBuffer* FFI_FUNC(concatToStringBuffer) (Value selfVal, StringBuffer* sb, hys32 mode)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    const char* str = selfVal.toString();
    if (mode < 0) {
        sb->sprintf("\"%s\":<String>", str);
    } else {
        sb->concat(str, HMD_STRLEN(str));
    }
    return sb;
}

hys32 FFI_FUNC(hashCode) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return StringBuffer::calcHashCode(selfVal.toString());
}

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

hys32 FFI_FUNC(length) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    const char* str = selfVal.toString();
    return (hys32) HMD_STRLEN(str);
}

} FFI_DEFINITION_END
