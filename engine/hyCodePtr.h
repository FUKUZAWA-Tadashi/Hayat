/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyValue.h"
#include "hyClass.h"


#ifndef m_HYCODEPTR_H_
#define m_HYCODEPTR_H_


namespace Hayat {
    namespace Engine {
        class Bytecode;
        //class Object;
        class StringBuffer;


        class CodePtr {
        public:
            CodePtr(const hyu8* startAddr = NULL) { initialize(startAddr); }
            void            initialize(const hyu8* startAddr);
            void            finalize(void);
            const hyu8*       addr(void) const { return m_ptr; }
            void            advance(int n) { m_ptr += n; }
            void            jump(const hyu8* addr) { m_ptr = addr; }

            bool            haveCode(void) { return m_ptr != NULL; }

            hyu8              operator*() { return *m_ptr; }
            void            operator+=(int n) { m_ptr += n; }

        protected:
            const hyu8*       m_ptr;

        };




        // �C���X�^���X�����܂񂾃o�C�g�R�[�h���
        // FFI�ɂ��Ή�
        class MethodPtr : public CodePtr {
        public:
            MethodPtr(void) { finalize(); }
            MethodPtr(const Value& self) : m_self(self) {
                setMethodAddr(self.getScope(), NULL); }
            MethodPtr(const Value& self, const hyu8* methodAddr, hyu16 methodID = (hyu16)-1) : m_self(self) {
                setMethodAddr(self.getScope(), methodAddr, methodID); }

            // �C���X�^���X��ݒ�
            void setSelf(const Value& self) { m_self = self; }
            // �o�C�g�R�[�h�Ƃ��Đݒ�
            void setMethodAddr(const HClass* lexicalScope, const hyu8* methodAddr, hyu16 methodID = (hyu16)-1);
            // FFI�Ƃ��Đݒ�
            void setFfi(const HClass* lexicalScope, Ffi_t ffi);

            // �N���X���\�b�h�Ȃ� true, �C���X�^���X���\�b�h�Ȃ� false ��ݒ�
            // FFI�̏ꍇ�̂ݗL��
            void setClassMethod(bool bClassMethod) { m_bClassMethod = bClassMethod; }
            // ���\�b�hID���擾 ; FFI�ł͂Ȃ��ꍇ�̂ݗL��
            hyu16 getMethodID(void) const { HMD_ASSERT(!m_bFfi); return m_methodID; }

            const HClass* lexicalScope(void) const { return m_lexicalScope; }

            void finalize(void);

            const Value&    getSelf(void) const { return m_self; }
            Ffi_t       getFfi(void) const { return (Ffi_t)m_ptr; }

            bool    isFfi(void) const { return m_bFfi; }

            // �N���X���\�b�h���ǂ��� �F FFI�̏ꍇ�̂ݗL��
            bool    isClassMethod(void) const  { return m_bClassMethod; }

        protected:
            Value       m_self;
            // �X�[�p�[�N���X�̃��\�b�h���Ăԏꍇ�Am_lexicalScope��
            // �X�[�p�[�N���X���w���B�łȂ���� m_self.getScope()�Ɠ����B
            const HClass*     m_lexicalScope;
            bool        m_bFfi;  // FFI���ǂ���
            union {
                bool    m_bClassMethod; // ffi���L��
                hyu16     m_methodID; // !ffi���L��
            };

        public:
            void m_GC_mark(void);
        };

    }
}


#endif /* m_HYCODEPTR_H_ */
