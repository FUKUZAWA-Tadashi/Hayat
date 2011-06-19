/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCStrTable.h"
#include "hyPacking.h"
#include "hyCFileOut.h"
#include "hyMemPool.h"
#include "hyEndian.h"
#include "mbcsconv.h"
#include <stdio.h>
#include "hyCCompileError.h"


using namespace Hayat::Common;
using namespace Hayat::Compiler;
using namespace Hayat::Parser;



#ifndef WIN32
// gccではこれが必要…
const hyu32 StrTable::NOT_FOUND;// = (hyu32)-1;
const hyu16 StrTable::NOT_FOUND_ID;// = (hyu16)-1;
#endif



StrTable::StrTable(void)
    : m_numStr(0), m_strOffsSize(0), m_strBufSize(0)
{
}

StrTable::~StrTable()
{
    finalize();
}

void StrTable::initialize(hyu32 initStrOffsSize, hyu32 initStrBufSize)
{
    HMD_DEBUG_ASSERT(initStrOffsSize > 0);
    HMD_DEBUG_ASSERT(initStrBufSize > 0);
    HMD_DEBUG_ASSERT(m_strOffsSize == 0);
    HMD_DEBUG_ASSERT(m_strBufSize == 0);
    m_numStr = 0;
    m_strOffs = gMemPool->allocT<hyu32>(initStrOffsSize);
    HMD_ASSERT(m_strOffs != NULL);
    m_strOffsSize = initStrOffsSize;
    m_strOffs[0] = 0;
    m_strBuf = gMemPool->allocT<char>(initStrBufSize);
    HMD_ASSERT(m_strBuf != NULL);
    m_strBufSize = initStrBufSize;
}

void StrTable::finalize(void)
{
    if (m_strOffsSize > 0) {
        gMemPool->free(m_strOffs);
        m_strOffsSize = 0;
    }
    if (m_strBufSize > 0) {
        gMemPool->free(m_strBuf);
        m_strBufSize = 0;
    }
}

// 重複チェックして(もし必要なら文字列を格納し)そのオフセットを返す
hyu32 StrTable::getOffs(const char* str, hyu32 len)
{
    hyu32 offs = checkOffs(str, len);
    if (offs == NOT_FOUND)
        offs = addStr(str, len);
    return offs;
}
// 重複チェックして(もし必要ならバイト列を格納し)その番号返す
hyu16 StrTable::getIdBytes(const hyu8* bp, hyu32 len)
{
    hyu16 id = checkIdBytes(bp, len);
    if (id == NOT_FOUND_ID)
        id = addBytes(bp, len);
    return id;
}


void StrTable::m_reserve(hyu32 len)
{
    if (m_strOffsSize == 0 && m_strBufSize == 0) {
        initialize();
    }

    hyu32 lastOffs = m_strOffs[m_numStr];
    if (lastOffs + len >= m_strBufSize) {
        HMD__PRINTF_MEM("StrTable: realloc m_strBuf(%x) size(%x)\n", m_strBuf, m_strBufSize*sizeof(char));
        while (lastOffs + len >= m_strBufSize)
            m_strBufSize *= 2;
        m_strBuf = gMemPool->reallocT<char>(m_strBuf, m_strBufSize);
        HMD__PRINTF_MEM("             -->     m_strBuf(%x) size(%x)\n", m_strBuf, m_strBufSize*sizeof(char));
        HMD_ASSERT(m_strBuf != NULL);
    }
}

// 重複チェックせずに文字列を格納し、そのオフセットを返す
hyu32 StrTable::addStr(const char* str, hyu32 len)
{
    if (len == 0)
        len = HMD_STRLEN(str) + 1;
    m_reserve(len);
    hyu32 lastOffs = m_strOffs[m_numStr];
    HMD_MEMCPY(m_strBuf + lastOffs, str, len);
    m_expandStrOffs();
    m_strOffs[++m_numStr] = lastOffs + len;
    return lastOffs;
}

// 重複チェックせずにバイト列を格納し、その番号を返す
hyu16 StrTable::addBytes(const hyu8* bp, hyu32 len)
{
    HMD_ASSERT(m_numStr < 0xffff);
    hyu16 id = (hyu16)m_numStr;
    m_reserve(len);
    hyu32 lastOffs = m_strOffs[m_numStr];
    memcpy(m_strBuf + lastOffs, bp, len);
    m_expandStrOffs();
    m_strOffs[++m_numStr] = lastOffs + len;
    return id;
}

void StrTable::m_expandStrOffs(void)
{
    if (m_numStr+1 >= m_strOffsSize) {
        HMD__PRINTF_MEM("StrTable: realloc m_strOffs(%x) size(%x)\n", m_strOffs, m_strOffsSize*sizeof(hyu32));
        while (m_numStr+1 >= m_strOffsSize)
            m_strOffsSize *= 2;
        m_strOffs = gMemPool->reallocT<hyu32>(m_strOffs, m_strOffsSize);
        HMD__PRINTF_MEM("             -->     m_strOffs(%x) size(%x)\n", m_strOffs, m_strOffsSize*sizeof(hyu32));
        HMD_ASSERT(m_strOffs != NULL);
    }
}


