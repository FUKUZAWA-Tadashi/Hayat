/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Context.h"

FFI_DEFINITION_START {

FFI_GCMARK_FUNC()
{
    object->cppObj<Context>(HSym_Context)->m_GC_mark();
}

} FFI_DEFINITION_END
