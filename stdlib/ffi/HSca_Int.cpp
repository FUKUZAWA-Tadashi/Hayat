/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include <math.h>
#include "HSca_Int.h"

FFI_DEFINITION_START {

// StringBuffer�̖����ɕ�����\����ǉ�
StringBuffer* FFI_FUNC(concatToStringBuffer) (Value selfVal, StringBuffer* sb, hys32 mode)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    hys32 self = selfVal.toInt();
    sb->sprintf("%d", (int)self);
    if (mode < 0)
        sb->concat(":<Int>");
    return sb;
}

// �n�b�V���R�[�h
DEEP_FFI_FUNC(hashCode) {
    HMD_ASSERT(numArgs == 0);
    (void)context;
    // Int�l�����̂܂܃n�b�V���R�[�h�ɂ���
    // context->pushInt(context->popInt());
}

// "+@" �P���v���X 
DEEP_FFI_FUNC_X(2b40)
{
    HMD_ASSERT(numArgs == 0);
    (void)context;
    //hys32 self = context->popInt();
    //context->pushInt(self);
}

// "+" ���Z
hys32 FFI_FUNC_X(2b) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() + x;
}

// "-@" �P���}�C�i�X 
hys32 FFI_FUNC_X(2d40) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return -selfVal.toInt();
}

// "-" ���Z
hys32 FFI_FUNC_X(2d) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() - x;
}

// "*" ��Z 
hys32 FFI_FUNC_X(2a) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() * x;
}

// "/" ���Z 
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

// "%" ��] 
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

// "**" �ׂ��� 
hys32 FFI_FUNC_X(2a2a) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return (hys32)(powf((hyf32)selfVal.toInt(), (hyf32)x) + 0.5f);
}

// "==" ��r 
bool FFI_FUNC_X(3d3d) (Value selfVal, Value x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    if (x.type->symCheck(HSym_Int)) {
        return selfVal.toInt() == x.toInt();
    } else {
        return false;
    }
}

// "<=>" ��r 
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

// "<" ��r 
bool FFI_FUNC_X(3c) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() < x;
}

// "<=" ��r 
bool FFI_FUNC_X(3c3d) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() <= x;
}

// ">" ��r 
bool FFI_FUNC_X(3e) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() > x;
}

// ">=" ��r 
bool FFI_FUNC_X(3e3d) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() >= x;
}

// "&" �r�b�gAND 
hys32 FFI_FUNC_X(26) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() & x;
}

// "|" �r�b�gOR 
hys32 FFI_FUNC_X(7c) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() | x;
}

// "^" �r�b�gXOR 
hys32 FFI_FUNC_X(5e) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toInt() ^ x;
}

// "~" �r�b�gNOT 
hys32 FFI_FUNC_X(7e) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return ~selfVal.toInt();
}

// Float�� 
hyf32 FFI_FUNC(toFloat) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return (hyf32)selfVal.toInt();
}

// ��Βl
hys32 FFI_FUNC(abs) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    hys32 self = selfVal.toInt();
    if (self < 0)
        return -self;
    return self;
}

} FFI_DEFINITION_END
