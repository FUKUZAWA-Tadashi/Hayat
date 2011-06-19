/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYOBJECT_H_
#define m_HYOBJECT_H_

#include "hyClass.h"
#include "hyStaticAssert.h"


/*
  �I�u�W�F�N�g(�C���X�^���X)�̃f�[�^�\��

  +----- (A)
  | void* m_type         bit0 = GC�}�[�N�t���O (0 = mark, 1 = unmark)
  |                     ��ʃr�b�g = HClass*
  +----- field(0)�̃A�h���X
  |
  | �����o�ϐ��̈�
  | 
  |
  +----- (B)
  | hyu32 �����o�ϐ��̈�̃o�C�g��
  +----- (C)
  |
  | C++ �C���X�^���X�̈�
  |
  |
  +-----

(A) -> (C)
  T* cppObj<T>()

(C) -> (A)
  Object* fromCppObj(const void*)

*/


using namespace Hayat::Common;


class Test_hyGC;                // for unittest

namespace Hayat {
    namespace Common {
        class MemPool;
    }

    namespace Engine {
        class Thread;
        class ThreadManager;

        /// ��{�I�u�W�F�N�g
        class Object {
            friend class GC;
            friend class ::Test_hyGC;           // for unittest
            friend class ThreadManager;     // for Thread GC

        protected:

            union m_TypeMask_t {
                void*           voidp;  // �|�C���^
                unsigned int    intv;   // �|�C���^���r�b�g���Z���邽��
            } m_type;
            // m_type.voidp �� m_type.intv �͓����T�C�Y�łȂ��Ƃ����Ȃ�
            STATIC_ASSERT(sizeof(void*) == sizeof(unsigned int), m_TypeMask_t_size);

            static const unsigned int        m_MASK_TYPE = ~3;
            static const unsigned int        m_MASK_GC_MARK = 1;

            void    m_mark(void) { m_type.intv &= ~m_MASK_GC_MARK; }
            void    m_unmark(void) { m_type.intv |= m_MASK_GC_MARK; }

        public:
            // �������}�[�N����Ă��Ȃ���΁A
            // �����Ƀ}�[�N���āA�����type�ɉ����ĕێ����Ă���Object*���}�[�N����
            void    m_GC_mark(void);

#ifdef HY_ENABLE_BYTECODE_RELOAD
            // �N���X��ύX����B�V���N���X�̃����o�ϐ��̒������s�Ȃ��B
            // �����o�ϐ��̌����قȂ�ꍇ�A�ړ����s�Ȃ���B
            // �V�I�u�W�F�N�g�̏ꏊ��Ԃ��B
            Object* classGeneration(const HClass* newClass);
#if 0
            void s_dump(void);
#endif
#endif

        protected:
            // �f�X�g���N�^
            void    m_finalize(Context* finalizeContext);

        public:
            static Object* create(const HClass* type, size_t size);
            static Object* create(SymbolID_t bytecodeSym, SymbolID_t classSym, size_t size);

            void            destroy(void);
 
            size_t          cellSize(void);  // �Ή�����MemCell�̃o�C�g��
            size_t          size(void);     // �^�����������t�B�[���h�������T�C�Y

            // ���e�̂����A�����o�ϐ����S�����������ǂ���
            bool            equals_memb(Context* context, Object* o);

            // ���e�����������ǂ���
            bool            equals(Context* context, Object* o);

            /// �I�u�W�F�N�g�̌^
            const HClass*   type(void) { return (const HClass*)(m_type.intv & m_MASK_TYPE); }

            bool    isMarked(void) { return (m_type.intv & m_MASK_GC_MARK) == 0; }

            /// �t�B�[���h�̃A�h���X + offs(bytes)
            void*           field(int offs = 0) { return (void*)(((char*)this) + sizeof(Object) + offs); }

            /// n�Ԗڂ̃X�[�p�[�N���X�I�u�W�F�N�g���擾
            Object*         superObj(int n) { return *(Object**)field(sizeof(Object*) * n); }

            /// C++�C���X�^���X�̃A�h���X
            template <typename T> T* cppObj(void) {
			  return (T*) field(type()->fieldSize() + sizeof(hyu32)); }
#ifdef HMD_DEBUG
            template <typename T> T* cppObj(SymbolID_t sym) {
                HMD_ASSERT(type()->symCheck(sym));
                return (T*) field(type()->fieldSize() + sizeof(hyu32)); }
#else
            template <typename T> T* cppObj(SymbolID_t) {
                return (T*) field(type()->fieldSize() + sizeof(hyu32)); }
#endif

            /// C++�C���X�^���X�̃A�h���X����AObject�̃A�h���X���v�Z
            static Object* fromCppObj(const void* cppObj);


            /// GC�̂��߂�mark����B�A�����g�͊֒m���Ȃ��B
            void            markSelfOnly(void) { m_mark(); }

            // GC�̑Ώۂł͂Ȃ����� (�f�t�H���g��GC�ΏۂɂȂ��Ă���)
            void    unsetGCObject(void);
            // GC�̑Ώۂɂ���
            void    setGCObject(void);
            // GC�̑Ώۂ��ǂ���
            bool    isGCObject(void);

        };

    }
}

#endif /* m_HYOBJECT_H_ */