// strと同じ文字列が既に格納されていたら、そのオフセットを返す
// 格納されていなかったら、NOT_FOUND を返す
hyu32 StrTable::checkOffs(const char* str, hyu32 len)
{
    if (len == 0)
        len = HMD_STRLEN(str) + 1;
    hyu16 i = checkIdBytes((const hyu8*)str, len);
    if (i == NOT_FOUND_ID)
        return NOT_FOUND;
    return m_strOffs[i];
}
// bpと同じバイト列が既に格納されていたら、その番号を返す
// 格納されていなかったら、NOT_FOUND_ID を返す
hyu16 StrTable::checkIdBytes(const hyu8* bp, hyu32 len)
{
    hyu32 offs = 0; //m_strOffs[0]
    for (hyu32 i = 1; i <= m_numStr; i++) {
        hyu32 nextOffs = m_strOffs[i];
        if (nextOffs - offs == len)
            if (0 == HMD_MEMCMP(bp, m_strBuf + offs, len))
                return (hyu16)(i-1);
        offs = nextOffs;
    }
    return NOT_FOUND_ID;
}

// オフセットに対応する文字列を返す
const char* StrTable::getStr(hyu32 offset)
{
    if (offset >= m_strOffs[m_numStr])
        return NULL;
    HMD_DEBUG_ASSERT(offset == 0 || m_strBuf[offset-1] == '\0');
    return m_strBuf + offset;
}
// 番号に対応するバイト列を返す
const hyu8* StrTable::getBytes(hyu16 id, hyu32* pLen)
{
    if (id >= m_numStr)
        return NULL;
    hyu32 offs = m_strOffs[id];
    if (pLen != NULL)
        *pLen = m_strOffs[id + 1] - offs;
    return (const hyu8*)(m_strBuf + offs);
}


// FILE* からテーブルを読み込む
void StrTable::readFile(FILE* fp)
{
    hyu8 buf[4];
    size_t nread = fread(buf, 1, 4, fp);
    HMD_ASSERT(nread == 4);
    m_strBufSize = Endian::unpack<hyu32>(buf);
    if (m_strBufSize == 0) {
        m_strBufSize = m_INIT_STR_BUF_SIZE;
        m_strBuf = gMemPool->allocT<char>(m_strBufSize);
        m_strOffsSize = m_INIT_STR_OFFS_SIZE;
        m_strOffs = gMemPool->allocT<hyu32>(m_strOffsSize);
        m_strOffs[0] = 0;
        m_numStr = 0;
        return;
    }
    m_strBuf = gMemPool->allocT<char>(m_strBufSize);
    nread = fread(m_strBuf, 1, m_strBufSize, fp);
    HMD_ASSERT(nread == m_strBufSize);
    hyu32 odd = m_strBufSize % 4;
    if (odd != 0) {
        // padding読み飛ばし
        nread = fread(buf, 1, 4 - odd, fp);
        HMD_ASSERT(nread == 4 - odd);
    }

    m_strOffsSize = m_INIT_STR_OFFS_SIZE;
    m_strOffs = gMemPool->allocT<hyu32>(m_strOffsSize);
    m_strOffs[0] = 0;
    m_numStr = 0;
    hyu32 o = 0;
    while (o < m_strBufSize) {
        m_expandStrOffs();
        o += HMD_STRLEN(m_strBuf + o) + 1;
        m_strOffs[++m_numStr] = o;
    }

}

// テーブルをFILE*に出力
void StrTable::writeFile(FILE* fp)
{
    hyu8 buf[4];
    hyu32 size = m_strOffs[m_numStr];
    Endian::pack<hyu32>(buf, size);
    size_t nwrite = fwrite(buf, 1, 4, fp);
    HMD_ASSERT(nwrite == 4);
    nwrite = fwrite(m_strBuf, 1, size, fp);
    HMD_ASSERT(nwrite == size);
    hyu32 odd = size % 4;
    if (odd != 0) {
        // padding書き込み
        memset(buf, 0, 4);
        nwrite = fwrite(buf, 1, 4 - odd, fp);
        HMD_ASSERT(nwrite == 4 - odd);
    }
}

// バイト列テーブルを出力
void StrTable::writeBytes(TArray<hyu8>* out)
{
    packOut<hyu32>(out, m_numStr);
    if (m_numStr == 0) {
        packOut<hyu32>(out, 0);
        return;
    }
    for (hyu32 i = 0; i <= m_numStr; ++i) {
        packOut<hyu32>(out, m_strOffs[i]);
    }
    hyu32 size = m_strOffs[m_numStr];
    memcpy(out->addSpaces(size), m_strBuf, size);
    out->align(4, 0);
}


// バイト列テーブルを読み込む
void StrTable::readBytes(const hyu8** pinp)
{
    m_numStr = Endian::unpackP<hyu32>(pinp);
    if (m_numStr == 0) {
        hyu32 s = Endian::unpackP<hyu32>(pinp);
        HMD_ASSERT(s == 0);
        return;
    }
    if (m_strOffsSize < m_numStr + 1) {
        m_strOffsSize = m_numStr + 1;
        m_strOffs = gMemPool->reallocT<hyu32>(m_strOffs, m_strOffsSize);
    }
    hyu32* p = m_strOffs;
    for (hyu32 i = 0; i <= m_numStr; ++i) {
        *p++ = Endian::unpackP<hyu32>(pinp);
    }
    hyu32 size = m_strOffs[m_numStr];
    if (m_strBufSize < size) {
        m_strBufSize = size;
        m_strBuf = gMemPool->reallocT<char>(m_strBuf, size);
    }
    memcpy(m_strBuf, *pinp, size);
    *pinp += size + ((-(hys32)size) & 3);
}

#ifdef HMD_DEBUG
void StrTable::dumpByteTable(void)
{
    hyu32 offs = 0;
    for (hyu32 i = 1; i <= m_numStr; ++i) {
        HMD_PRINTF("%2d: %08x ", i-1, m_strBuf + m_strOffs[i-1]);
        while (offs < m_strOffs[i])
            HMD_PRINTF(" %02x", (hyu8)m_strBuf[offs++]);
        HMD_PRINTF("\n");
    }
}
#endif
