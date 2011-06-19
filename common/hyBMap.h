/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYBMAP_H_
#define m_HYBMAP_H_

#include "hyArray.h"
#include "hyBinarySearch.h"

namespace Hayat {
    namespace Common {

        // keyが ==, < 等の比較演算子で比較可能であるような連想配列
        // keyを探す時に二分探索を使用する
        // 挿入・削除する時はメモリ移動が行なわれるので遅い

        template<typename KEY_T, typename VAL_T> class BMap {

        public:
            static void* operator new(size_t size)
            {
                return (void*) gMemPool->alloc(size, "BMAP");
            }
            static void operator delete(void* p)
            {
                gMemPool->free(p);
            }

            static const hyu32 DEFAULT_INIT_CAPACITY = 16;

            BMap(hyu32 capacity = DEFAULT_INIT_CAPACITY)
                : m_keys(capacity), m_values(capacity)
            {
#ifdef HMD_DEBUG_MEMORY_ID
                MemCell* p = (MemCell*) m_keys.top();
                if (p != NULL) p->setMemID("MAPK");
                p = (MemCell*) m_values.top();
                if (p != NULL) p->setMemID("MAPV");
#endif
            }

                ~BMap() { finalize(); }

                void initialize(hyu32 capacity = DEFAULT_INIT_CAPACITY) {
                    if (m_keys.capacity() == 0) {
                        m_keys.initialize(capacity);
                        m_values.initialize(capacity);
                    } else if (m_keys.capacity() < capacity) {
                        m_keys.reserve(capacity);
                        m_values.reserve(capacity);
                    }
#ifdef HMD_DEBUG_MEMORY_ID
                    else
                        return;
                    MemCell* p = (MemCell*) m_keys.top();
                    if (p != NULL) p->setMemID("MAPK");
                    p = (MemCell*) m_values.top();
                    if (p != NULL) p->setMemID("MAPV");
#endif
                }

                void finalize(void) {
                    m_keys.finalize();
                    m_values.finalize();
                }

                hyu32     size(void) const { return m_keys.size(); }

                TArray<KEY_T>&  keys(void) { return m_keys; }
                TArray<VAL_T>&  values(void) { return m_values; }

                VAL_T*  find(const KEY_T& key) const {
                    int idx;
                    if (binarySearch<KEY_T>(m_keys.top(), m_keys.size(), key, &idx))
                        return m_values.nthAddr(idx);
                    return NULL;
                }
        
                // keyに対応した値の参照を返す。
                // keyが存在しなかった場合は、値領域を確保してその参照を返す。その際
                // メモリ移動が起こるので、以前返した参照は無効となるので注意。
                VAL_T&  operator[](const KEY_T key) {
                    int idx;
                    int n = m_keys.size();
                    if (! binarySearch<KEY_T>(m_keys.top(), n, key, &idx)) {
                        // 新規
                        m_keys.insert(idx) = key;
                        m_values.insert(idx);
                    }
                    return m_values[idx];
                }

                // keyの重複を許容する追加。
                // 重複があったあ場合、同じkeyの最後の位置に追加する。
                void    forceAdd(const KEY_T key, const VAL_T val) {
                    int idxMin, idxMax;
                    if (binarySearchRange<KEY_T>(m_keys.top(), m_keys.size(), key, &idxMin, &idxMax)) {
                        ++ idxMax;
                    }
                    m_keys.insert(idxMax) = key;
                    m_values.insert(idxMax) = val;
                }


                bool    remove(const KEY_T key) {
                    int idx;
                    if (binarySearch<KEY_T>(m_keys.top(), m_keys.size(), key, &idx)) {
                        m_keys.remove(idx);
                        m_values.remove(idx);
                        return true;
                    }
                    return false;
                }

                void    copyFrom(BMap<KEY_T,VAL_T>& other) {
                    hyu32 size = other.size();
                    TArray<KEY_T>& okeys = other.keys();
                    TArray<VAL_T>& ovalues = other.values();
                    for (hyu32 i = 0; i < size; ++i) {
                        (*this)[okeys[i]] = ovalues[i];
                    }
                }
                
        protected:
                TArray<KEY_T>   m_keys;
                TArray<VAL_T>   m_values;
        };

    }
}

#endif /* m_HYBMAP_H_ */
