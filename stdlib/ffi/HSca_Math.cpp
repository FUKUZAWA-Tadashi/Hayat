/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Math.h"

#include <math.h>
#include "HSca_Math.h"

FFI_DEFINITION_START {

// ê≥å∑ (class method)
hyf32 FFI_FUNC(sin) (Value selfVal, hyf32 x)
{
    FFI_ASSERT_CLASSMETHOD(selfVal);
    return sinf(x);
}

// ó]å∑ (class method)
hyf32 FFI_FUNC(cos) (Value selfVal, hyf32 x)
{
    FFI_ASSERT_CLASSMETHOD(selfVal);
    return cosf(x);
}

// ïΩï˚ç™ (class method)
hyf32 FFI_FUNC(sqrt) (Value selfVal, hyf32 x)
{
    FFI_ASSERT_CLASSMETHOD(selfVal);
    return sqrtf(x);
}

// é©ëRëŒêî (class method)
hyf32 FFI_FUNC(log) (Value selfVal, hyf32 x)
{
    FFI_ASSERT_CLASSMETHOD(selfVal);
    return logf(x);
}

} FFI_DEFINITION_END
