/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYARRAY_H_
#define m_HYARRAY_H_

#include "hyTuning.h"
#include "hyMMes.h"
#include "hyBinarySearch.h"
#include <string.h>
#include <stdlib.h>

/*
  要素の型が T であるような配列。

  m_contents は、NULL でなければデータの先頭アドレス。
  m_size は、m_contents の中で有効なデータの個数。
  m_capacity は m_contents のために確保してある容量。
  m_capacity が 0 で m_contents が NULL でない場合、m_contents は外部の
  メモリを指していて、自分でgMemPoolからallocしたものではない事を示す。

  (1) m_capacity == 0 , m_size == 0 , m_contents == NULL
    [未使用状態]
    メモリを確保していない、何のデータも持っていない状態

  (2) m_capacity > 0 , m_size <= m_capacity , m_contents != NULL
    [通常使用状態]
    メモリをgMemPoolから確保してあり、そこに書き込める状態

  (3) m_capacity == 0, m_size > 0 , m_contents != NULL
    [外部参照状態]
    自分で確保したメモリではなく外部のデータを指している状態
    reserve()の呼び出しにより、gMemPoolからメモリを確保し、そこに外部データ
    をコピーして、書き込み可能な(2)の状態に移行する。
    この状態でm_contentsの中身を変更すると、外部のデータが破壊されるので注意！
    reserve()を呼ばずにポインタ・参照を返す関数 nth(), nthAddr(), operator[]
    の取り扱いに注意が必要。

 
  m_contents
  +--------------+------------+
  | T [0]                     | nth(0)
  +--------------+------------+
  | T [1]                     | nth(1)
  +--------------+------------+
  | ....

 */

class Test_Array;       // for unittest
class Test_HSca_Array;   // for unittest

namespace Hayat {
    namespace Common {

