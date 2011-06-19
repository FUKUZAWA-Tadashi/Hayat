/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyDebugSourceInfo.h"
#include "hyMemPool.h"
#include "hySymbolTable.h"
#include "hyEndian.h"

namespace Hayat {
    namespace Engine {
        DebugSourceInfo_t NULL_DebugSourceInfo = {
            NULL, 0, 0
        };
    }
}

using namespace Hayat::Engine;


DebugInfos::DebugInfos(void)
    : m_bytecodeSymbol(0),
      m_debInfos(NULL),
      m_debFileBuf(NULL),
      m_numDebInfos(0),
      m_numPaths(0),
      m_paths(NULL),
      m_haveDebugInfo(false)
{
}

DebugInfos::DebugInfos(const char* fpath)
    : m_bytecodeSymbol(0),
      m_debInfos(NULL),
      m_debFileBuf(NULL),
      m_numDebInfos(0),
      m_numPaths(0),
      m_paths(NULL),
      m_haveDebugInfo(false)
{
    readDebugInfo(fpath);
}

DebugInfos::DebugInfos(const hyu8* buf, hyu32 size)
    : m_bytecodeSymbol(0),
      m_debInfos(NULL),
      m_debFileBuf(NULL),
      m_numDebInfos(0),
      m_numPaths(0),
      m_paths(NULL),
      m_haveDebugInfo(false)
{
    setDebugInfo(buf, size);
}

DebugInfos::~DebugInfos()
{
    hmd_freeFileBuf(m_debFileBuf, Debug::getDebugMemPool());
    m_debFileBuf = NULL;
    m_debInfos = NULL;
    m_numDebInfos = 0;
    m_numPaths = 0;
    m_paths = NULL;
    m_haveDebugInfo = false;
}

void* DebugInfos::operator new(size_t size)
{
    return (DebugInfos*) Debug::getDebugMemPool()->alloc(size, "DEBI");
}
    
void DebugInfos::operator delete(void* p)
{
    Debug::getDebugMemPool()->free(p);
}


void DebugInfos::debugGetInfo(char* buf, hyu32 bufSize, hyu32 pos) const
{
    if (m_haveDebugInfo) {
        if (pos == 0) {
            HMD_SNPRINTF(buf, bufSize, "%s:-----", gSymbolTable.id2str(m_bytecodeSymbol));
        } else {
            DebugSourceInfo_t dsi = searchDebugInfo(pos);
            HMD_SNPRINTF(buf, bufSize,
                         "%s:%d:%d",
                         dsi.path, dsi.line, dsi.col);
        }
    } else {
        HMD_SNPRINTF(buf, bufSize,
                     "%s:: %8x",
                     gSymbolTable.id2str(m_bytecodeSymbol),
                     pos);
    }
}


DebugSourceInfo_t DebugInfos::searchDebugInfo(hyu32 pos) const
{
    if (m_numDebInfos == 0)
        return NULL_DebugSourceInfo;
    int i = 0;
    int j = m_numDebInfos;
    int k;
    while (i+1 < j) {
        k = (i + j) / 2;
        if (m_debInfos[k].addr == pos) {
            i = k;
            break;
        } else if (m_debInfos[k].addr > pos)
            j = k;
        else
            i = k;
    }
    DebugSourceInfo_t dsi;
    hyu16 id = Endian::unpack<hyu16>((const hyu8*)&m_debInfos[i].pathId);
    HMD_DEBUG_ASSERT(id < m_numPaths);
    const char* pp = m_paths;
    while (id-- > 0) {
        while (*pp++ != '\0')
            ;
    }
    dsi.path = pp;
    dsi.line = Endian::unpack<hyu16>((const hyu8*)&m_debInfos[i].line);
    dsi.col = Endian::unpack<hyu16>((const hyu8*)&m_debInfos[i].col);
    return dsi;
}

void DebugInfos::readDebugInfo(const char* fpath)
{
    hyu32 length = hmd_loadFile_inPath(fpath, &m_debFileBuf, NULL, Debug::getDebugMemPool());
    if (length != 0) {
#if 0
        HMD_PRINTF("debug info file loaded: %s\n", fpath);
#endif
        setDebugInfo((const hyu8*)m_debFileBuf, length);
    }
#if 0
    else {
        HMD_PRINTF("debug info file load failed: %s\n", fpath);
    }
#endif

}
    
void DebugInfos::setDebugInfo(const hyu8* buf, hyu32 size)
{
    (void)size;
    m_debInfos = (DebInfo_t*)buf;
    m_numDebInfos = 0;
    while (m_debInfos[m_numDebInfos].addr != 0) {
        m_debInfos[m_numDebInfos].addr = Endian::unpack<hyu16>((const hyu8*)&m_debInfos[m_numDebInfos].addr);
        ++ m_numDebInfos;
    }
    const hyu8* p = (const hyu8*)&m_debInfos[m_numDebInfos];
    m_numPaths = Endian::unpack<hyu16>(p+4);
    m_paths = (const char*)(p+6);
    m_haveDebugInfo = true;
}

void DebugInfos::getNthInfo(char* buf, hyu32 bufSize, hyu32 n) const
{
    if (n >= m_numDebInfos) {
        *buf = '\0';
        return;
    }

    hyu16 id = Endian::unpack<hyu16>((const hyu8*)&m_debInfos[n].pathId);
    HMD_DEBUG_ASSERT(id < m_numPaths);
    const char* pp = m_paths;
    while (id-- > 0) {
        while (*pp++ != '\0')
            ;
    }
    hyu16 line = Endian::unpack<hyu16>((const hyu8*)&m_debInfos[n].line);
    hyu16 col = Endian::unpack<hyu16>((const hyu8*)&m_debInfos[n].col);

    HMD_SNPRINTF(buf, bufSize,
                 "%4x:%4d:%2d:%s",
                 m_debInfos[n].addr, line, col, pp);
}
