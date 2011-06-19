/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

// hyDebug.h  for engine


#ifndef m_HYDEBUG_H_
#define m_HYDEBUG_H_

#include "machdep.h"


// HMD__OPCODE_PRINT
//      1�X�e�b�v���ɃI�y�R�[�h��\��
//      ���s���� Debug::setPrintOpcode() �ŕύX�\�@
// HMD__STACK_PRINT
//      1�X�e�b�v���ɃX�^�b�N�̏�Ԃ�\��
//      ���s���� Debug::setPrintStack() �ŕύX�\�@
// HMD__PRINTF_GC
//      �K�[�x�W�R���N�^�֘A����\��
// HMD__PRINTF_FK
//      �J���l�f�o�b�O�\��
// HMD_DEBUG_INFO_LOAD
//      �f�o�b�O�����f�t�H���g�Ń��[�h���邩�ǂ���
// HMD__DEBUG_MARK_TREE
//      GC�̃}�[�N�̗l�q���c���[�Ńf�o�b�O�\������@�\��ǉ����邩


#ifdef HMD_DEBUG

# define HMD__PRINTF_FK(...)   HMD_PRINTF(__VA_ARGS__)
//# define HMD__STACK_PRINT
//# define HMD__OPCODE_PRINT
//# define HMD__PRINTF_GC(...)	HMD_PRINTF(__VA_ARGS__)
//# define HMD_DEBUG_INFO_LOAD
//# define HMD__DEBUG_MARK_TREE

#else // HMD_DEBUG

# undef HMD__STACK_PRINT
# undef HMD__OPCODE_PRINT
# undef HMD__PRINTF_GC
# undef HMD__PRINTF_FK
# undef HMD_DEBUG_INFO_LOAD
# undef HMD__DEBUG_MARK_TREE

#endif // HMD_DEBUG



// don't touch below
#ifndef HMD__PRINTF_GC
# define HMD__PRINTF_GC(...)   ((void)0)
#endif
#ifndef HMD__PRINTF_FK
# define HMD__PRINTF_FK(...)    ((void)0)
#endif




namespace Hayat {
    namespace Common {
        class MemPool;
    }

    namespace Engine {

        // �f�o�b�O����
        class Debug {

#ifdef HMD_DEBUG
        public:
            static bool isPrintStack(void) { return m_bPrintStack; }
            static bool isPrintOpcode(void) { return m_bPrintOpcode; }
            static void setPrintStack(bool b) { m_bPrintStack = b; }
            static void setPrintOpcode(bool b) { m_bPrintOpcode = b; }
            static void printAllStackTrace(void);
        protected:
            static bool m_bPrintStack;
            static bool m_bPrintOpcode;

#else
        public:
            static bool isPrintStack(void) { return false; }
            static bool isPrintOpcode(void) { return false; }
            static void setPrintStack(bool) {}
            static void setPrintOpcode(bool) {}
            static void printAllStackTrace(void) {}
#endif                


#ifdef HMD__DEBUG_MARK_TREE
        public:
            static void printMarkTreeOn(void) { m_bPrintMarkTree = true; m_printMarkTreeLevel = 0; }
            static bool isPrintMarkTreeOn(void) { return m_bPrintMarkTree; }
            static void incMarkTreeLevel(void) { m_printMarkTreeLevel++; }
            static void decMarkTreeLevel(void) { m_printMarkTreeLevel--; }
            static int markTreeLevel(void) { return m_printMarkTreeLevel; }
            static void printMarkTreeOff(void) { m_bPrintMarkTree = false; }
        protected:
            static bool m_bPrintMarkTree;
            static int m_printMarkTreeLevel;
#else
        public:
            static void printMarkTreeOn(void) {}
            static bool isPrintMarkTreeOn(void) { return false; }
            static void incMarkTreeLevel(void) {}
            static void decMarkTreeLevel(void) {}
            static int markTreeLevel(void) { return 0; }
            static void printMarkTreeOff(void) {}
#endif


        public:
            static void setDebugMemPool(Hayat::Common::MemPool* pool) { m_debugMemPool = pool; }
            static Hayat::Common::MemPool* getDebugMemPool(void);
        protected:
            static Hayat::Common::MemPool*     m_debugMemPool;


        };

    }
}


#endif /* m_HYDEBUG_H_ */
