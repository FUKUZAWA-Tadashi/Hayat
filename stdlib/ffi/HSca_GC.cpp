/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_GC.h"

FFI_DEFINITION_START {

hys32 FFI_FUNC(countObjects) (Value)
{
    return GC::countObjects(gMemPool);
}

} FFI_DEFINITION_END
