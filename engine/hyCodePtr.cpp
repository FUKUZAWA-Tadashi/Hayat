/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCodePtr.h"
#include "hyStringBuffer.h"
#include "hyCodeManager.h"
#include <string.h>


using namespace Hayat::Common;
using namespace Hayat::Engine;

void CodePtr::initialize(const hyu8* startAddr)
{
    m_ptr = startAddr;
}

void CodePtr::finalize(void)
{
    m_ptr = NULL;
}


/*
const hyu8* CodePtr::getString(hyu32 offs)
{
    return bytecode()->getString(offs);
}

HClass* CodePtr::mainClass(void)
{
    return bytecode()->mainClass();
}

void CodePtr::getInfo(StringBuffer* sb) const
{
    if (bytecode() != NULL)
        bytecode()->getInfo(sb, m_ptr);
    else
        sb->sprintf("<no bytecode:%x>",m_ptr);
}

#ifdef HMD_DEBUG
void CodePtr::m_debugGetInfo(char* buf, hyu32 bufSize) const
{
    if (bytecode() != NULL)
        bytecode()->m_debugGetInfo(buf, bufSize, m_ptr);
    else
        HMD_STRNCPY(buf, "<no bytecode>", 14);
}
#endif
*/




void MethodPtr::setMethodAddr(const HClass* lexicalScope, const hyu8* methodAddr, hyu16 methodID)
{
    m_lexicalScope = lexicalScope;
    m_ptr = methodAddr;
    m_methodID = methodID;
    m_bFfi = false;
}

void MethodPtr::setFfi(const HClass* lexicalScope, Ffi_t ffi)
{
    m_lexicalScope = lexicalScope;
    m_ptr = (const hyu8*) ffi;
    m_bFfi = true;
}

void MethodPtr::finalize(void)
{
    m_self = NIL_VALUE;
    m_lexicalScope = NULL;
    CodePtr::finalize();
}

void MethodPtr::m_GC_mark(void)
{
    gCodeManager.usingCodeAt(addr());
    gCodeManager.usingCodeAt((const hyu8*) m_lexicalScope->bytecode());
    GC::markValue(m_self);
}
