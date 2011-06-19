/*  -*- coding: sjis-dos -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_HyTest_ffi__Inner.h"

using namespace Hayat::Common;
using namespace Hayat::Engine;

FFI_DEFINITION_START {

hys32 FFI_FUNC(testAdd) (Value, hys32 x, hys32 y)
{
    return x + y;
}

} FFI_DEFINITION_END
