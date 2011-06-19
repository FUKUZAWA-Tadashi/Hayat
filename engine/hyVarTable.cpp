/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyVarTable.h"
#include "hyMemPool.h"
#include "hyBinarySearch.h"
#include "hyGC.h"
#include "hySymbolTable.h"
#include "hyDebug.h"
#include <string.h>


using namespace Hayat::Common;
using namespace Hayat::Engine;


VarTable Hayat::Engine::gGlobalVar;


VarTable::VarTable(void)
    : m_bufSize(0), m_numVars(0), m_symbolTable(NULL), m_varTable(NULL)
{
}

VarTable::~VarTable()
{
}

void VarTable::initialize(int initSize)
{
    m_bufSize = initSize;
    m_numVars = 0;
    m_symbolTable = gMemPool->allocT<SymbolID_t>(initSize, "VSYM");
    m_varTable = gMemPool->allocT<Value>(initSize, "VTBL");
}

void VarTable::finalize(void)
{
    m_bufSize = 0;
    m_numVars = 0;
    gMemPool->free(m_symbolTable);
    m_symbolTable = NULL;
    gMemPool->free(m_varTable);
    m_varTable = NULL;
}

Value* VarTable::getVarAddr_notCreate(SymbolID_t varSym)
{
    int idx;
    if (! binarySearch<SymbolID_t>(m_symbolTable, m_numVars, varSym, &idx)) {
        return NULL;
    }
    return &m_varTable[idx];
}

Value* VarTable::getVarAddr(SymbolID_t varSym)
{
    int idx;
    if (! binarySearch<SymbolID_t>(m_symbolTable, m_numVars, varSym, &idx)) {
        m_insert(idx, varSym);
    }
    return &m_varTable[idx];
}

void VarTable::m_insert(int idx, SymbolID_t varSym)
{
    if (m_numVars >= m_bufSize) {
        hyu16 newBufSize = m_bufSize + m_EXPAND_SIZE;
        m_symbolTable = gMemPool->reallocT<SymbolID_t>(m_symbolTable, newBufSize);
        HMD_ASSERT(m_symbolTable != NULL);
        m_varTable = gMemPool->reallocT<Value>(m_varTable, newBufSize);
        HMD_ASSERT(m_varTable != NULL);
        m_bufSize = newBufSize;
    }
    memmove(&m_symbolTable[idx+1], &m_symbolTable[idx], (m_numVars-idx) * sizeof(SymbolID_t));
    memmove(&m_varTable[idx+1], &m_varTable[idx], (m_numVars-idx) * sizeof(Value));
    ++ m_numVars;
    m_symbolTable[idx] = varSym;
    m_varTable[idx] = NIL_VALUE;
}


void VarTable::m_GC_mark(void)
{
    Value* pv = m_varTable;
    for (int i = 0; i < m_numVars; i++)
        GC::markValue(*pv++);
}


void VarTable::startMarkIncremental(void)
{
    GC::m_pValArray = m_varTable;
    GC::m_arrIdx = 0;
    GC::m_countdown_inc = GC::m_num_perPart_inc;
    Debug::incMarkTreeLevel();
}

void VarTable::markIncremental(void)
{
    for ( ; GC::m_arrIdx < m_numVars; ++GC::m_arrIdx) {
#ifdef HMD__DEBUG_MARK_TREE
        if (Debug::isPrintMarkTreeOn())
            HMD_PRINTF("mark var %s\n", gSymbolTable.id2str(m_symbolTable[GC::m_arrIdx]));
#endif
        GC::markValue(*(GC::m_pValArray)++);
        if (--GC::m_countdown_inc <= 0) {
            GC::m_countdown_inc += GC::m_num_perPart_inc;
            return;
        }
    }
    GC::m_subPhase_end = true;
    Debug::decMarkTreeLevel();
}
