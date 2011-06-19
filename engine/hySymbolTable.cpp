/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


#include "hySymbolTable.h"
#include "hyEndian.h"
#include "hyDebug.h"
#include "hyMMes.h"
#include <stdio.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;

// グローバルシンボルテーブル
SymbolTable Hayat::Engine::gSymbolTable;


SymbolTable::SymbolTable(void)
    : m_numSymbols(0),
      m_tableSize(0),
      m_table(NULL),
      m_tableBottom(NULL),
      m_fileBuf(NULL),
      m_id2str(NULL)
{
}

void SymbolTable::readTable(const hyu8* buf)
{
    m_numSymbols = Endian::unpack<hys32>(buf);
    m_tableSize = Endian::unpack<hys32>(buf + sizeof(hys32));
    m_table = buf + sizeof(hys32) * 2;
    m_id2str = Debug::getDebugMemPool()->allocT<const hyu8*>(m_numSymbols);
    const hyu8* p = m_table;
    for (int i = 0; i < m_numSymbols; i++) {
        m_id2str[i] = p;
        while (*p++ != '\0') ;
    }
    HMD_ASSERT(p <= m_table + m_tableSize);
    m_tableBottom = (hyu8*)p;
}

void SymbolTable::takeTable(const hyu8* buf, hyu32 size)
{
    m_fileBuf = hmd_allocFileBuf(size, Debug::getDebugMemPool());
    HMD_MEMCPY(m_fileBuf, buf, size);
    readTable(m_fileBuf);
}

bool SymbolTable::readFile(const char* fpath)
{
    hyu32 length = hmd_loadFile_inPath(fpath, &m_fileBuf, NULL, Debug::getDebugMemPool());
    if (length == 0)
        return false;

    readTable((const hyu8*)m_fileBuf);

    return true;
}

void SymbolTable::finalize(void)
{
    hmd_freeFileBuf(m_fileBuf, Debug::getDebugMemPool());
    m_fileBuf = NULL;
    m_numSymbols = 0;
    m_tableSize = 0;
    Debug::getDebugMemPool()->free(m_id2str);
    m_id2str = NULL;
}


const hyu8* SymbolTable::id2str(SymbolID_t id)
{
    static int n = 0;
    static char tmp[8][32];

    if (m_numSymbols == 0 || id >= m_numSymbols) {
        int k = n;
        if (++n >= 4) n = 0;
        HMD_SNPRINTF(tmp[k], 32, "{%d}", id);
        return (const hyu8*) tmp[k];
    }
    return m_id2str[id];
}

void SymbolTable::id2str_n(hyu8* buf, int bufSize, SymbolID_t id)
{
    if (id < m_numSymbols) {
        HMD_STRNCPY((char*)buf, (const char*)m_id2str[id], bufSize);
    } else {
        *buf = '\0';
    }
    char tmp[16];
    HMD_SNPRINTF(tmp, 16, "{%d}", id);
    HMD_STRSCAT((char*)buf, tmp, bufSize);
}

SymbolID_t SymbolTable::addSymbol(const hyu8* str)
{
    HMD_ASSERTMSG(m_tableBottom != NULL, M_M("symbol table is not initialized"));
    int len = HMD_STRLEN((const char*)str) + 1;
    if (m_tableBottom + len <= m_table + m_tableSize) {
        HMD_STRNCPY((char*)m_tableBottom, (const char*)str, len);
        m_id2str = Debug::getDebugMemPool()->reallocT<const hyu8*>(m_id2str, m_numSymbols);
        m_id2str[m_numSymbols] = m_tableBottom;
        m_tableBottom += len;
        return (SymbolID_t) m_numSymbols++;
    } else {
        return SYMBOL_ID_ERROR;
    }
}

SymbolID_t SymbolTable::symbolID(const char* str, hyu32 len)
{
    for (int i = 0; i < m_numSymbols; i++) {
        const char* p = (const char*)m_id2str[i];
        if (p[len] == '\0')
            if (!HMD_STRNCMP(str, p, len))
                return (SymbolID_t)i;
    }
    return SYMBOL_ID_ERROR;
}

SymbolID_t SymbolTable::symbolID(const char* str)
{
    return symbolID(str, HMD_STRLEN(str));
}
