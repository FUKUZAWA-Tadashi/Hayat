/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYSTACK_H_
#define m_HYSTACK_H_

#include "hyMMes.h"
#include <string.h>


namespace Hayat {
    namespace Common {

        template<typename ContentType> class Stack
        {
        public:
            static const hyu32 DEFAULT_EXPAND_CAPACITY = 8;

            hyu32 getExpandCapacity(void);


            // Stack<Value>�ɂ��Ă� new,delete �� template<> �Œ�`����
            static void* operator new(size_t)
            {
                HMD_FATAL_ERROR("Stack<T>::operator new() not implemented");
                return NULL;
            }
            static void operator delete(void*)
            {
                HMD_FATAL_ERROR("Stack<T>::operator delete() not implemented");
            }


            Stack(hyu32 initCapacity = 0)
                : m_capacity(0), m_sp(0), m_pStack(NULL)
            {
                initialize(initCapacity);
            }
            ~Stack() { finalize(); }

            void initialize(hyu32 capacity = 0)
            {
                m_sp = 0;
                m_capacity = capacity;
                if (m_pStack != NULL)
                    gMemPool->free(m_pStack);
                if (capacity > 0) {
                    m_pStack = gMemPool->allocT<ContentType>(capacity, "STAC");
                } else
                    m_pStack = NULL;
            }
        
            void finalize(void)
            {
                gMemPool->free(m_pStack);
                m_pStack = NULL;
                m_capacity = 0;
                m_sp = 0;
            }
        
            void clean(void)
            {
                m_sp = 0;
            }

            hyu32 capacity(void)
            {
                return m_capacity;
            }

            hyu32 size(void)
            {
                return m_sp;
            }

            void push(const ContentType& x)
            {
                if (m_sp >= m_capacity)
                    prepareCapacity(m_sp);
                m_pStack[m_sp++] = x;
            }
        
            ContentType pop(void)
            {
                HMD_ASSERTMSG(m_sp > 0, M_M("stack underflow"));
                return m_pStack[--m_sp];
            }

            ContentType& getTop(void)
            {
                HMD_ASSERTMSG(m_sp > 0, M_M("stack underflow"));
                return m_pStack[m_sp-1];
            }
	
            ContentType& getNth(hyu32 n)  // �X�^�b�N����n�Ԗڂ̗v�f(top��n=1)
            {
                HMD_ASSERTMSG(m_sp >= n, M_M("stack underflow"));
                return m_pStack[m_sp-n];
            }
	
            void copyNth(hyu32 n)
            {
                push(getNth(n));
            }

            void drop(hyu32 n)    // �X�^�b�N����n�폜
            {
                HMD_ASSERTMSG(m_sp >= n, M_M("stack underflow"));
                m_sp -= n;
            }

            void dropTo(hyu32 pos)	// size��pos�ɂȂ�悤�ɍ폜
            {
                HMD_ASSERTMSG(m_sp >= pos, M_M("stack underflow"));
                m_sp = pos;
            }
	
            void rot(hyu32 n)		// n�Ԗڂ̗v�f��top�Ɉړ�
            {
                ContentType v = m_pStack[m_sp-n];
                while (n > 1) {
                    m_pStack[m_sp-n] = m_pStack[m_sp-n+1];
                    n--;
                }
                m_pStack[m_sp-1] = v;
            }

            void rotr(hyu32 n)	// top�̗v�f��n�ԖڂɈړ�
            {
                ContentType v = m_pStack[m_sp-1];
                for (hyu32 i = 1; i < n; i++)
                    m_pStack[m_sp-i] = m_pStack[m_sp-i-1];
                m_pStack[m_sp-n] = v;
            }

            ContentType& getAt(hyu32 pos)
            {
                HMD_ASSERTMSG(pos < m_sp, M_M("getAt(%d) stack over (sp %d)"),pos,m_sp);
                return m_pStack[pos];
            }
	
            ContentType* addrAt(hyu32 pos)
            {
                HMD_ASSERTMSG(pos < m_sp, M_M("addrAt(%d) stack over (sp %d)"),pos,m_sp);
                return &(m_pStack[pos]);
            }

            ContentType* topAddr(void)
            {
                if (m_sp == 0)
                    return NULL;
                return &(m_pStack[m_sp-1]);
            }

            void setAt(hyu32 pos, const ContentType& val)
            {
                // setAt �ł̓X�^�b�N�T�C�Y�g���͂��Ȃ�
                HMD_ASSERTMSG(pos < m_sp, M_M("setAt(%d) stack over (sp %d)"),pos,m_sp);
                m_pStack[pos] = val;
            }

            void alloc_a(hyu32 n)	 // �X�^�b�N��n�̈���m��
            {
                if (m_sp + n >= m_capacity)
                    prepareCapacity(m_sp + n);
                m_sp += n;
            }

            void changeCapacity(hyu32 newCapacity)
            {
                if (newCapacity == m_capacity)
                    return;
                if (newCapacity < m_sp)
                    newCapacity = m_sp;
                m_pStack = gMemPool->reallocT<ContentType>(m_pStack, newCapacity, m_sp);
                m_capacity = newCapacity;
            }

            // �Œ�K�v�ȗe�ʈȏ�Ɋm��
            void prepareCapacity(hyu32 needCapacity)
            {
                hyu32 expandCapacity = getExpandCapacity();
                if (needCapacity < expandCapacity)
                    changeCapacity(expandCapacity);
                else
                    changeCapacity(needCapacity + expandCapacity);
            }

            // �X�^�b�N�g�b�v����n��other�Ɉړ�����
            void cutPaste(Stack& other, size_t n)
            {
                other.alloc_a(n);
                ContentType* p1 = topAddr();
                ContentType* p2 = other.topAddr();
                for (size_t i = 0; i < n; i++) {
                    *p2-- = *p1--;
                }
                drop(n);
            }

            // pos�ʒu��num�̃X�y�[�X��}��
            void insertAt(hyu32 pos, hyu32 num = 1)
            {
                HMD_ASSERT(pos <= m_sp);
                hyu32 d = m_sp - pos;
                m_sp += num;
                if (m_sp >= m_capacity)
                    prepareCapacity(m_sp);
                memmove(&(m_pStack[pos+num]), &(m_pStack[pos]), sizeof(ContentType) * d);
            }

            // pos�ʒu��1���폜
            void removeAt(hyu32 pos)
            {
                HMD_ASSERT(pos >= 0 && pos < m_sp);
                --m_sp;
                memmove(&(m_pStack[pos]), &(m_pStack[pos+1]), sizeof(ContentType) * (m_sp - pos));
            }

            // �X�^�b�N����n�Ԗڂ̗v�f(top��n=1)���폜
            void removeNth(hyu32 n)
            {
                removeAt(m_sp - n);
            }


        private:
            hyu32             m_capacity;  // �X�^�b�N�T�C�Y
            hyu32             m_sp;    // �X�^�b�N�|�C���^
            ContentType*    m_pStack;        // �X�^�b�N
        };

        template<typename ContentType> hyu32 Stack<ContentType>::getExpandCapacity(void) { return DEFAULT_EXPAND_CAPACITY; }

    }
}

#endif /* m_HYSTACK_H_ */
