/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYGC_H_
#define m_HYGC_H_


#include "hyValue.h"
#include "hyBMap.h"

using namespace Hayat::Common;


class Test_hyGC;

namespace Hayat {
    namespace Engine {

        class Object;
        class Context;

        class GC {
            friend class ::Test_hyGC;   // for unittest
            friend class ThreadManager; // for incremental GC
            friend class CodeManager;   // for incremental GC
            friend class VarTable;      // for incremental GC
            friend class HClass;        // for incremental GC
        public:
            // Object* �ȉ����}�[�N����֐��^
            typedef void (*MarkFunc_t)(Object*);

            // sweep�ɕK�v�ȃ��������ŏ��Ɋ����ĂĂ���
            static void     initialize(void);
            // �����ĂĂ������������J��
            static void     finalize(void);

            // �Ǘ����Ă���Z���S�Ă��A���}�[�N
            static void     unmark(void);
            // Value��REF�������炻�̐��Object���}�[�N
            static void     markValue(Value& val);
            // obj���}�[�N�Bstdlib������class��Object�ȊO�ŌĂ�ł͂����Ȃ�
            static void     markObj(Object* obj);
            // obj���}�[�N�AHY_ENABLE_RELOCATE_OBJECT�Ȃ�΃|�C���^�𒲐�
            static void     markObjP(Object** pObj);
            // List���}�[�N����
            static void     markList(ValueList* p);
            // �g�p����Ă���Z���S�Ă��}�[�N
            static void     markAll(void);
            // orgVal�̃}�[�N�t���O��newObj�ɃR�s�[����
            static void	    copyMarkFlag(Object* newObj, Value& orgVal);
            // mark�t�F�[�Y���̏������݂� write barrier
            static inline void  writeBarrier(const Value& val) {
                if (isPhaseMarking())
                    m_writeBarrier(val);
            }

            // �}�[�N����Ă��Ȃ��Z����S��free����
            static void     sweep(void);

            // GC�������i�߂�
            static void     incremental(void);
            // unmark->mark->sweep�̃t��GC���s�Ȃ�
            static void     full(void);
            // �ׂ荇�����t���[�ȃZ�����������đ傫�ȃZ���ɂ܂Ƃ߂�
            static void     coalesce(void);



            static void     startUnmarkIncremental(void);
            static void     unmarkIncremental(void);
            static void     startSweepIncremental(void);
            static void     sweepIncremental_1(void);
            static void     sweepIncremental_2(void);
            static void     sweepIncremental_3(void);


            static int      countCellsUsing(const MemPool* pool = NULL);
            static int      countObjects(const MemPool* pool = NULL);
            static void     printGCMark(Value& val);
            static const char* getPhaseStr(void);
            static void     printGCPhase(void);
            static bool     isPhaseUnmark(void) { return m_phase == PHASE_UNMARKED || m_phase == PHASE_UNMARKING; }
            static bool     isPhaseMarking(void) { return m_phase == PHASE_MARKING; }
            static bool     isPhaseSwept(void) { return m_phase == PHASE_SWEPT; }


#ifdef HY_ENABLE_BYTECODE_RELOAD
            // ���ꂪ��NULL���ƁAmark����HClass*�̕t���ւ����s�Ȃ���
            static BMap<const HClass*,const HClass*>*       pGenerationMap;
            // pGenerationMap�ɏ]����HClass*�̕t���ւ�������
            static void genClass(const HClass** pHClassPtr);
#endif


        protected:
            static void     m_writeBarrier(const Value& val);




        public:
            typedef enum {
                PHASE_UNMARKING,       // unmark��
                PHASE_UNMARKED,        // unmark����
                PHASE_MARKING,         // mark��
                PHASE_MARKED,          // mark����
                PHASE_SWEEPING,        // sweep��
                PHASE_SWEPT            // sweep����
            } Phase_e;
            static Phase_e          m_phase;

        protected:
            static Context*         m_finalizeContext;



        protected:
            typedef enum {
                SUBPHASE_GLOBAL,
                SUBPHASE_CODEMANAGER,
                SUBPHASE_CODEMANAGER_2,
                SUBPHASE_CODEMANAGER_3,
                SUBPHASE_THREAD,
                SUBPHASE_SWEEP_1,
                SUBPHASE_SWEEP_2,
                SUBPHASE_SWEEP_3
            } SubPhase_e;
            static SubPhase_e   m_subPhase;
            static bool         m_subPhase_end;

            // �O��unmark����GCObject�̌�
            static hys32        m_num_gcobj;
            // incrementalGC��1���unmark/sweep����ő��
            static hys32        m_num_perPart_inc;
            // m_num_perPart_inc�̍ŏ��l
            static const hys32  MIN_NUM_PERPART_INC = 50;
            // unmark�t�F�[�Y�̕�����
            static hys32        m_num_unmark_part;
            // sweep�t�F�[�Y�̕�����
            static hys32        m_num_sweep_part;
            // incrementalGC�̓r�����
            static CellIterator m_itr;
            static Value*       m_pValArray;
            static hys32        m_arrIdx;
            static hys32        m_arrIdx_2;
            static hys32        m_countdown_inc;

#ifndef EXACT_FREEMEMSIZE
            static size_t       m_freeSize;
#endif
        };

    }
}
#endif /* m_HYGC_H_ */
