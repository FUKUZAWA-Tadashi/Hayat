/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYBITARRAY_H_
#define m_HYBITARRAY_H_

#include "machdep.h"

class Test_hyBitArray;

namespace Hayat {
    namespace Engine {

        class BitArray {
            friend class ::Test_hyBitArray;
            
        public:
            typedef     hyu8    MemAllocType_t;
        protected:
            hys32       m_size; // �r�b�g��
            hys32       m_memSize; // MemAllocType_t�̌�
            MemAllocType_t*     m_memory;

        public:
            static void* operator new(size_t size, void* addr); // placement new
            static void operator delete(void*, void*) {}
            BitArray(hys32 size = 0);
            BitArray(const BitArray& other);
            ~BitArray() { finalize(); }
            void        finalize(void);
            // finalize()����AchangeSize()�ŗ̈���m�ۂ���΂܂��g����

            hys32       size(void) { return m_size; }
            bool        getAt(hys32 idx);
            void        setAt(hys32 idx, bool x);
            void        insertAt(hys32 idx, bool x);
            void        changeSize(hys32 newSize);
            void        setAll(bool x);
            // idx1 <= i <= idx2 �͈̔͑S���� setAt(i, x)
            bool        setRange(hys32 idx1, hys32 idx2, bool x);
            bool        isSame(const BitArray& other);

            hys32       hashCode(void);
        };

    }
}

#endif /* m_HYBITARRAY_H_ */
