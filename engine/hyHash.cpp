/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyHash.h"
#include "hyThread.h"
#include <string.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;


void* Hash::operator new(size_t size)
{
#ifndef HMD_DEBUG
    (void)size;
#endif
    HMD_DEBUG_ASSERT(size == sizeof(Hash));
    Object* obj = Object::create(HC_Hash, sizeof(Hash) + sizeof(hyu32));
    return (void*) obj->cppObj<Hash>();
}

void Hash::operator delete(void* p)
{
    Object::fromCppObj(p)->destroy();
}

void Hash::initialize(hys32 initCapacity, hyu32 loadFactor)
{
    m_load = 0;
    m_loadFactor = loadFactor;
    m_bucketSize = initCapacity;
    m_buckets = gMemPool->allocT<HashCell_t*>(initCapacity, "HASH");
    memset(m_buckets, 0, sizeof(HashCell_t*) * initCapacity);
}

void Hash::finalize(void)
{
    clear();
    gMemPool->free(m_buckets);
    m_buckets = NULL;
    m_bucketSize = 0;
}


// 要素数を返す
hys32 Hash::size(void)
{
    HMD_DEBUG_ASSERT(getObj()->type()->symCheck(HSym_Hash));
    Iterator itr(this);
    hys32 count = 0;
    while (itr.next() != NULL)
        ++count;
    return count;
}


Hash::HashCell_t::Iterator Hash::m_search(Context* context, Value key, hyu32 hashCode)
{
    HMD_DEBUG_ASSERT(getObj()->type()->symCheck(HSym_Hash));
    hyu32 idx = hashCode % m_bucketSize;
    // HMD__PRINTF_FK("key %d ; idx = %d\n", key.data, idx);
    HashCell_t::Iterator cellItr(&m_buckets[idx]);
    while (! cellItr.empty()) {
        HashPair_t& pair = cellItr.get()->content();
        if (hashCode == pair.hashCode)
            if (key.equals(context, pair.key))
                break;
        cellItr.next();
    }
    return cellItr;
}



// keyに対してvalを関連付ける。
void Hash::put(Context* context, Value key, Value val)
{
    hyu32 hashCode = key.hashCode(context);
    HashCell_t::Iterator cellItr = m_search(context, key, hashCode);
    if (cellItr.empty()) {
        HashPair_t pair = {hashCode, key, val};
        cellItr.insertPrev(pair);
        if (++m_load >= m_loadFactor * m_bucketSize / 100) {
            // HMD__PRINTF_FK("rehash by put\n");
            rehash(context);
        }
    } else {
        Value old = cellItr.get()->content().val;
        // HMD__PRINTF_FK("replace old=%d\n",old.data);
        cellItr.get()->content().val = val;
    }
}

// keyに関連付けられた値を返す
Value Hash::get(Context* context, Value key)
{
    HashCell_t::Iterator cellItr = m_search(context, key);
    if (cellItr.empty()) {
        return NIL_VALUE;
    } else {
        return cellItr.get()->content().val;
    }
}
    

// keyに関連する要素を削除。削除された値を返す
Value Hash::remove(Context* context, Value key)
{
    HashCell_t::Iterator cellItr = m_search(context, key);
    if (cellItr.empty()) {
        return NIL_VALUE;
    } else {
        HashCell_t* p = cellItr.remove();
        Value oldVal = p->content().val;
        delete p;
        return oldVal;
    }
}


// 中身を空にする
void Hash::clear(void)
{
    HMD_DEBUG_ASSERT(getObj()->type()->symCheck(HSym_Hash));
    for (hyu32 i = 0; i < m_bucketSize; i++) {
        HashCell_t::deleteAll(m_buckets[i]);
        m_buckets[i] = NULL;
    }
    m_load = 0;
}

// 中身を全部コピーした別ハッシュを作成する
Hash* Hash::clone(Context* context)
{
    HMD_DEBUG_ASSERT(getObj()->type()->symCheck(HSym_Hash));
    Hash* dest = new Hash(m_bucketSize, m_loadFactor);
    Iterator itr(this);
    for (HashCell_t* pCell = itr.next(); pCell != NULL; pCell = itr.next()) {
        dest->put(context, pCell->content().key, pCell->content().val);
    }
    return dest;
}


// keyをキーとして持つかどうか
bool Hash::hasKey(Context* context, Value key)
{
    HashCell_t::Iterator itr = m_search(context, key);
    return ! itr.empty();
}


// 全てのキーを配列に入れて返す
ValueArray* Hash::keys(void)
{
    HMD_DEBUG_ASSERT(getObj()->type()->symCheck(HSym_Hash));
    ValueArray* arr = new ValueArray(size());
    hys32 i = 0;
    Iterator itr(this);
    for (HashCell_t* pCell = itr.next(); pCell != NULL; pCell = itr.next()) {
        arr->subst(i++, pCell->content().key, NIL_VALUE);
    }
    return arr;
}


