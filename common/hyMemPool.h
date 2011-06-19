/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

/*
 * �჌�x���������Ǘ����s�Ȃ��B
 * 1�̘A���������̈��ێ����A
 * �������݂̑��o���A�ԋp���s�Ȃ��B
 */

/*
  �g�p��

  void* p = System_Allocate(size);
  MemPool* pool = MemPool::manage(p, size);
  MemCell* cell = pool->alloc(x);
  pool->free(cell);
  System_Free(p);  �܂���  System_Free(pool);    // pool == p
*/

#ifndef m_HYMEMPOOL_H_
#define m_HYMEMPOOL_H_


#include "machdep.h"



// HMD__PRINTF_MEM
//      �������֘A����\��
// HMD__MEM_CHECK
//      �������̊m�ہE�J���EGC���ɁA�Ǘ��e�[�u���̐������`�F�b�N������
// HMD_DEBUG_MEMORY_ID
//      �e�������Z����ID���4�o�C�g���m�ۂ��邩�ǂ���
// HMD_PRINT_MEMCELL_WHEN_ALLOC_FAILED
//      �������m�ێ��s���ɁA�������Z�������o�͂��邩�ǂ���


#ifdef HMD_DEBUG
// �g���������ɃA���R�����g����
//# define HMD__PRINTF_MEM(...)	HMD_PRINTF(__VA_ARGS__)
//# define HMD__MEM_CHECK
# define HMD_DEBUG_MEMORY_ID
#ifndef TEST__CPPUNIT
# define HMD_PRINT_MEMCELL_WHEN_ALLOC_FAILED
#endif
#else
// �ȉ��ύX���Ȃ���
# undef HMD__PRINTF_MEM
# undef HMD__MEM_CHECK
# undef HMD_DEBUG_MEMORY_ID
# undef HMD_PRINT_MEMCELL_WHEN_ALLOC_FAILED
#endif


// �ȉ��ύX���Ȃ���
#ifndef HMD__PRINTF_MEM
# define HMD__PRINTF_MEM(...)   ((void)0)
#endif




// ���������m�ۂ��鎞�ɁA�v������T�C�Y���A���[�i�ɂ҂�����[�܂�悤��
// �T�C�Y�������s�Ȃ��ꍇ�� define ����
// �����define���Ă��Acoalesce����ƃA���[�i�ɂ҂�����łȂ��Ȃ�ꍇ�͂���
#define m_MEMCELL_JUST_ARENA_SIZE_


class Mock_MemPool;      // for unittest
class Test_hyMemPool;    // for unittest
class Test_hyGC;         // for unittest
namespace Hayat {
    namespace Engine {
        class GC;
    }
}


namespace Hayat {
    namespace Common {

        class CellIterator;

        inline void* addAddr(void* addr, size_t d) {
            return (void*)(((char*)addr) + d);
        }


        // ==================== class MemCell ====================
        //
        // +------------
        // | prevSize       ���O�̃Z���̃T�C�Y
        // |                ���O�̃Z�����t���[�̎��̂ݗL��
        // |                ���O�̃Z���̃������̈�Əd�Ȃ��Ă���
        // +----(A)-----
        // | size           ���̃Z���̃T�C�Y�Fthis�ȍ~�̎g�p�\�ȃT�C�Y
        // |                ��ɗL��
        // |                ����2bit�̓t���O
        // |                  bit0 = ���O�̃Z�����t���[�Ȃ� 1
        // |                  bit1 = ���O�̃Z����GC�ΏۂȂ� 1 �ŁA���e�� Object �ł���

        // +------------
        // | memID    HMD_DEBUG_MEMORY_ID �� define ����Ă��鎞�ɂ̂ݗL��

        // +----(B)----- this : ������MemCell�I�u�W�F�N�g�̃A�h���X
        // | nextFree       �A���[�i���Ŏ��̃t���[�ȃZ���̃A�h���X
        // |                ���̃Z�����t���[�̎��̂ݗL��
        // +------------
        // | prevFree       �A���[�i���őO�̃t���[�ȃZ���̃A�h���X
        // |                ���̃Z�����t���[�̎��̂ݗL��
        // +------------
        // | �Z�����̓����Ă��Ȃ�������
        // +------------
        // | ���̃Z����prevSize  ���̃Z�����t���[�̎��̂ݗL���ŁA���̎�
        // | (== size)           ���̃Z����size��bit0��1�ɂȂ��Ă���
        // +----(C)-----
        //
        // 1��MemCell��(A)����(C)�܂ł̃��������Ǘ�����B
        // �A�v����(B)����(C)�܂ł̃��������g�p�\�B
        // �A�v�����g�p���Ă��Ȃ��t���[��MemCell�̏ꍇ�AnextFree,prevFree,
        // ���̃Z���� prevSize �̕����̃f�[�^���L���B
        //

        class MemCell {
            friend class MemPool;
            friend class CellIterator;
            friend class Engine::GC;
            friend class ::Mock_MemPool;      // for unittest
            friend class ::Test_hyMemPool;    // for unittest
            friend class ::Test_hyGC;         // for unittest

        private:
            MemCell(void) {HMD_HALT();}     // �R���X�g���N�^����

        protected:
            static const size_t MASK_SIZE = (size_t)~3;
            static const size_t MASK_FLAGS = (size_t)3;
            static const size_t MASK_PREV_FREE = 0x00000001;
            static const size_t MASK_GC_OBJECT = 0x00000002;
            static const size_t MASK_GC_OBJECT_FREE = (MASK_PREV_FREE | MASK_GC_OBJECT);
            static const size_t SIZEINFO_SIZE = sizeof(size_t);

#ifdef HMD_DEBUG_MEMORY_ID
#define m_MEMID_SIZE 4
#else
#define m_MEMID_SIZE 0
#endif
            static const size_t MEMCELL_HDR_SIZE = SIZEINFO_SIZE + m_MEMID_SIZE;

#ifdef HMD_DEBUG_MEMORY_ID
# ifdef m_MEMCELL_JUST_ARENA_SIZE_
            static const size_t MINIMUM_FREECELL_SIZE = SIZEINFO_SIZE + sizeof(MemCell*) * 2 + MEMCELL_HDR_SIZE + 4;
# else
            static const size_t MINIMUM_FREECELL_SIZE = SIZEINFO_SIZE + sizeof(MemCell*) * 2 + MEMCELL_HDR_SIZE;
# endif
#else
            static const size_t MINIMUM_FREECELL_SIZE = SIZEINFO_SIZE + sizeof(MemCell*) * 2 + MEMCELL_HDR_SIZE;
#endif

            MemCell*        m_nextFree;
            MemCell*        m_prevFree;

#ifdef HMD_DEBUG_MEMORY_ID
            // �T�C�Y���i�[���Ă���A�h���X���擾
            size_t*         m_pSize(void) { return ((size_t*)this) - 2; }
            // ���O�Z���̃T�C�Y���i�[���Ă���A�h���X���擾
            size_t*         m_pPrevSize(void) { return ((size_t*)this) - 3; }
            // �f�o�b�O�p������ID���i�[���Ă���A�h���X���擾
            char*           m_pMemID(void) { return ((char*)this) - 4; }
            // ����̃Z���̃A�h���X (�������I�[�o�[�ɒ���)
            MemCell*        m_pNextNeighbor(void) { return (MemCell*)addAddr(this, size() + SIZEINFO_SIZE + 4); }
#else
            // �T�C�Y���i�[���Ă���A�h���X���擾
            size_t*         m_pSize(void) { return ((size_t*)this) - 1; }
            // ���O�Z���̃T�C�Y���i�[���Ă���A�h���X���擾
            size_t*         m_pPrevSize(void) { return ((size_t*)this) - 2; }
            // ����̃Z���̃A�h���X (�������I�[�o�[�ɒ���)
            MemCell*        m_pNextNeighbor(void) { return (MemCell*)addAddr(this, size() + SIZEINFO_SIZE); }
#endif
            // ����̃Z����m_pPrevSize()
            size_t*         m_pNextNeighborsPrevSize(void) { return (size_t*)addAddr(this, size() - SIZEINFO_SIZE); }
            // ���̃Z�����t���[���H
            bool            m_bFree(void) { return ((*(size_t*)addAddr(this, size())) & MASK_PREV_FREE) != 0; }
            // ���O�̃Z�����t���[���H
            bool            m_bPrevFree(void) { return ((*m_pSize()) & MASK_PREV_FREE) != 0; }
            // ���̃Z�����t���[�Ƃ���
            void            m_setFree(void);
            // ���̃Z�����t���[�ł͂Ȃ��AGCObject�ł��Ȃ��Ƃ���
            // alloc��������͂��̏�ԂɂȂ�BsetGCObject()�͕K�v�������
            // alloc���Ăяo�������[�`�������s����B
            void            m_setAlloc(void) { (*(size_t*)addAddr(this, size())) &= ~MASK_GC_OBJECT_FREE; }


