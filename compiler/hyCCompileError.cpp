/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCCompileError.h"
#include "hpSyntaxTree.h"
#include "hpInputBuffer.h"

namespace Hayat {
    namespace Compiler {
        hyu32     m_compileError_pos = 0;
    }
}

using namespace Hayat::Compiler;
using namespace Hayat::Parser;


void Hayat::Compiler::outEventTitle(bool bErr)
{
#ifdef WIN32
    HMD_PRINTF(bErr ? "error: " : "warning: ");
#else
    HMD_PRINTF(bErr
               ? "\n****** COMPILE ERROR ******\n"
               : "Warning: ");
#endif
}

void Hayat::Compiler::outSourceInfo(hyu32 sourcePos)
{
    SourceInfo si;
    if (gpInp != NULL) {
        gpInp->buildSourceInfo(&si, sourcePos);
#ifdef WIN32
        HMD_PRINTF("%s(%d) : ", si.fname, si.line);
#else
        HMD_PRINTF("%s:%d:%d: ", si.fname, si.line, si.col);
#endif
    }
}

void Hayat::Compiler::outMessage(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    v_outMessage(fmt, ap);
    va_end(ap);
}

void Hayat::Compiler::v_outMessage(const char* fmt, va_list ap)
{
    HMD_VPRINTF(fmt, ap);
    HMD_PRINTF("\n");
}

void Hayat::Compiler::v_outEvent(bool ev, hyu32 sourcePos, const char* fmt, va_list ap)
{
#ifdef WIN32
    outEventTitle(ev);
    outSourceInfo(sourcePos);
#else
    outSourceInfo(sourcePos);
    outEventTitle(ev);
#endif
    v_outMessage(fmt, ap);
}


void Hayat::Compiler::outWarning(hyu32 sourcePos, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    v_outEvent(false, sourcePos, fmt, ap);
    va_end(ap);
}

void Hayat::Compiler::outError(hyu32 sourcePos, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    v_outEvent(true, sourcePos, fmt, ap);
    va_end(ap);
}

void Hayat::Compiler::compileError(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    v_outError(m_compileError_pos, fmt, ap);
    va_end(ap);
    CException::throwEx("compileError");
}

void Hayat::Compiler::compileError_pos(hyu32 sourcePos, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    v_outError(sourcePos, fmt, ap);
    va_end(ap);
    CException::throwEx("compileError");
}

void Hayat::Compiler::compileError_pos(SyntaxTree* st, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    v_outError(st->str.endPos, fmt, ap);
    va_end(ap);
    CException::throwEx("compileError");
}


void Hayat::Compiler::setCompileErrorPos(hyu32 sourcePos)
{
    m_compileError_pos = sourcePos;
}

void Hayat::Compiler::setCompileErrorPos(Hayat::Parser::SyntaxTree* st)
{
    m_compileError_pos = st->str.endPos;
}

hyu32 Hayat::Compiler::getCompileErrorPos(void)
{
    return m_compileError_pos;
}

