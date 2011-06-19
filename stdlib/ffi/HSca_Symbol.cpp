/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Symbol.h"
#include "hySymbolTable.h"

FFI_DEFINITION_START {

// デバッグ表示
StringBuffer* FFI_FUNC(concatToStringBuffer) (Value selfVal, StringBuffer* sb, hys32 mode)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    SymbolID_t self = selfVal.toSymbol();
    if (mode <= 0)
        sb->concat(":");
    sb->sprintf("%s", gSymbolTable.id2str(self));
    if (mode < 0)
        sb->concat(":<Symbol>");
    return sb;
}

hys32 FFI_FUNC(hashCode) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return (hys32)(selfVal.data);
}

// "==" 比較 
bool FFI_FUNC_X(3d3d) (Value selfVal, Value o)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    if (o.type->symCheck(HSym_Symbol)) {
        return selfVal == o;
    } else {
        return false;
    }
}

} FFI_DEFINITION_END
