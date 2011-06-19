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


  m_buckets[n] �ɁAn = key.hashCode() % m_bucketSize �Ƃ���悤��
  {key,val}�y�A��CellList ������B

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
            hyu32             m_loadFactor;    // �p�[�Z���g
            hyu32             m_bucketSize;
            HashCell_t**    m_buckets;       // HashCell_t* �̔z��ւ̃|�C���^

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

            // �v�f����Ԃ�
            hys32     size(void);
            // ���̃n�b�V������������Object
            Object* getObj(void) { return Object::fromCppObj(this); }

            // key�ɑ΂���val���֘A�t����B
            void    put(Context* context, Value key, Value val);
            // key�Ɋ֘A�t����ꂽ�l��Ԃ�
            Value   get(Context* context, Value key);
            // ���g����ɂ���
            void    clear(void);
            // ���g��S���R�s�[�����ʃn�b�V�����쐬����
            Hash*   clone(Context* context);
            // key�Ɋ֘A����v�f���폜�B�폜���ꂽ�l��Ԃ�
            Value   remove(Context* context, Value key);
            // key���L�[�Ƃ��Ď����ǂ���
            bool    hasKey(Context* context, Value key);
            // �S�ẴL�[��z��ɓ���ĕԂ�
            ValueArray*     keys(void);
            // �S�Ă̒l��z��ɓ���ĕԂ�
            ValueArray*     values(void);

            // �L�[�̃n�b�V���l���Čv�Z����Bsize�� capacity() * m_loadFactor/100 ��
            // �z���Ă�����Acapacity�𑝉�������
            void    rehash(Context* context);

            // ���������ǂ���
            bool    equals(Context* context, Hash* o);

            // �������g�̃n�b�V���l���v�Z
            // HashPair_t::hashCode ���g���̂ŁArehash()����ƈႤ�l�ɂȂ�\��������
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

            // �L�[��key�ł���Cell�̃A�h���X�������C�e���[�^��Ԃ�
            // hashCode�𕡐���Čv�Z����Ɩ��ʂȂ̂ŁA�v�Z�ς�hashCode��n��
            HashCell_t::Iterator m_search(Context* context, Value key, hyu32 hashCode);
            // �L�[��key�ł���Cell�̃A�h���X�������C�e���[�^��Ԃ�
            // hashCode�͓����Ōv�Z����
            HashCell_t::Iterator m_search(Context* context, Value key) { return m_search(context, key, key.hashCode(context)); }

        };

    }
}
#endif /* m_HYHASH_H_ */
