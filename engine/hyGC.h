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
            // Object* 以下をマークする関数型
            typedef void (*MarkFunc_t)(Object*);

            // sweepに必要なメモリを最初に割当てておく
            static void     initialize(void);
            // 割当てていたメモリを開放
            static void     finalize(void);

            // 管理しているセル全てをアンマーク
            static void     unmark(void);
            // ValueがREFだったらその先のObjectをマーク
            static void     markValue(Value& val);
            // objをマーク。stdlib所属のclassのObject以外で呼んではいけない
            static void     markObj(Object* obj);
            // objをマーク、HY_ENABLE_RELOCATE_OBJECTならばポインタを調整
            static void     markObjP(Object** pObj);
            // Listをマークする
            static void     markList(ValueList* p);
            // 使用されているセル全てをマーク
            static void     markAll(void);
            // orgValのマークフラグをnewObjにコピーする
            static void	    copyMarkFlag(Object* newObj, Value& orgVal);
            // markフェーズ中の書き込みの write barrier
            static inline void  writeBarrier(const Value& val) {
                if (isPhaseMarking())
                    m_writeBarrier(val);
            }

            // マークされていないセルを全てfreeする
            static void     sweep(void);

            // GCを少し進める
            static void     incremental(void);
            // unmark->mark->sweepのフルGCを行なう
            static void     full(void);
            // 隣り合ったフリーなセルを結合して大きなセルにまとめる
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
            // これが非NULLだと、mark時にHClass*の付け替えが行なわれる
            static BMap<const HClass*,const HClass*>*       pGenerationMap;
            // pGenerationMapに従ってHClass*の付け替えをする
            static void genClass(const HClass** pHClassPtr);
#endif


        protected:
            static void     m_writeBarrier(const Value& val);




        public:
            typedef enum {
                PHASE_UNMARKING,       // unmark中
                PHASE_UNMARKED,        // unmark完了
                PHASE_MARKING,         // mark中
                PHASE_MARKED,          // mark完了
                PHASE_SWEEPING,        // sweep中
                PHASE_SWEPT            // sweep完了
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

            // 前回unmarkしたGCObjectの個数
            static hys32        m_num_gcobj;
            // incrementalGCで1回にunmark/sweepする最大個数
            static hys32        m_num_perPart_inc;
            // m_num_perPart_incの最小値
            static const hys32  MIN_NUM_PERPART_INC = 50;
            // unmarkフェーズの分割個数
            static hys32        m_num_unmark_part;
            // sweepフェーズの分割個数
            static hys32        m_num_sweep_part;
            // incrementalGCの途中状態
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