// 全ての値を配列に入れて返す
ValueArray* Hash::values(void)
{
    HMD_DEBUG_ASSERT(getObj()->type()->symCheck(HSym_Hash));
    ValueArray* arr = new ValueArray(size());
    hys32 i = 0;
    Iterator itr(this);
    for (HashCell_t* pCell = itr.next(); pCell != NULL; pCell = itr.next()) {
        arr->subst(i++, pCell->content().val, NIL_VALUE);
    }
    return arr;
}

// 全キーのハッシュ値を再計算して再配置する。
// もし m_load が capacity() * m_loadFactor / 100 を越えていたら、
// capacityを増加させたハッシュを作成してそちらに移動する
void Hash::rehash(Context* context)
{
    HMD_DEBUG_ASSERT(getObj()->type()->symCheck(HSym_Hash));
    if (m_load >= m_loadFactor * m_bucketSize / 100) {
        // 容量を増やしたハッシュに全コピー
        size_t newBucketSize = m_bucketSize * 2 + 1;
        while (m_load >= m_loadFactor * newBucketSize / 100) {
            newBucketSize = newBucketSize * 2 + 1;
        }
        HashCell_t** newBuckets = gMemPool->allocT<HashCell_t*>(newBucketSize, "Hash");
        memset(newBuckets, 0, sizeof(HashCell_t*) * newBucketSize);
        m_load = 0;

        for (hyu32 i = 0; i < m_bucketSize; i++) {
            HashCell_t::Iterator srcItr(&m_buckets[i]);
            HashCell_t* hc;
            while ((hc = srcItr.remove()) != NULL) {
                HashPair_t& pair = hc->content();
                hyu32 hashCode = pair.key.hashCode(context);
                pair.hashCode = hashCode;
                hyu32 newIdx = hashCode % newBucketSize;
                HashCell_t::Iterator dstItr(&newBuckets[newIdx]);
                if (dstItr.get() == NULL)
                    ++m_load;
                dstItr.insertPrev(hc);
            }
        }
        
        gMemPool->free(m_buckets); // 元のメモリを解放
        m_buckets = newBuckets;
        m_bucketSize = newBucketSize;

    } else {
        // 再計算して値が違ったものだけ再配置
        // HMD__PRINTF_FK("rehash %x relocation\n", this);
        for (hyu32 i = 0; i < m_bucketSize; i++) {
            HashCell_t::Iterator itr(&m_buckets[i]);
            while (! itr.empty()) {
                HashPair_t& pair = itr.get()->content();
                hyu32 hashCode = pair.key.hashCode(context);
                if (pair.hashCode != hashCode) {
                    pair.hashCode = hashCode;
                    hyu32 idx = hashCode % m_bucketSize;
                    if (i != idx) {
                        // 再配置
                        HashCell_t* cell = itr.remove();
                        cell->link(m_buckets[idx]);
                        m_buckets[idx] = cell;
                        continue;
                    }
                }
                itr.next();
            }
        }
    }
}


hyu32 Hash::calcHashCode(void)
{
    Iterator itr(this);
    hyu32 h = 0;
    for (HashCell_t* pCell = itr.next(); pCell != NULL; pCell = itr.next()) {
        h = h * 0x1f1f1f1f + pCell->content().hashCode;
    }
    return h;
}

bool Hash::equals(Context* context, Hash* o)
{
    if (size() != o->size())
        return false;
    Iterator itr(this);
    for (HashCell_t* pCell = itr.next(); pCell != NULL; pCell = itr.next()) {
        HashPair_t& pair = pCell->content();
        Value v = o->get(context, pair.key);
        if (! pair.val.equals(context, v))
            return false;
    }
    return true;
}

void Hash::m_GC_mark(void)
{
    Iterator itr(this);
    for (HashCell_t* pCell = itr.next(); pCell != NULL; pCell = itr.next()) {
        HashPair_t& pair = pCell->content();
        GC::markValue(pair.key);
        GC::markValue(pair.val);
    }    
}


Hash::Iterator::Iterator(Hash* pHash)
{
    m_pHash = pHash;
    m_idx = -1;
}

Hash::HashCell_t* Hash::Iterator::next(void)
{
    if (m_idx < 0) {
        m_idx = 0;
        m_cellItr.initialize(m_pHash->m_buckets);
    }
    while (m_cellItr.empty()) {
        if ((hyu32)++m_idx >= m_pHash->m_bucketSize)
            return NULL;
        m_cellItr.initialize(&(m_pHash->m_buckets[m_idx]));
    }
    HashCell_t* cell = m_cellItr.get();
    m_cellItr.next();
    // HMD__PRINTF_FK("#%d# ", cell->content().key.data);
    return cell;
}
