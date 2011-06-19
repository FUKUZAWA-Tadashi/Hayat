/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyException.h"
#include "hyThread.h"
#include "hyStringBuffer.h"
#include "hyGC.h"
#include "hySymbolTable.h"
#include "hyBytecode.h"

using namespace Hayat::Common;
using namespace Hayat::Engine;



void* Exception::operator new(size_t size)
{
#ifndef HMD_DEBUG
    (void)size;
#endif
    HMD_DEBUG_ASSERT(size == sizeof(Exception));
    Object* obj = Object::create(HC_Exception, sizeof(Exception)  + sizeof(hyu32));
    return (void*) obj->cppObj<Exception>();
}

void Exception::operator delete(void* p)
{
    Object::fromCppObj(p)->destroy();
}

Exception::Exception(SymbolID_t type)
    : m_type(type), m_label(HSym_nil), m_val(NIL_VALUE), m_bt(0)
{}

Exception::Exception(SymbolID_t type, const Value& val)
    : m_type(type), m_label(HSym_nil), m_val(val), m_bt(0)
{}

void Exception::initialize(SymbolID_t type, const Value& val)
{
    m_type = type;
    m_val = val;
    m_bt.initialize();
}

void Exception::finalize(void)
{
    m_bt.finalize();
}

void Exception::setVal(Value& val)
{
    GC::writeBarrier(val);
    m_val = val;
}


void Exception::addBackTrace(const CodePtr& ptr, const HClass* ownerClass)
{
    BackTraceInfo_st bi;
    bi.ptr = ptr;
    bi.ownerClass = ownerClass;
    m_bt.subst(m_bt.size(), bi, bi);
}

const Exception::BackTraceInfo_st* Exception::getBackTrace(int level)
{
    if (-level > (int)m_bt.size() || level >= (int)m_bt.size())
        return NULL;
    return m_bt.nthAddr(level);
}

void Exception::m_GC_mark(void)
{
    GC::markValue(m_val);
}

#ifdef HMD_DEBUG
void Exception::debugPrint(Context* context)
{
    char buf[256];
    if (isError())
        HMD_PRINTF("Error thrown: ");
    else
        HMD_PRINTF("Exception thrown: ");
    StringBuffer* sb = new StringBuffer();
    sb->concat((const char*)gSymbolTable.id2str(m_type));
    sb->concat(", ");
    if (m_type == HSymR_jump_control) {
        sb->concat(":");
        sb->concat((const char*)gSymbolTable.id2str(m_label));
        sb->concat(", ");
    }
    sb->concat(context, m_val, -1);
    HMD_PRINTF("%s\n", sb->top());
    delete sb;
    for (int i = 0; (size_t)i < m_bt.size(); i++) {
        const BackTraceInfo_st* inf = getBackTrace(i);
        const Bytecode* bc;
        if (inf->ownerClass == NULL)
            bc = NULL;
        else
            bc = inf->ownerClass->bytecode();
        if (bc != NULL) {
            bc->debugGetInfo(buf, 256, inf->ptr.addr());
            HMD_PRINTF("%2d: %s\n", i, buf);
        } else {
            HMD_PRINTF("%2d: <no bytecode>\n", i);
        }
    }
}
#endif
