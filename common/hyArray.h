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
  �v�f�̌^�� T �ł���悤�Ȕz��B

  m_contents �́ANULL �łȂ���΃f�[�^�̐擪�A�h���X�B
  m_size �́Am_contents �̒��ŗL���ȃf�[�^�̌��B
  m_capacity �� m_contents �̂��߂Ɋm�ۂ��Ă���e�ʁB
  m_capacity �� 0 �� m_contents �� NULL �łȂ��ꍇ�Am_contents �͊O����
  ���������w���Ă��āA������gMemPool����alloc�������̂ł͂Ȃ����������B

  (1) m_capacity == 0 , m_size == 0 , m_contents == NULL
    [���g�p���]
    ���������m�ۂ��Ă��Ȃ��A���̃f�[�^�������Ă��Ȃ����

  (2) m_capacity > 0 , m_size <= m_capacity , m_contents != NULL
    [�ʏ�g�p���]
    ��������gMemPool����m�ۂ��Ă���A�����ɏ������߂���

  (3) m_capacity == 0, m_size > 0 , m_contents != NULL
    [�O���Q�Ə��]
    �����Ŋm�ۂ����������ł͂Ȃ��O���̃f�[�^���w���Ă�����
    reserve()�̌Ăяo���ɂ��AgMemPool���烁�������m�ۂ��A�����ɊO���f�[�^
    ���R�s�[���āA�������݉\��(2)�̏�ԂɈڍs����B
    ���̏�Ԃ�m_contents�̒��g��ύX����ƁA�O���̃f�[�^���j�󂳂��̂Œ��ӁI
    reserve()���Ă΂��Ƀ|�C���^�E�Q�Ƃ�Ԃ��֐� nth(), nthAddr(), operator[]
    �̎�舵���ɒ��ӂ��K�v�B

 
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

            // ���g�p��ԁE�ʏ�g�p��ԃR���X�g���N�^
            // capacity==0 �Ȃ疢�g�p��ԁAcapacity>0 �Ȃ�ʏ�g�p���
            TArray(hyu32 capacity = 0) { initialize(capacity); }

            // �O���Q�Ə�ԃR���X�g���N�^
            TArray(const T* data, hyu32 numData) { initialize(data, numData); }

            virtual ~TArray() { finalize(); }


            // ���g�p��ԁE�ʏ�g�p��Ԃŏ�����
            // capacity==0 �Ȃ疢�g�p��ԁAcapacity>0 �Ȃ�ʏ�g�p���
            void initialize(hyu32 capacity = 0) {
                m_capacity = capacity;
                if (capacity == 0) {
                    m_contents = NULL;
                } else {
                    m_contents = gMemPool->allocT<T>(capacity, "A_CO");
                }
                m_size = 0;
            }

            // �O���Q�Ə�Ԃŏ�����
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

            // �z��̃T�C�Y
            hyu32     size(void) const { return m_size; }

            // ���݂̗e��
            hyu32     capacity(void) const { return m_capacity; }

            // �c�e��
            hyu32     remain(void) { return m_capacity - m_size; }

            // �C���f�b�N�X���z��͈͂��z����}�C�i�X���l�łȂ����`�F�b�N
            bool    checkIndex(hys32 idx) { return (idx + (hys32)m_size >= 0); }

            // n�Ԗڂ̗v�f�̃A�h���X : �͈̓`�F�b�N����
            T*      nthAddr(int n) const { return m_contents + n; }
            // �擪�v�f�̃A�h���X
            T*      top() const { return m_contents; }

            // idx�Ԗڂ̗v�f : 0 ���ŏ��̗v�f
            // �}�C�i�X���w�肷��ƌ�납�琔���� : -1 ���Ō�̗v�f
            T&      nth(hys32 idx) const {
                HMD_ASSERTMSG(idx >= -(hys32)m_size && idx < (hys32)m_size, M_M("Array index %d out of bounds (size=%d)"),idx,m_size);
                if (idx < 0)
                    idx += m_size;
                return m_contents[idx];
            }

            T&      operator[] (hys32 idx) const { return nth(idx); }

            // idx�̈ʒu��n�̋�Ԃ�}�����A���̐擪�̎Q�Ƃ�Ԃ�
            // �}�������ʒu�̒l�͏��������Ă��Ȃ��̂Œ���
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


            // �S�v�f�� x �Ŗ��߂�
            void    fill(const T& x) {
                reserve(0);
                for (int i = m_size-1; i >= 0; --i) {
                    m_contents[i] = x;
                }
            }


            // offs�ɑ���ł���悤�ɁA�K�v�Ȃ烁�������m�ۂ���
            void    reserve(hyu32 offs) {
                if (m_contents != NULL && m_capacity == 0) {
                    // �O���Q�Ə�ԂȂ̂Ń������m�ۂ��ăR�s�[���Ēʏ�g�p��ԂɂȂ�
                    if (offs < m_size)
                        offs = m_size - 1;
                    m_capacity = (offs + 4) & ~3;
                    T* newmem = gMemPool->allocT<T>(m_capacity, "A_co");
                    memcpy(newmem, m_contents, sizeof(T)*m_size);
                    m_contents = newmem;
                } else if (offs >= m_capacity) {
                    if (m_contents != NULL) {
                        // �ʏ�g�p���
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
                        // ���g�p���
                        HMD_DEBUG_ASSERT(m_size == 0);
                        m_capacity = (offs + 4) & ~3;
                        m_contents = gMemPool->allocT<T>(m_capacity, "A_CO");
                    }
                }
            }

            // idx�Ԗڂ̗v�f�� x �ɂ��� �F �������s���Ȃ�V�������������m��
            // ���ԗv�f�ɂ� defaultVal ������
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

            // 1�𖖔��ɒǉ�����
            void    add(const T& val) {
                reserve(m_size);
                nth(m_size++) = val;
            }

            // values��size�̗v�f�𖖔��ɒǉ�����
            void    add(const T* values, hyu32 size) {
                if (size == 0) return;
                reserve(m_size - 1 + size);
                T* p = &m_contents[m_size];
                for (hyu32 i = 0; i < size; i++) {
                    *p++ = *values++;
                }
                m_size += size;
            }

            // ���̔z��̓��e��S���ǉ�
            void    add(const TArray<T>& fromArr) {
                hyu32 n = fromArr.size();
                if (n == 0) return;
                reserve(m_size - 1 + n);
                T* p = &m_contents[m_size];
                const T* q = fromArr.top();
                memcpy(p, q, sizeof(T) * n);
                m_size += n;
            }

            // ���g����ꂸ��size�̗v�f��ǉ����A�擪�A�h���X��Ԃ�
            // �o�b�t�@���m�ۂ��Ă����Ɏ��X�ƒ��g�����Ă����ꍇ�ɕ֗�
            // �o�b�t�@�I�[�o�[���Ȃ��悤�ɒ��ӂ��Ďg�p���鎖
            T*      addSpaces(hyu32 size) {
                HMD_DEBUG_ASSERT(size > 0);
                reserve(m_size - 1 + size);
                T* p = &m_contents[m_size];
                m_size += size;
                return p;
            }

            // �T�C�Y�� 0 �ɂ���
            void    clear(void) {
                if (m_capacity == 0)
                    m_contents = NULL; // �O���Q�Ə�ԂȂ疢�g�p��ԂɈڍs
                m_size = 0;
            }

            // �T�C�Y��؂�l�߂Ė������̂Ă�
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

            // �l�����o���āA�V�����l�ɒu��������
            T       replace(hys32 idx, T newVal) {
                reserve(0);
                T& ref = nth(idx);
                T val = ref;
                ref = newVal;
                return val;
            }

            // �w�肵���l�Ɠ������̂�S�Ď�菜��
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

            // �v�f���܂�ł��邩�H
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

            // �������̂�����΂��̃C���f�b�N�X�A������Ζ����ɒǉ����Ă��̃C���f�b�N�X��Ԃ�
            hys32     issue(T& val) {
                hys32 idx;
                if (! find(val, &idx)) {
                    idx = m_size;
                    add(val);
                }
                return idx;
            }


            // ��r�֐�compFunc��true��Ԃ��v�f������΁Aidx�ɃC���f�b�N�X��
            // �������true��Ԃ��B ������΁Aidx��ύX������false��Ԃ��B
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

            // ���g�������Ƀ\�[�g����Ă���Ƃ��āA2���T������B
            bool BSearch(const T& val, int* pIdx) const
            {
                return binarySearch<T>(m_contents, m_size, val, pIdx);
            }

            // ���g�������Ƀ\�[�g����Ă���Ƃ��āA
            // �����v�f��T���Ė�����ΓK�؂ȏꏊ�ɑ}������B
            hys32 BInsert(const T& val)
            {
                int idx;
                if (! BSearch(val, &idx)) {
                    insert((hys32)idx) = val;
                }
                return idx;
            }

            // ���g�������Ƀ\�[�g����Ă���Ƃ��āA
            // other�̒��g�S���ɂ��āA�����v�f��T����
            // ������ΓK�؂ȏꏊ�ɑ}������B
            void BMerge(const TArray<T>& other)
            {
                int n = other.size();
                const T* p = other.top();
                while (n-- > 0)
                    BInsert(*p++);
            }



            // bytes�P�ʂɃA���C�����g����B���Ԃ�pad�Ńp�f�B���O����B
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

            // �N�C�b�N�\�[�g
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