        template <typename T> class TArray {

            friend class ::Test_Array;        // for unittest
            friend class ::Test_HSca_Array;   // for unittest

        protected:
            hyu32     m_capacity;
            hyu32     m_size;
            T*      m_contents;

        public:
            static void* operator new(size_t size)
            {
                return (void*) gMemPool->alloc(size, "ARRY");
            }
            static void operator delete(void* p)
            {
                gMemPool->free(p);
            }

            // 未使用状態・通常使用状態コンストラクタ
            // capacity==0 なら未使用状態、capacity>0 なら通常使用状態
            TArray(hyu32 capacity = 0) { initialize(capacity); }

            // 外部参照状態コンストラクタ
            TArray(const T* data, hyu32 numData) { initialize(data, numData); }

            virtual ~TArray() { finalize(); }


            // 未使用状態・通常使用状態で初期化
            // capacity==0 なら未使用状態、capacity>0 なら通常使用状態
            void initialize(hyu32 capacity = 0) {
                m_capacity = capacity;
                if (capacity == 0) {
                    m_contents = NULL;
                } else {
                    m_contents = gMemPool->allocT<T>(capacity, "A_CO");
                }
                m_size = 0;
            }

            // 外部参照状態で初期化
            void initialize(const T* data, hyu32 numData) {
                m_capacity = 0;
                if (numData == 0) {
                    m_contents = NULL;
                    return;
                }
                HMD_DEBUG_ASSERT(data != NULL);
                m_contents = (T*)data;
                m_size = numData;
            }

            void finalize(void) {
                if (m_capacity > 0 && m_contents != NULL)
                    gMemPool->free(m_contents);
                m_contents = NULL;
                m_capacity = 0;
                m_size = 0;
            }

            // 配列のサイズ
            hyu32     size(void) const { return m_size; }

            // 現在の容量
            hyu32     capacity(void) const { return m_capacity; }

            // 残容量
            hyu32     remain(void) { return m_capacity - m_size; }

            // インデックスが配列範囲を越えるマイナス数値でないかチェック
            bool    checkIndex(hys32 idx) { return (idx + (hys32)m_size >= 0); }

            // n番目の要素のアドレス : 範囲チェック無し
            T*      nthAddr(int n) const { return m_contents + n; }
            // 先頭要素のアドレス
            T*      top() const { return m_contents; }

            // idx番目の要素 : 0 が最初の要素
            // マイナスを指定すると後ろから数える : -1 が最後の要素
            T&      nth(hys32 idx) const {
                HMD_ASSERTMSG(idx >= -(hys32)m_size && idx < (hys32)m_size, M_M("Array index %d out of bounds (size=%d)"),idx,m_size);
                if (idx < 0)
                    idx += m_size;
                return m_contents[idx];
            }

            T&      operator[] (hys32 idx) const { return nth(idx); }

            // idxの位置にn個の空間を挿入し、その先頭の参照を返す
            // 挿入した位置の値は初期化していないので注意
            // -size <= idx <= size
            T&      insert(hys32 idx, hyu32 n = 1) {
                HMD_ASSERTMSG(idx >= -(hys32)m_size && idx <= (hys32)m_size, M_M("Array index %d out of bounds (size=%d)"),idx,m_size);
                if (idx < 0)
                    idx += m_size;
                reserve(m_size + n - 1);
                memmove(m_contents+idx+n, m_contents+idx, sizeof(T)*(m_size-idx));
                m_size += n;
                return m_contents[idx];
            }


            // 全要素を x で埋める
            void    fill(const T& x) {
                reserve(0);
                for (int i = m_size-1; i >= 0; --i) {
                    m_contents[i] = x;
                }
            }


            // offsに代入できるように、必要ならメモリを確保する
            void    reserve(hyu32 offs) {
                if (m_contents != NULL && m_capacity == 0) {
                    // 外部参照状態なのでメモリ確保してコピーして通常使用状態になる
                    if (offs < m_size)
                        offs = m_size - 1;
                    m_capacity = (offs + 4) & ~3;
                    T* newmem = gMemPool->allocT<T>(m_capacity, "A_co");
                    memcpy(newmem, m_contents, sizeof(T)*m_size);
                    m_contents = newmem;
                } else if (offs >= m_capacity) {
                    if (m_contents != NULL) {
                        // 通常使用状態
                        while (offs >= m_capacity) {
                            if (m_capacity < ARRAY_CAPACITY_DOUBLE_LIMIT)
                                m_capacity *= 2;
                            else if (m_capacity < ARRAY_CAPACITY_ONE_HALF_LIMIT)
                                m_capacity += (m_capacity >> 1) & ~3;
                            else
                                m_capacity += ARRAY_CAPACITY_INCREMENTS;
                        }
                        m_contents = gMemPool->reallocT<T>(m_contents, m_capacity);
                    } else {
                        // 未使用状態
                        HMD_DEBUG_ASSERT(m_size == 0);
                        m_capacity = (offs + 4) & ~3;
                        m_contents = gMemPool->allocT<T>(m_capacity, "A_CO");
                    }
                }
            }

            // idx番目の要素を x にする ： メモリ不足なら新しいメモリを確保
            // 隙間要素には defaultVal を入れる
            void    subst(hys32 idx, const T& x, const T& defaultVal) {
                HMD_ASSERTMSG(checkIndex(idx),M_M("Array index out of bounds"));
                if (idx < 0)
                    idx += m_size;
                reserve((hyu32)idx);
                if ((hyu32)idx >= m_size) {
                    for (int i = m_size; i < idx; i++) {
                        m_contents[i] = defaultVal;
                    }
                    m_size = idx + 1;
                }
                m_contents[idx] = x;
            }

            // 1個を末尾に追加する
            void    add(const T& val) {
                reserve(m_size);
                nth(m_size++) = val;
            }

            // valuesのsize個の要素を末尾に追加する
            void    add(const T* values, hyu32 size) {
                if (size == 0) return;
                reserve(m_size - 1 + size);
                T* p = &m_contents[m_size];
                for (hyu32 i = 0; i < size; i++) {
                    *p++ = *values++;
                }
                m_size += size;
            }

            // 他の配列の内容を全部追加
            void    add(const TArray<T>& fromArr) {
                hyu32 n = fromArr.size();
                if (n == 0) return;
                reserve(m_size - 1 + n);
                T* p = &m_contents[m_size];
                const T* q = fromArr.top();
                memcpy(p, q, sizeof(T) * n);
                m_size += n;
            }

            // 中身を入れずにsize個の要素を追加し、先頭アドレスを返す
            // バッファを確保してそこに次々と中身を入れていく場合に便利
            // バッファオーバーしないように注意して使用する事
            T*      addSpaces(hyu32 size) {
                HMD_DEBUG_ASSERT(size > 0);
                reserve(m_size - 1 + size);
                T* p = &m_contents[m_size];
                m_size += size;
                return p;
            }

            // サイズを 0 にする
            void    clear(void) {
                if (m_capacity == 0)
                    m_contents = NULL; // 外部参照状態なら未使用状態に移行
                m_size = 0;
            }

            // サイズを切り詰めて末尾を捨てる
            void    chop(hys32 idx) {
                HMD_ASSERTMSG(idx >= -(hys32)m_size && idx <= (hys32)m_size, M_M("Array index %d out of bounds (size=%d)"),idx,m_size);
                if (idx < 0)
                    idx += m_size;
                m_size = idx;
            }

            void    remove(hys32 idx) {
                HMD_ASSERTMSG(idx >= -(hys32)m_size && idx < (hys32)m_size, M_M("Array index %d out of bounds (size=%d)"),idx,m_size);
                if (idx < 0)
                    idx += m_size;
                reserve(0);
                memmove(m_contents+idx, m_contents+idx+1, sizeof(T)*(m_size-idx-1));
                --m_size;
            }

            // 値を取り出して、新しい値に置き換える
            T       replace(hys32 idx, T newVal) {
                reserve(0);
                T& ref = nth(idx);
                T val = ref;
                ref = newVal;
                return val;
            }

            // 指定した値と同じものを全て取り除く
            void    deleteVal(const T& d) {
                reserve(0);
                hys32 n = 0;
                for (hys32 i = 0; i < (hys32)m_size; ++i) {
                    if (m_contents[i] != d) {
                        if (n != i)
                            m_contents[n] = m_contents[i];
                        ++n;
                    }
                }
                m_size = n;
            }

            // 要素を含んでいるか？
            bool    isInclude(const T& val) { return find(val, NULL); }
            bool    find(const T& val, hys32* pidx = NULL) {
                for (hys32 i = (hys32)m_size - 1; i >= 0; --i) {
                    if (val == m_contents[i]) {
                        if (pidx != NULL)
                            *pidx = i;
                        return true;
                    }
                }
                return false;
            }            

            // 同じものがあればそのインデックス、無ければ末尾に追加してそのインデックスを返す
            hys32     issue(T& val) {
                hys32 idx;
                if (! find(val, &idx)) {
                    idx = m_size;
                    add(val);
                }
                return idx;
            }


            // 比較関数compFuncがtrueを返す要素があれば、idxにインデックスを
            // 代入してtrueを返す。 無ければ、idxを変更せずにfalseを返す。
            bool search(const T& val, bool (*compFunc)(const T&, const T&), hyu32& idx)
            {
                for (hyu32 i = 0; i < m_size; i++) {
                    if (compFunc(val, m_contents[i])) {
                        idx = i;
                        return true;
                    }
                }
                return false;
            }

            // 中身が昇順にソートされているとして、2分探索する。
            bool BSearch(const T& val, int* pIdx) const
            {
                return binarySearch<T>(m_contents, m_size, val, pIdx);
            }

            // 中身が昇順にソートされているとして、
            // 同じ要素を探して無ければ適切な場所に挿入する。
            hys32 BInsert(const T& val)
            {
                int idx;
                if (! BSearch(val, &idx)) {
                    insert((hys32)idx) = val;
                }
                return idx;
            }

            // 中身が昇順にソートされているとして、
            // otherの中身全部について、同じ要素を探して
            // 無ければ適切な場所に挿入する。
            void BMerge(const TArray<T>& other)
            {
                int n = other.size();
                const T* p = other.top();
                while (n-- > 0)
                    BInsert(*p++);
            }



            // bytes単位にアライメントする。隙間はpadでパディングする。
            hyu32 align(hys32 bytes, hyu8 pad = (hyu8)0)
            {
                HMD_DEBUG_ASSERT(bytes == 2 || bytes == 4 || bytes == 8 || bytes == 16 || bytes == 32 || bytes == 64 || bytes == 128 || bytes == 256);
                hyu32 gap = (hyu32)((-(hys32)m_size) & (bytes-1));
                if (gap > 0) {
                    hyu8* p = addSpaces(gap);
                    memset(p, pad, gap);
                }
                return gap;
            }

            // クイックソート
            void qsort(int (*compar)(const T*, const T*))
            {
                ::qsort(m_contents, m_size, sizeof(T), (int(*)(const void*,const void*))compar);
            }


#ifdef HMD_DEBUG_MEMORY_ID
            void    setContentsMemID(const char* id) { if(m_contents!=NULL && m_capacity!=0)((MemCell*)m_contents)->setMemID(id); }
#else
            inline void     setContentsMemID(const char*) {}
#endif
        };


        template <typename T> class TArrayIterator {
        protected:
            TArray<T>*      m_arr;
            hys32             m_idx;
        public:
            TArrayIterator(TArray<T>* arr) : m_arr(arr), m_idx(0) {}
            bool hasMore(void) { return (hyu32)m_idx < m_arr->size(); }
            T& next(void) { return m_arr->nth(m_idx++); }
            void rewind(void) { m_idx = 0; }
        };

    }
}

#endif /* m_HYARRAY_H_ */
