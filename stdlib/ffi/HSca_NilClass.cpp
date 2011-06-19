/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_NilClass.h"

FFI_DEFINITION_START {

// StringBuffer�̖����ɕ�����\����ǉ�
StringBuffer* FFI_FUNC(concatToStringBuffer) (Value, StringBuffer* sb, hys32 mode)
{
    if (mode < 0)
        sb->concat("nil:<NilClass>");
    else if (mode == 0)
        sb->concat("nil");
    // mode > 0 �Ȃ牽���ǉ����Ȃ�
    return sb;
}

hys32 FFI_FUNC(hashCode) (Value selfVal)
{
    return 0;
}

bool FFI_FUNC_X(3d3d) (Value, Value o)
{
    return o.type->symCheck(HSym_NilClass);
}

} FFI_DEFINITION_END
