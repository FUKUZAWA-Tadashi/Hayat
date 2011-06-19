/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYHASH_H_
#define m_HYHASH_H_

#include "hyCellList.h"
#include "hyValueArray.h"
#include "hyTuning.h"

/*

  this
  +----------------------
  | hyu32 m_load
  +----------------------
  | hyu32 m_loadFactor
  +----------------------
  | hyu32 m_bucketSize
  +----------------------
  | HashCell_t** m_buckets
  +----------------------


  m_buckets
  +------------------------
  | HashCell_t* m_buckets[0]
  +
  | HashCell_t* m_buckets[1]
  +
  | ....


  m_buckets[n] に、n = key.hashCode() % m_bucketSize とするような
  {key,val}ペアのCellList が入る。

*/


class Test_hyHash;

namespace Hayat {
    namespace Engine {

        class Context;

        class Hash {
            friend class ::Test_hyHash;
            friend class HSca_Hash;
            friend class GC;

        public:
            typedef struct { hyu32 hashCode; Value key; Value val; } HashPair_t;
            typedef CellList<HashPair_t> HashCell_t;

        protected:
            hyu32             m_load;
            hyu32             m_loadFactor;    // パーセント
            hyu32             m_bucketSize;
            HashCell_t**    m_buckets;       // HashCell_t* の配列へのポインタ

        public:

            static void* operator new(size_t);
            static void operator delete(void*);
            Hash(hyu32 initCapacity = HASH_DEFAULT_INIT_CAPACITY, hyu32 loadFactor = HASH_DEFAULT_LOAD_FACTOR) {
                initialize(initCapacity, loadFactor);
            }
            ~Hash() { finalize(); }


            hyu32 capacity() { return m_bucketSize; }

            void    initialize(hys32 initCapacity = HASH_DEFAULT_INIT_CAPACITY, hyu32 loadFactor = HASH_DEFAULT_LOAD_FACTOR);
            void    finalize(void);

            // 要素数を返す
            hys32     size(void);
            // このハッシュを所持するObject
            Object* getObj(void) { return Object::fromCppObj(this); }

            // keyに対してvalを関連付ける。
            void    put(Context* context, Value key, Value val);
            // keyに関連付けられた値を返す
            Value   get(Context* context, Value key);
            // 中身を空にする
            void    clear(void);
            // 中身を全部コピーした別ハッシュを作成する
            Hash*   clone(Context* context);
            // keyに関連する要素を削除。削除された値を返す
            Value   remove(Context* context, Value key);
            // keyをキーとして持つかどうか
            bool    hasKey(Context* context, Value key);
            // 全てのキーを配列に入れて返す
            ValueArray*     keys(void);
            // 全ての値を配列に入れて返す
            ValueArray*     values(void);

            // キーのハッシュ値を再計算する。sizeが capacity() * m_loadFactor/100 を
            // 越えていたら、capacityを増加させる
            void    rehash(Context* context);

            // 等しいかどうか
            bool    equals(Context* context, Hash* o);

            // 自分自身のハッシュ値を計算
            // HashPair_t::hashCode を使うので、rehash()すると違う値になる可能性がある
            hyu32     calcHashCode(void);

        public:
            void    m_GC_mark(void);


        public:
            friend class Iterator;
            class Iterator {
            protected:
                Hash* m_pHash;
                hys32 m_idx;
                HashCell_t::Iterator m_cellItr;
            public:
                Iterator(Hash*);
                HashCell_t* next(void);
                hys32         index(void) { return m_idx; }
            };

            // キーがkeyであるCellのアドレスを示すイテレータを返す
            // hashCodeを複数回再計算すると無駄なので、計算済みhashCodeを渡す
            HashCell_t::Iterator m_search(Context* context, Value key, hyu32 hashCode);
            // キーがkeyであるCellのアドレスを示すイテレータを返す
            // hashCodeは内部で計算する
            HashCell_t::Iterator m_search(Context* context, Value key) { return m_search(context, key, key.hashCode(context)); }

        };

    }
}
#endif /* m_HYHASH_H_ */
