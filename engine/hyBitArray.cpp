/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyBitArray.h"
#include "hyMemPool.h"
#include <string.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;



static const hys32 MB = sizeof(BitArray::MemAllocType_t);
static const hys32 MS = 8 * MB;
static const hys32 MF = MS - 1;
static const hys32 MSB = 1 << MF;
static const BitArray::MemAllocType_t FULL_BIT = (BitArray::MemAllocType_t)0xffffffff;
static inline const BitArray::MemAllocType_t BIT_MASK(int i) {
    return 1 << i;
}
static inline const BitArray::MemAllocType_t LOW_MASK(int i) {
    return FULL_BIT >> (MF - i);
}
static inline const BitArray::MemAllocType_t HI_MASK(int i) {
    return FULL_BIT << i;
}
static inline const BitArray::MemAllocType_t SHIFT_HI(BitArray::MemAllocType_t x) {
    return x << 1;
}
static inline void SHIFT_REF_HI(BitArray::MemAllocType_t& ref) {
    ref <<= 1;
}


void* BitArray::operator new(size_t size, void* addr)
{
    HMD_DEBUG_ASSERT(size == sizeof(BitArray));
    return addr;
}

BitArray::BitArray(hys32 size)
    : m_size(size)
{
    HMD_ASSERT(size >= 0);
    m_memSize = (m_size + MF) / MS;
    if (m_memSize > 0) {
        m_memory = gMemPool->allocT<MemAllocType_t>(m_memSize, "BTAR");
        setAll(false);
    } else {
        m_memory = NULL;
    }
}

BitArray::BitArray(const BitArray& other)
{
    m_size = other.m_size;
    m_memSize = (m_size + MF) / MS;
    if (m_memSize > 0) {
        m_memory = gMemPool->allocT<MemAllocType_t>(m_memSize, "BTAR");
        memcpy(m_memory, other.m_memory, MB * m_memSize);
    } else {
        m_memory = NULL;
    }
}

void BitArray::finalize()
{
    gMemPool->free(m_memory);
    m_memory = NULL;
    m_memSize = 0;
    m_size = 0;
}


bool BitArray::getAt(hys32 idx)
{
    if (idx < 0 || idx >= m_size) return false;
    return (m_memory[idx / MS] & BIT_MASK(idx % MS)) != 0;
}

void BitArray::setAt(hys32 idx, bool x)
{
    if (idx < 0 || idx >= m_size) return;
    if (x)
        m_memory[idx / MS] |= BIT_MASK(idx % MS);
    else
        m_memory[idx / MS] &= ~BIT_MASK(idx % MS);
}

void BitArray::insertAt(hys32 idx, bool x)
{
    changeSize(m_size + 1);
    hys32 n = idx / MS;
    hys32 b = idx % MS;
    MemAllocType_t m = m_memory[n];
    MemAllocType_t m1 = (b == 0) ? (MemAllocType_t)0 : (m & LOW_MASK(b-1));
    MemAllocType_t m2 = (b == MF) ? (MemAllocType_t)0 : (SHIFT_HI(m) & HI_MASK(b+1));
    MemAllocType_t mx = x ? BIT_MASK(b) : 0;
    m_memory[n] = m1 | mx | m2;

    MemAllocType_t msb = m & MSB;
    while (++n < m_memSize) {
        MemAllocType_t& mref = m_memory[n];
        MemAllocType_t nextMsb = mref & MSB;
        SHIFT_REF_HI(mref);
        if (msb != 0)
            mref |= 1;
        msb = nextMsb;
    }

}


void BitArray::setAll(bool x)
{
    if (m_size <= 0)
        return;
    hys32 j = m_size - 1;
    hys32 n = j / MS;
    if (x) {
        m_memory[n] = LOW_MASK(j % MS);
        if (n > 0)
            memset(m_memory, FULL_BIT, n * MB);
    } else {
        memset(m_memory, 0, ++n * MB);
    }
}

bool BitArray::setRange(hys32 idx1, hys32 idx2, bool x)
{
    if (m_size <= 0)
        return false;
    if (idx1 < 0 || idx1 >= m_size) return false; // out of array
    if (idx2 < 0 || idx2 >= m_size) return false; // out of array
    if (idx1 > idx2) {
        hys32 tmp = idx1; idx1 = idx2; idx2 = tmp;
    }
    hys32 n1 = idx1 / MS;
    MemAllocType_t mask1 = HI_MASK(idx1 % MS);
    hys32 n2 = idx2 / MS;
    MemAllocType_t mask2 = LOW_MASK(idx2 % MS);
    if (n1 == n2) {
        if (x) {
            m_memory[n1] |= (mask1 & mask2);
        } else {
            m_memory[n1] &= ~(mask1 & mask2);
        }
    } else {
        if (x) {
            m_memory[n1] |= mask1;
            m_memory[n2] |= mask2;
            if (++n1 < n2)
                memset(&m_memory[n1], FULL_BIT, (n2-n1) * MB);
        } else {
            m_memory[n1] &= ~mask1;
            m_memory[n2] &= ~mask2;
            if (++n1 < n2)
                memset(&m_memory[n1], 0, (n2-n1) * MB);
        }
    }
    return true;
}


void BitArray::changeSize(hys32 newSize)
{
    HMD_DEBUG_ASSERT(newSize >= 0);
    if (newSize == m_size)
        return;
    if (newSize == 0) {
        finalize();
        return;
    }
    if (newSize < m_size) {
        setRange(newSize, m_size - 1, false); // 減少分クリア
        m_size = newSize;
    } else {
        hys32 newMemSize = (newSize + MF) / MS;
        if (newMemSize > m_memSize) {
            m_memory = gMemPool->reallocT<MemAllocType_t>(m_memory, newMemSize);
            m_memSize = newMemSize;
        }
        hys32 oldSize = m_size;
        m_size = newSize;
        setRange(oldSize, m_size - 1, false); // 増加分クリア
    }
}


bool BitArray::isSame(const BitArray& other)
{
    if (m_size != other.m_size)
        return false;
    if (m_size <= 0)
        return true;
    hys32 n = (m_size - 1) / MS + 1;
    while (--n >= 0) {
        if (m_memory[n] != other.m_memory[n])
            return false;
    }
    return true;
}

hys32 BitArray::hashCode(void)
{
    hyu32 hash = m_size;
    hyu8* p = (hyu8*)m_memory;
    for (hys32 i = (m_size - 1) / 8; i >= 0; --i) {
        hash *= 0x1f1f1f1f;
        hash += *p++;
    }
    return (hys32)hash;
}
