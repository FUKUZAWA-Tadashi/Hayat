/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyMMes.h"
#include "hyEndian.h"
#include "hyBinarySearch.h"

using namespace Hayat::Common;

namespace Hayat {
    namespace Common {
        MMes::mm_pair_st*       MMes::m_mes_tbl;
        int                     MMes::m_tbl_size;
        hyu8*                   MMes::m_fileBuf;
        MemPool*                MMes::m_fileBuf_pool;
    }
}

void MMes::initialize(void)
{
    m_mes_tbl = NULL;
    m_tbl_size = 0;
    m_fileBuf = NULL;
    m_fileBuf_pool = NULL;
}

void MMes::finalize(void)
{
    if (m_fileBuf != NULL)
        hmd_freeFileBuf(m_fileBuf, m_fileBuf_pool);
    m_fileBuf = NULL;
    m_fileBuf_pool = NULL;
    m_mes_tbl = NULL;
    m_tbl_size = 0;
}


int MMes::m_cmpfn(MMes::mm_pair_st& ent, const char*& v)
{
    return HMD_STRCMP(ent.orig, v);
}
const char* MMes::getMMes(const char* mes)
{
    int idx;
#if 1
    if (binarySearchFn<mm_pair_st, const char*>(m_mes_tbl, m_tbl_size, m_cmpfn, mes, &idx)) {
        return m_mes_tbl[idx].trans;
    }
#else
    for (idx = 0; idx < m_tbl_size; ++idx){
        if (0==HMD_STRCMP(m_mes_tbl[idx].orig, mes)) {
            //printf(";%s; -> ;%s;\n",mes,m_mes_tbl[idx].trans);
            return m_mes_tbl[idx].trans;
        }
    }
#endif
    return mes;
}

void MMes::readTable(const char* file, MemPool* pool)
{
    if (file == NULL)
        return;
    hyu32 size = hmd_loadFile_inPath(file, &m_fileBuf, NULL, pool);
    if (size > 0) {
        m_fileBuf_pool = pool;
        if (! takeTable((const char*)m_fileBuf, (int)size))
            HMD_PRINTF("message file '%s' format error : ignored\n", file);
    }
}

static const char* s_skipstr(const char* p)
{
    while (*p++ != '\0')
        ;
    return p;
}
bool MMes::takeTable(const char* buf, int bufSize)
{
    if (bufSize > 4 && buf[0] == 'M' && buf[1] == 'm') {
        m_tbl_size = Endian::unpackE<Endian::BIG, hyu16>((const hyu8*)(buf+2));
        if ((size_t)bufSize >= m_tbl_size * sizeof(mm_pair_st) + 4) {
            m_mes_tbl = (mm_pair_st*)(buf + 4);
            const char* p = buf + m_tbl_size * sizeof(mm_pair_st) + 4;
            int i;
            for (i = 0; i < m_tbl_size; ++i) {
                const char* q = s_skipstr(p);
                if (q > buf + bufSize) break;
                m_mes_tbl[i].orig = p;
                p = s_skipstr(q);
                if (p > buf + bufSize) break;
                m_mes_tbl[i].trans = q;
            }
            if (i == m_tbl_size) {
                const char* q = buf + bufSize;
                if (p <= q) {
                    int d = q - p;
                    if (d <= 1) { // Windows‚Ìê‡ EOF‚Ì ^Z ‚ª‚ ‚è‚¤‚é
                        // all OK !
                        return true;
                    }
                }
            }
            HMD_PRINTF("i=%x,m_tbl_size=%x,p=%x,buf+bufSize=%x\n",i,m_tbl_size,p,buf+bufSize);
        }
        else HMD_PRINTF("bufSize=%x,m_tbl_size=%x\n",bufSize,m_tbl_size);

    }
    // something wrong
    finalize();
    return false;
}
