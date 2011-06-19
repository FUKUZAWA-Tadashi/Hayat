/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyStringBox.h"


using namespace Hayat::Common;
using namespace Hayat::Engine;

StringBox::StringBox(void)
    : m_strStorage(0), m_markFlags(0)
{
}

void StringBox::finalize(void)
{
    m_strStorage.finalize();
    m_markFlags.finalize();
}

static int cmpfn(const char*& p1, const char*& p2)
{
    return HMD_STRCMP(p1, p2);
}

const char* StringBox::store(const char* str)
{
    int idx;
    if (binarySearchFn(m_strStorage.top(), m_strStorage.size(), cmpfn, str, &idx)) {
        return m_strStorage[idx];
    }

    size_t len = HMD_STRLEN(str);
    MemCell* mem = gMemPool->alloc(len + 1, "strB");
    HMD_ASSERT(mem != NULL);
    HMD_STRNCPY((char*)mem, str, len+1);
    m_strStorage.insert(idx) = (const char*)mem; // ‘}“ü
    m_markFlags.insertAt(idx, true);
    //HMD_PRINTF("StringBox new store('%s' %x)\n",mem,mem);
    return (const char*)mem;
}


void StringBox::unmark(void)
{
    m_markFlags.changeSize(m_strStorage.size());
    m_markFlags.setAll(false);
}

void StringBox::mark(const char* str)
{
    int idx;
    if (binarySearchFn(m_strStorage.top(), m_strStorage.size(), cmpfn, str, &idx)) {
        m_markFlags.setAt(idx, true);
        //HMD_PRINTF("StringBox mark('%s' %x)\n",m_strStorage[idx],m_strStorage[idx]);
    }
}

void StringBox::sweep(void)
{
    hys32 i = m_strStorage.size();
    HMD_DEBUG_ASSERT(i == m_markFlags.size());
    while (--i >= 0) {
        if (! m_markFlags.getAt(i)) {
            // mark‚³‚ê‚Ä‚¢‚È‚¢
            //HMD_PRINTF("StringBox swept('%s' %x)\n",m_strStorage[i],m_strStorage[i]);
            gMemPool->free((MemCell*)m_strStorage[i]);
            m_strStorage.remove(i);
        }
    }

    m_markFlags.changeSize(0);
}