        public:
            // �Z�����g�p�\�ȃ������T�C�Y���擾 
            size_t  size(void) { return (*m_pSize()) & MASK_SIZE; }
            // �t���O��ύX�����Ƀ������T�C�Y��ݒ�
            void    setSize(size_t size) {
                HMD_DEBUG_ASSERT((size & MASK_FLAGS) == 0);
                size_t* p = m_pSize();
                *p &= MASK_FLAGS;
                *p += size; }
            // ���g�� 0 �Ŗ��߂�
            void    fillZero(void);
            // Garbage Collection �Ώۂł���t���O�𗧂Ă�
            void    setGCObject(void);
            bool    isGCObject(void);
            void    unsetGCObject(void);
#ifdef HMD_DEBUG_MEMORY_ID
            char*   memID(void) { return m_pMemID(); }
            void    setMemID(const char* id);
#else
            inline void     setMemID(const char*) {}
#endif

            void    printMarkNode(void);
        };













        // ==================== class MemPool ====================

        class MemPool {
            friend class ::Test_hyMemPool;      // for unittest
            friend class ::Test_hyGC;           // for unittest
            friend class Engine::GC;

        private:
            MemPool(void) {}
        public:
            // MemPool�C���X�^���X���Ǘ����������Ɋm�ۂ��A�Ǘ��J�n
            // addr��4byte align �łȂ��Ƃ����Ȃ�
            static MemPool* manage(void* addr, size_t size);

            static void     initGMemPool(void* addr, size_t size);

            static const int DEFAULT_ALERT_LEVEL = 80;

        public:
            // �Ǘ����郁�����̈��ݒ�
            void            initialize(void* addr, size_t size);

            MemCell*        alloc(size_t size); // 4byte align�Ŋm�� 
            template<typename T> T* allocT(size_t num) { return (T*)alloc(num * sizeof(T)); }

#ifdef HMD_DEBUG_MEMORY_ID
            MemCell*        alloc(size_t size, const char* id) { MemCell* p = alloc(size); p->setMemID(id); return p; }
            template<typename T> T* allocT(size_t num, const char* id) { MemCell* p = alloc(num * sizeof(T)); p->setMemID(id); return (T*) p;}
#else
            inline MemCell* alloc(size_t size, const char*) { return alloc(size); }
            template<typename T> T* allocT(size_t num, const char*) { return (T*)alloc(num * sizeof(T)); }
#endif


            // ���A���P�[�g
            // �����Z�����Ŋm�ۂł���΂��̂܂܁B
            // �ׂ̃Z�����t���[�ŗe�ʂ��m�ۂł���Ό����B
            // �ʃZ���ɂȂ�̂Ȃ� copySize �o�C�g�̓��e�����R�s�[�B
            // copySize��-1��n���ƁA�O�̃T�C�Y��S�R�s�[�B
            MemCell*        realloc(MemCell* pCell, size_t size, size_t copySize = (size_t)-1);
            // T�^�̃I�u�W�F�N�g��num����悤�ȗe�ʂŃ��A���P�[�g
            // copyNum��T���R�s�[�����
            template<typename T> T* reallocT(T* pCell, size_t num, size_t copyNum = (size_t)-1) {
                if (copyNum == (size_t)-1)
                    return (T*)realloc((MemCell*)pCell, num * sizeof(T), (size_t)-1);
                else
                    return (T*)realloc((MemCell*)pCell, num * sizeof(T), copyNum * sizeof(T));
            }

