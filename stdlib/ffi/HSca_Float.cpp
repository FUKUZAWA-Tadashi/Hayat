/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include <math.h>
#include "HSca_Float.h"

FFI_DEFINITION_START {

// StringBuffer�̖����ɕ�����\����ǉ�
StringBuffer* FFI_FUNC(concatToStringBuffer) (Value selfVal, StringBuffer* sb, hys32 mode)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    hyf32 self = selfVal.toFloat();
    sb->sprintf("%g", self);
    if (mode < 0)
        sb->concat(":<Float>");
    return sb;
}

DEEP_FFI_FUNC(hashCode) {
    HMD_ASSERT(numArgs == 0);
    hyf32 self = context->popFloat();
    context->pushInt((hys32)self);
}

// "+@" �P���v���X 
DEEP_FFI_FUNC_X(2b40)
{
    HMD_ASSERT(numArgs == 0);
    (void)context;
    //hyf32 self = context->popFloat();
    //context->pushFloat(self);
}

// "+" ���Z
hyf32 FFI_FUNC_X(2b) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() + x;
}

// "-@" �P���}�C�i�X 
hyf32 FFI_FUNC_X(2d40) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return -selfVal.toFloat();
}

// "-" ���Z
hyf32 FFI_FUNC_X(2d) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() - x;
}

// "*" ��Z 
hyf32 FFI_FUNC_X(2a) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() * x;
}

// "/" ���Z 
DEEP_FFI_FUNC_X(2f)
{
    HMD_ASSERT(numArgs == 1);

    hyf32 self = context->popFloat();
    hyf32 o = context->popFloat();
    context->pushFloat(self / o);
}

// "**" �ׂ��� 
hyf32 FFI_FUNC_X(2a2a) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return powf(selfVal.toFloat(), x);
}


// "==" ��r 
bool FFI_FUNC_X(3d3d) (Value selfVal, Value x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    if (x.type->symCheck(HSym_Float)) {
        return selfVal.toFloat() == x.toFloat();
    } else {
        return false;
    }
}

// "<=>" ��r 
hys32 FFI_FUNC_X(3c3d3e) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    hyf32 self = selfVal.toFloat();
    if (self == x)
        return 0;
    else if (self < x)
        return -1;
    else
        return 1;
}

// "<" ��r 
bool FFI_FUNC_X(3c) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() < x;
}

// "<=" ��r 
bool FFI_FUNC_X(3c3d) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() <= x;
}

// ">" ��r 
bool FFI_FUNC_X(3e) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() > x;
}

// ">=" ��r 
bool FFI_FUNC_X(3e3d) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() >= x;
}

// Int�� (0�ɋ߂�����) 
hys32 FFI_FUNC(toInt) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return (hys32)selfVal.toFloat();
}

// Int�� (���̎������z���Ȃ�����) 
hys32 FFI_FUNC(floor) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return (hys32)floorf(selfVal.toFloat());
}

} FFI_DEFINITION_END
