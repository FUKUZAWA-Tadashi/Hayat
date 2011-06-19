/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyDebug.h"
#ifndef HY_TOOLS_BUILD
#include "hyThreadManager.h"
#else
#include "hyMemPool.h"
#endif

using namespace Hayat::Engine;
using namespace Hayat::Common;


MemPool* Debug::m_debugMemPool = NULL;


#ifdef HMD_DEBUG

#ifdef HMD__STACK_PRINT
bool Debug::m_bPrintStack = true;
#else
bool Debug::m_bPrintStack = false;
#endif

#ifdef HMD__OPCODE_PRINT
bool Debug::m_bPrintOpcode = true;
#else
bool Debug::m_bPrintOpcode = false;
#endif

#ifdef HMD__DEBUG_MARK_TREE
bool Debug::m_bPrintMarkTree = false;
int Debug::m_printMarkTreeLevel = 0;
#endif

MemPool* Debug::getDebugMemPool(void)
{
    if (m_debugMemPool != NULL)
        return m_debugMemPool;
    return gMemPool;
}

#ifdef HY_TOOLS_BUILD
void Debug::printAllStackTrace(void)
{
    HMD_FATAL_ERROR("Debug::printAllStackTrace() is not available");
}
#else
void Debug::printAllStackTrace(void)
{
    gThreadManager.printAllStackTrace();
}
#endif



#else

MemPool* Debug::getDebugMemPool(void)
{
    return gMemPool;
}

#endif