            void            free(MemCell* pCell);
            void            free(void* pMem) { free((MemCell*)pMem); }
            void            coalesce(void);  // �t���[�ȗׂ肠�����Z����S������

            size_t          manageSize(void) { return ((char*)m_poolEnd) - ((char*)m_poolAddr); }

            // �w��A�h���X���Ǘ��̈�����ǂ���
            bool            inside(void* pMem) { return ((pMem >= m_poolAddr) && (pMem < m_poolEnd)); }


            // �g�p�������p�[�Z���g
            int     percentUsed(void);
            // �g�p������
            int     getUsed(void);

            // �댯���x����ݒ�
            // �g�p�������p�[�Z���g������ȏ�ɂȂ�ƁAisAlert()��true�ɂȂ�
            void    setAlertLevel(int percent);
            // �������g�p�󋵂��댯���x�����ǂ���
            bool    isAlertLevel(void);
            // �������g�p�󋵂�����Ɋ댯���x�����ǂ���
            bool    isAlertAbsorbLevel(void);
            // sweep�I����ɁAm_alertAbsorbLevel���Čv�Z
            void    calcAlertAbsorbLevel(void);
            // isAlertAbsorbLevel()��true��Ԃ���������
            void    enableAbsorbAlert(void) { m_absorbAlertEnableFlag = true; }
            // isAlertAbsorbLevel()��true��Ԃ������֎~
            void    disableAbsorbAlert(void) { m_absorbAlertEnableFlag = false; }


        protected:
            // �W���X�g�t�B�b�g�ȃZ����T���A������΂��傫���Z����T���Đ؂�o��
            MemCell*        m_alloc_1(size_t size);

            // m_head[ind]�̃Z�� pCell ��(�K�؂Ȃ�)��������size�̃Z����؂�o��
            MemCell*        m_divide(int ind, MemCell* pCell, size_t size);

            // 2�ׂ̗������t���[�ȃZ��������
            void            m_coalesce2(MemCell* p, MemCell* q);

        protected:
            void*           m_poolAddr;
            void*           m_poolEnd;
            bool            m_GC_working_in_alloc;


        protected:
            // �A���[�i

#define MA_BIN0SIZE        8
#define MA_BIN0LIMIT       128
#define MA_BIN0INDMAX      (MA_BIN0LIMIT/MA_BIN0SIZE)
#define MA_BIN1SIZE        32
#define MA_BIN1LIMIT       512
#define MA_BIN1INDMAX      (MA_BIN0INDMAX + ((MA_BIN1LIMIT - MA_BIN0LIMIT)/MA_BIN1SIZE))
#define MA_BIN2SIZE        128
#define MA_BIN2LIMIT       2048
#define MA_BIN2INDMAX      (MA_BIN1INDMAX + ((MA_BIN2LIMIT - MA_BIN1LIMIT)/MA_BIN2SIZE))
#define MA_NUMBINS         3
#define MA_BINLASTINDMAX   MA_BIN2INDMAX
#define MA_NUM_TOTAL_BINS  (MA_BINLASTINDMAX+1)
            static const size_t     m_binSizes[MA_NUMBINS];
            static const size_t     m_binLimits[MA_NUMBINS+1];
            static const int        m_binIndMax[MA_NUMBINS+1];
            // m_binSize[n]���݂ŃA���[�i�̃T�C�Y�������Ă����B
            // �T�C�Y��m_binLimits[n+1]�ɂȂ�Ǝ��̃X�e�[�W(++n)�B

