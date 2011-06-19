/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include <math.h>
#include "HSca_Int.h"

FFI_DEFINITION_START {

// StringBufferの末尾に文字列表現を追加
StringBuffer* FFI_FUNC(concatToStringBuffer) (Value selfVal, StringBuffer* sb, hys32 mode)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    hys32 self = selfVal.toInt();
    sb->sprintf("%d", (int)self);
    if (mode < 0)
        sb->concat(":<Int>");
    return sb;
}

// ハッシュコード
DEEP_FFI_FUNC(hashCode) {
    HMD_ASSERT(numArgs == 0);
    (void)context;
    // Int値をそのままハッシュコードにする
    // context->pushInt(context->popInt());
}

// "+@" 単項プラス 
DEEP_FFI_FUNC_X(2b40)
{
    HMD_ASSERT(numArgs == 0);
    (void)context;
    //hys32 self = context->popInt();
    //context->pushInt(self);
}

// "+" 加算
hys32 FFI_FUNC_X(2b) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() + x;
}

// "-@" 単項マイナス 
hys32 FFI_FUNC_X(2d40) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return -selfVal.toInt();
}

// "-" 減算
hys32 FFI_FUNC_X(2d) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() - x;
}

// "*" 乗算 
hys32 FFI_FUNC_X(2a) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() * x;
}

// "/" 除算 
DEEP_FFI_FUNC_X(2f)
{
    HMD_ASSERT(numArgs == 1);

    hys32 self = context->popInt();
    hys32 o = context->popInt();
    if (o == 0)
        context->throwException(HSym_division_by_zero, M_M("division by zero"));
    else
        context->pushInt(self / o);
}

// "%" 剰余 
DEEP_FFI_FUNC_X(25)
{
    HMD_ASSERT(numArgs == 1);

    hys32 self = context->popInt();
    hys32 o = context->popInt();
    if (o == 0)
        context->throwException(HSym_division_by_zero, M_M("division by zero"));
    else        
        context->pushInt(self % o);
}

// "**" べき乗 
hys32 FFI_FUNC_X(2a2a) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return (hys32)(powf((hyf32)selfVal.toInt(), (hyf32)x) + 0.5f);
}

// "==" 比較 
bool FFI_FUNC_X(3d3d) (Value selfVal, Value x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    if (x.type->symCheck(HSym_Int)) {
        return selfVal.toInt() == x.toInt();
    } else {
        return false;
    }
}

// "<=>" 比較 
hys32 FFI_FUNC_X(3c3d3e) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    hys32 self = selfVal.toInt();
    if (self == x)
        return 0;
    else if (self < x)
        return -1;
    else
        return 1;
}

// "<" 比較 
bool FFI_FUNC_X(3c) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() < x;
}

// "<=" 比較 
bool FFI_FUNC_X(3c3d) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() <= x;
}

// ">" 比較 
bool FFI_FUNC_X(3e) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() > x;
}

// ">=" 比較 
bool FFI_FUNC_X(3e3d) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() >= x;
}

// "&" ビットAND 
hys32 FFI_FUNC_X(26) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() & x;
}

// "|" ビットOR 
hys32 FFI_FUNC_X(7c) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() | x;
}

// "^" ビットXOR 
hys32 FFI_FUNC_X(5e) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() ^ x;
}

// "~" ビットNOT 
hys32 FFI_FUNC_X(7e) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return ~selfVal.toInt();
}

// Float化 
hyf32 FFI_FUNC(toFloat) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return (hyf32)selfVal.toInt();
}

// 絶対値
hys32 FFI_FUNC(abs) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    hys32 self = selfVal.toInt();
    if (self < 0)
        return -self;
    return self;
}

} FFI_DEFINITION_END
