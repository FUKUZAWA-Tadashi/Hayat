/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include <math.h>
#include "HSca_Float.h"

FFI_DEFINITION_START {

// StringBufferÇÃññîˆÇ…ï∂éöóÒï\åªÇí«â¡
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

// "+@" íPçÄÉvÉâÉX 
DEEP_FFI_FUNC_X(2b40)
{
    HMD_ASSERT(numArgs == 0);
    (void)context;
    //hyf32 self = context->popFloat();
    //context->pushFloat(self);
}

// "+" â¡éZ
hyf32 FFI_FUNC_X(2b) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() + x;
}

// "-@" íPçÄÉ}ÉCÉiÉX 
hyf32 FFI_FUNC_X(2d40) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return -selfVal.toFloat();
}

// "-" å∏éZ
hyf32 FFI_FUNC_X(2d) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() - x;
}

// "*" èÊéZ 
hyf32 FFI_FUNC_X(2a) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() * x;
}

// "/" èúéZ 
DEEP_FFI_FUNC_X(2f)
{
    HMD_ASSERT(numArgs == 1);

    hyf32 self = context->popFloat();
    hyf32 o = context->popFloat();
    context->pushFloat(self / o);
}

// "**" Ç◊Ç´èÊ 
hyf32 FFI_FUNC_X(2a2a) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return powf(selfVal.toFloat(), x);
}


// "==" î‰är 
bool FFI_FUNC_X(3d3d) (Value selfVal, Value x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    if (x.type->symCheck(HSym_Float)) {
        return selfVal.toFloat() == x.toFloat();
    } else {
        return false;
    }
}

// "<=>" î‰är 
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

// "<" î‰är 
bool FFI_FUNC_X(3c) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() < x;
}

// "<=" î‰är 
bool FFI_FUNC_X(3c3d) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() <= x;
}

// ">" î‰är 
bool FFI_FUNC_X(3e) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() > x;
}

// ">=" î‰är 
bool FFI_FUNC_X(3e3d) (Value selfVal, hyf32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toFloat() >= x;
}

// Intâª (0Ç…ãﬂÇ¢êÆêî) 
hys32 FFI_FUNC(toInt) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return (hys32)selfVal.toFloat();
}

// Intâª (å≥ÇÃé¿êîÇâzÇ¶Ç»Ç¢êÆêî) 
hys32 FFI_FUNC(floor) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return (hys32)floorf(selfVal.toFloat());
}

} FFI_DEFINITION_END
