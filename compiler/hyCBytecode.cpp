/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCBytecode.h"
#include "hyPacking.h"

using namespace Hayat::Common;
using namespace Hayat::Compiler;


Bytecode::Bytecode(void)
    : m_offset(0), m_signatureID(0), m_defValOffs(0), m_numLocalAlloc(0),
      m_jumpTableCodeSize((hyu16)-1), m_jumpTables(0), m_sourceInfos(0)
{
    m_byteCodes.initialize(m_INIT_CAPACITY);
}

Bytecode::~Bytecode()
{
    m_byteCodes.finalize();
    hyu32 n = m_jumpTables.size();
    for (hyu32 i = 0; i < n; i++)
        delete m_jumpTables[i];
}

bool Bytecode::removeLastCode(hyu8 code)
{
    hys32 idx = (hys32) m_byteCodes.size() - 1;
    if (idx < 0)
        return false;
    if (m_byteCodes[idx] == code) {
        m_byteCodes.remove(idx);
        return true;
    }
    return false;
}

void Bytecode::replaceCodes(hyu32 offs, const hyu8* codes, hyu32 size)
{
    m_byteCodes.reserve(offs + size);
    hyu8* p = m_byteCodes.nthAddr(offs);
    memcpy(p, codes, size);
}

hyu8 Bytecode::getCode(hyu32 offs)
{
    HMD_DEBUG_ASSERT((hys32)offs >= 0);
    return m_byteCodes.nth((hys32)offs);
}

const hyu8* Bytecode::getCodes(hyu32 offs)
{
    HMD_DEBUG_ASSERT((hys32)offs >= 0);
    return m_byteCodes.nthAddr((hys32)offs);
}

hyu32 Bytecode::getSize(void)
{
    return m_byteCodes.size();
}
void Bytecode::addJumpTable(TArray<hyu8>* table)
{
    m_jumpTables.add(table);
}

void Bytecode::write(TArray<hyu8>* out)
{
    hyu32 n = m_jumpTables.size();
    HMD_ASSERT(n < 256);        // 今の所 jumptable は最大255個
    hyu16 paddingBytes = (n % 2) * 2;
    hyu16 tblOffs = 4 + (n * 2) + paddingBytes;
    HMD_DEBUG_ASSERT((tblOffs & 3) == 0);
    TArray<hyu8> o;
    for (hyu32 i = 0; i < n; i++) {
        packOut<hyu16>(o, tblOffs);
        tblOffs += m_jumpTables[i]->size();
        HMD_DEBUG_ASSERT((tblOffs & 3) == 0);
    }
    packOut<hyu16>(out, tblOffs);
    packOut<hyu8>(out, n);
    packOut<hyu8>(out, (hyu8)0);
    out->add(o);
    out->align(4, 0xfe);
    for (hyu32 i = 0; i < n; i++) {
        out->add(*(m_jumpTables[i]));
    }
    m_jumpTableCodeSize = tblOffs;
    packOut<hyu16>(out, m_signatureID);
    packOut<hyu16>(out, m_defValOffs);
    packOut<hyu8>(out, m_numLocalAlloc);
    packOut<hyu8>(out, 0xfc);
    packOut<hyu8>(out, 0xfc);
    packOut<hyu8>(out, 0xfc);
    out->add(m_byteCodes);
    out->align(4, 0xfd);
}

const SourceInfo* Bytecode::lastSourceInfo()
{
    hyu32 idx = m_sourceInfos.size();
    if (idx == 0)
        return NULL;
    return m_sourceInfos.values().nthAddr(idx-1);
}

const SourceInfo* Bytecode::getSourceInfoAt(hyu32 offs)
{
    TArray<hyu32>& posarr = m_sourceInfos.keys();
    hyu32 size = posarr.size();
    if (size == 0)
        return NULL;
    int idx;
    binarySearch(posarr.top(), size, offs, &idx);
    return m_sourceInfos.values().nthAddr(idx);
}

SourceInfo* Bytecode::getSourceInfoToAdd()
{
    return &(m_sourceInfos[m_byteCodes.size()]);
}

void Bytecode::fwriteDebugInfo(FILE* fp, TArray<const char*>& paths, hyu32 offs)
{
    HMD_DEBUG_ASSERT(m_jumpTableCodeSize != (hyu16)-1);
    //printf("m_offset=%x,offs=%x : %x\n",m_offset,offs, m_jumpTableCodeSize + 8);
    offs += m_offset + m_jumpTableCodeSize + 8;

    hyu32 n = m_sourceInfos.size();
    hyu32 prevAddr = 0;
    hyu32* addrs = m_sourceInfos.keys().top();
    SourceInfo* infos = m_sourceInfos.values().top();
    hyu8 buf[12];
    buf[10] = 0xf1; buf[11] = 0xf2; // padding

    for (hyu32 i = 0; i < n; i++,++addrs,++infos) {
        // バイトコードアドレス
        hyu32 addr = *addrs + offs;
        HMD_ASSERT(addr > prevAddr || prevAddr == 0);
        prevAddr = addr;
        Endian::pack<hyu32>(&buf[0], addr);

        // パス情報番号
        hys32 pathId = paths.issue(infos->fname);
        HMD_ASSERT(pathId <= 0xffff);
        Endian::pack<hyu16>(&buf[4], (hyu16)pathId);

        // 行
        HMD_ASSERT(infos->line <= 0xffff);
        Endian::pack<hyu16>(&buf[6], (hyu16)infos->line);

        // カラム
        HMD_ASSERT(infos->col <= 0xffff);
        Endian::pack<hyu16>(&buf[8], (hyu16)infos->col);

        fwrite(buf, 12, 1, fp);
    }
}