            // �A���[�i�̃T�C�Y�� 0x10, 0x18, 0x20, 0x28 .. �ƂȂ��Ă��鎞�A
            // 0x1c�̃Z����alloc���������ɂ� 0x20 �̃A���[�i����擾���A
            // 0x1c�̃Z����free���������ɂ� 0x18 �̃A���[�i�ɒǉ�����

#ifdef m_MEMCELL_JUST_ARENA_SIZE_
            static size_t   m_binIndex2size[MA_NUM_TOTAL_BINS];
            static bool     m_binIndex2size_initialized_flag;
#endif
            // �T�C�Y����Ή����� m_head �̃C���f�b�N�X���擾: �Z����alloc���鎞
            static int      m_size2binIndex_alloc(size_t size);
            // �T�C�Y����Ή����� m_head �̃C���f�b�N�X���擾: �Z����free���鎞
            static int      m_size2binIndex_free(size_t size);

            // �T�C�Y�ʃt���[�Z�����X�g
            MemCell*        m_head[MA_NUM_TOTAL_BINS];
            // �t���[�ȃZ���̃T�C�Y���v
            size_t          m_freeCellSizeTotal;
            // alert�ɂȂ�t���[�Z���T�C�Y���v
            size_t          m_alertLevel;
            // alertAbsorbLevel�ɂȂ�t���[�Z���T�C�Y���v
            size_t          m_alertAbsorbLevel;
            // ���ꂪfalse����alertAbsorbLevel��false��Ԃ�
            bool            m_absorbAlertEnableFlag;
#ifdef HMD_DEBUG
            // GC����̋󂫃������Z���ő�T�C�Y
            size_t          m_maxFreeCellSize;
#endif
            // �Z�����A���[�i�̎w�胊�X�g�ɒǉ�
            void            m_attachTo(MemCell* pCell, int ind);
            // �Z�����A���[�i�ɒǉ�
            void            m_attach(MemCell* pCell) { m_attachTo(pCell, m_size2binIndex_free(pCell->size())); }
            // �Z�����A���[�i�̎w�胊�X�g����O��
            void            m_detachFrom(MemCell* pCell, int ind);
            // �Z�����A���[�i����O��
            void            m_detach(MemCell* pCell) { m_detachFrom(pCell, m_size2binIndex_free(pCell->size())); }



        public:
            // �C�e���[�^

            CellIterator    begin(void) const;
            CellIterator    end(void) const;

            // coalesce���鎞�ɁA�����ɓo�^�����C�e���[�^�̎����Z����
            // �������ꂽ�Ȃ�A�C�e���[�^�Ɏ����|�C���g������
            void            registerCellIterator(CellIterator* pItr) { registeredCellIterator = pItr; }

        protected:
            CellIterator*   registeredCellIterator;


        public:
            typedef void (*MemCellPrintFunc_t)(MemCell*);
            MemCellPrintFunc_t      memCellPrintFunc;
            void    printCellInfo(MemCellPrintFunc_t prFunc = NULL);
            void    printFreeCellInfo(void);
            size_t  totalFreeSize(void) { return m_freeCellSizeTotal; } // ���m�ł͂Ȃ�

#if defined(HMD_DEBUG) || defined(TEST__CPPUNIT)
            int     check_neighbor(void);
            bool    check_free_consistency(MemCell* pHead);
            bool    check(void);
#endif


#ifdef HMD_DEBUG
            // GC����ɂ̂ݐ��������l���Ԃ�
            // �ő�t���[�������Z���T�C�Y
            size_t  maxFreeCellSize(void) { return m_maxFreeCellSize; }
#endif

        };


        // coalesce����ƃ|�C���^���L�����ۏ؂ł��Ȃ�
        class CellIterator {
        protected:
            MemCell*    p;
        public:
            CellIterator(void) { p = NULL; }
            CellIterator(MemCell* cell) { p = cell; }
            CellIterator(const CellIterator& itr) { p = itr.p; }
            
            MemCell*        operator*() const { return p; }
            void            operator++(int) { p = p->m_pNextNeighbor(); }
            // CellIterator&   operator++(int) { p = p->m_pNextNeighbor(); return *this; }
            bool            operator<(const CellIterator& itr) { return p < itr.p; }
        };


        // �O���[�o��MemPool
        HMD_EXTERN_DECL MemPool* gMemPool;

    }
}

#endif /* m_HYMEMPOOL_H_ */
