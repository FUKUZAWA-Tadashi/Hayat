/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyMethod.h"
#include "hyContext.h"

using namespace Hayat::Common;
using namespace Hayat::Engine;


Method::Method(Value& self, SymbolID_t methodSym)
    : m_self(self), m_methodSym(methodSym)
{}

void Method::initialize(Value& self, SymbolID_t methodSym)
{
    m_self = self;
    m_methodSym = methodSym;
}

void Method::call(Context* context, hyu8 numArgs)
{
    context->methodCall(m_self, m_methodSym, numArgs);
}

void Method::m_GC_mark(void)
{
    GC::markValue(m_self);
}
