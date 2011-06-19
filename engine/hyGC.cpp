/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyGC.h"
#include "hyMemPool.h"
#include "hyObject.h"
#include "hyVarTable.h"
#include "hyThread.h"
#include "hyThreadManager.h"
#include "hyCodeManager.h"
#include "hyBytecode.h"
#include "hyContext.h"
#include "hyDebug.h"


#ifdef HMD__MEM_CHECK
#define DEBUG_CHECK_MEMORY(p)     HMD_DEBUG_ASSERT(p->check())
#else
#define DEBUG_CHECK_MEMORY(p)     ((void)0)
#endif



using namespace Hayat::Common;
using namespace Hayat::Engine;

GC::Phase_e GC::m_phase = GC::PHASE_SWEPT;
Context* GC::m_finalizeContext = NULL;


GC::SubPhase_e   GC::m_subPhase;
bool        GC::m_subPhase_end;
hys32       GC::m_num_gcobj = 0;
hys32       GC::m_num_perPart_inc = 0;
hys32       GC::m_num_unmark_part = 20;
hys32       GC::m_num_sweep_part = 20;
CellIterator    GC::m_itr;
Value*      GC::m_pValArray;
hys32       GC::m_arrIdx;
hys32       GC::m_arrIdx_2;
hys32       GC::m_countdown_inc = 0;

#ifndef EXACT_FREEMEMSIZE
size_t      GC::m_freeSize;
#endif

#ifdef HY_ENABLE_BYTECODE_RELOAD
BMap<const HClass*,const HClass*>* GC::pGenerationMap = NULL;
#endif


//============================================================
//                          GC
//============================================================


static hyu32 m_INIT_FC_STACKSIZE = 32;
static hyu32 m_INIT_FC_FRAMESTACKSIZE = 8;

void GC::initialize(void)
{
    // sweepに必要なメモリを最初に割当てておく
    m_finalizeContext = gMemPool->allocT<Context>(1, "GcFC");
    m_finalizeContext->initialize(m_INIT_FC_STACKSIZE, m_INIT_FC_FRAMESTACKSIZE);
    m_phase = PHASE_SWEPT;
    m_num_gcobj = 0;
    m_num_unmark_part = 20;
    m_num_sweep_part = 20;
    m_countdown_inc = 0;

#ifdef HY_ENABLE_BYTECODE_RELOAD
    pGenerationMap = NULL;
#endif
    Debug::printMarkTreeOff();
}

void GC::finalize(void)
{
    if (m_finalizeContext != NULL) {
        gMemPool->free(m_finalizeContext);
        m_finalizeContext = NULL;
    }
}


// void GC::unmark(void)
// {
//     DEBUG_CHECK_MEMORY(gMemPool);
//     m_num_gcobj = 0;
//     CellIterator last = gMemPool->end();
//     for (CellIterator itr = gMemPool->begin(); itr < last; itr++) {
//         MemCell* cell = *itr;
//         if (cell->isGCObject()) {
//             ((Object*)cell)->m_unmark();
//             ++m_num_gcobj;
//         }
//     }
//     DEBUG_CHECK_MEMORY(gMemPool);
//     gCodeManager.clearCodeUsingFlag();
// }

void GC::startUnmarkIncremental(void)
{
    HMD_DEBUG_ASSERT(m_phase == PHASE_SWEPT);
    m_num_perPart_inc = m_num_gcobj / m_num_unmark_part;
    if (m_num_perPart_inc < MIN_NUM_PERPART_INC)
        m_num_perPart_inc = MIN_NUM_PERPART_INC;
    m_phase = PHASE_UNMARKING;
    m_num_gcobj = 0;
    m_itr = gMemPool->begin();
    m_subPhase_end = false;
}

void GC::unmarkIncremental(void)
{
    hys32 countdown = m_num_perPart_inc;
    CellIterator last = gMemPool->end();
    for ( ; m_itr < last; m_itr++) {
        MemCell* cell = *m_itr;
        if (cell->isGCObject()) {
            ((Object*)cell)->m_unmark();
            ++m_num_gcobj;
        }
        if (--countdown <= 0) {
            gMemPool->registerCellIterator(&m_itr);
            return;
        }
    }
    gCodeManager.clearCodeUsingFlag();
    gCodeManager.unmarkStringBox();
    gMemPool->registerCellIterator(NULL);
    m_subPhase_end = true;
}


    
#ifdef HY_ENABLE_BYTECODE_RELOAD
void GC::genClass(const HClass** pHClassPtr)
{
    const HClass** pp = pGenerationMap->find(*pHClassPtr);
    if (pp != NULL) {
        //HMD__PRINTF_FK("genClass %x(%s) -> %x(%s)\n",*pHClassPtr,(*pHClassPtr)->name(),*pp,(*pp)->name());
        *pHClassPtr = *pp;
    }
}
#endif


void GC::markValue(Value& val)
{
    Debug::incMarkTreeLevel();
    if (val.type == HC_REF) {
        markObjP(&(val.objPtr));
    } else if (val.type == HC_INDIRECT_REF) {
        markObjP(&(val.objPtr));
        //HMD_PRINTF("mark indirect %x\n",val.ptrData);
    } else if (val.type->symCheck(HSym_List)) {
        markList((ValueList*) val.ptrData);
#ifdef HY_ENABLE_BYTECODE_RELOAD
    } else if (val.type == HC_String) {
        // バイトコードの文字列テーブルを参照している
        gCodeManager.markString((const char**)&(val.ptrData));
    } else if (val.type == HC_Class) {
        if (pGenerationMap != NULL)
            genClass((const HClass**)(&(val.ptrData)));
    } else {
        if (pGenerationMap != NULL)
            genClass(&(val.type));
    }
#endif
    Debug::decMarkTreeLevel();
}

void GC::markObj(Object* obj)
{
#ifdef HY_ENABLE_RELOCATE_OBJECT
    const HClass* pClass = obj->type();
    if (pClass == HC_RELOCATED_OBJ) {
        obj = *(Object**)obj->field(0);
    }
#ifdef HY_ENABLE_BYTECODE_RELOAD
    else if (pGenerationMap != NULL) {
        const HClass** pNew = pGenerationMap->find(pClass);
        if (pNew != NULL) {
            obj = obj->classGeneration(*pNew);
        }
    }
#endif
#endif
    ((MemCell*)obj)->printMarkNode();
    obj->m_GC_mark();
}

void GC::markObjP(Object** pObj)
{
    if (*pObj == NULL) return;
#ifdef HY_ENABLE_RELOCATE_OBJECT
    const HClass* pClass = (*pObj)->type();
    if (pClass == HC_RELOCATED_OBJ) {
        *pObj = *(Object**)(*pObj)->field(0);
    }
#ifdef HY_ENABLE_BYTECODE_RELOAD
    else if (pGenerationMap != NULL) {
        const HClass** pNew = pGenerationMap->find(pClass);
        if (pNew != NULL) {
            *pObj = (*pObj)->classGeneration(*pNew);
        }
    }
#endif
#endif
    ((MemCell*)*pObj)->printMarkNode();
    (*pObj)->m_GC_mark();
}

void GC::markList(ValueList* p)
{
    for ( ; p != NULL; p = p->tail()) {
        Object::fromCppObj(p)->markSelfOnly();
        ((MemCell*)Object::fromCppObj(p))->printMarkNode();
        GC::markValue(p->head());
    }
}

// void GC::markAll(void)
// {
//     DEBUG_CHECK_MEMORY(gMemPool);
//     // グローバル変数
//     gGlobalVar.m_GC_mark();
        
//     // 全バイトコード → トップレベル定数・クラス変数
//     gCodeManager.m_GC_mark();

//     // スレッド → 全インスタンス
//     ThreadManager::m_GC_mark();

// #ifdef HY_ENABLE_BYTECODE_RELOAD
//     // reloadされた古いバイトコード
//     //gCodeManager.m_GC_mark_replacedBytecodes();
// #endif
//     DEBUG_CHECK_MEMORY(gMemPool);
// }




void GC::copyMarkFlag(Object* newObj, Value& orgVal)
{
    Object* orgObj = NULL;
    if (orgVal.type == HC_REF) {
        orgObj = orgVal.objPtr;
    } else if (orgVal.type == HC_INDIRECT_REF) {
        orgObj = orgVal.objPtr;
    } else if (orgVal.type->symCheck(HSym_List)) {
        if (orgVal.ptrData != NULL)
            orgObj = Object::fromCppObj((ValueList*) orgVal.ptrData);
    }

    bool flag;
    if (orgObj == NULL) {
        flag = true;
    } else {
        flag = orgObj->isMarked();
    }
    //HMD_PRINTF("copyMark flag=%s org=%x new=%x\n", flag ? "true" : "false",orgObj,newObj);

    if (flag)
        newObj->m_mark();
    else
        newObj->m_unmark();
}

void GC::m_writeBarrier(const Value& val)
{
    // 手抜き実装: その場でマークしてしまう
    // 本来は新Objectなら記録しておいてGCフェーズにてやるべきだろうが、
    // 新旧Objectの判定などにコストがかかりそうなので手を抜いた。
    markValue(const_cast<Value&>(val));
}





// void GC::sweep()
// {
//     DEBUG_CHECK_MEMORY(gMemPool);
//     ThreadManager::m_sweep();        // 終了スレッドを削除

//     int freeSize = 0;
// #ifdef HMD_DEBUG
//     gMemPool->m_maxFreeCellSize = 0;
// #endif
//     CellIterator last = gMemPool->end();
//     for (CellIterator itr = gMemPool->begin(); itr < last; itr++) {
//         MemCell* cell = *itr;
//         if (cell->isGCObject()) {
//             if (! ((Object*)cell)->isMarked()) {
// #ifdef TEST__CPPUNIT
//                 // (HClass*)2000 以下はテスト用の仮クラス
//                 if ((hyu32)((Object*)cell)->type() > 2000)
// #endif
//                 {
//                     HMD__PRINTF_GC("GC collect obj %x %s size %x\n", cell, ((Object*)cell)->type()->name(), cell->size());
//                     ((Object*)cell)->m_finalize(m_finalizeContext);
//                 }
// #ifdef TEST__CPPUNIT
//                 else {
//                     HMD__PRINTF_GC("GC collect obj %x size %x\n", cell, cell->size());
//                 }
// #endif
//                 gMemPool->free(cell);
//             }
//         }
//         if (cell->m_bFree()) {
//             size_t size = cell->size();
// #ifndef EXACT_FREEMEMSIZE
//             freeSize += size;
// #endif
// #ifdef HMD_DEBUG
//             if (gMemPool->m_maxFreeCellSize < size)
//                 gMemPool->m_maxFreeCellSize = size;
// #endif
//         }
//     }

//     gCodeManager.deleteUnnecessaryBytecode();

// #ifdef EXACT_FREEMEMSIZE
//     freeSize = 0;
//     last = gMemPool->end();
//     for (CellIterator itr = gMemPool->begin(); itr < last; itr++) {
//         MemCell* cell = *itr;
//         if (cell->m_bFree())
//             freeSize += cell->size();
//     }
// #endif

//     gMemPool->m_freeCellSizeTotal = freeSize;
//     DEBUG_CHECK_MEMORY(gMemPool);
// }


void GC::startSweepIncremental(void)
{
#ifndef EXACT_FREEMEMSIZE
    m_freeSize = 0;
#endif
    m_itr = gMemPool->begin();
    m_num_perPart_inc = m_num_gcobj / m_num_sweep_part;
    if (m_num_perPart_inc < MIN_NUM_PERPART_INC)
        m_num_perPart_inc = MIN_NUM_PERPART_INC;
    m_subPhase_end = false;
}

void GC::sweepIncremental_1(void)
{
    ThreadManager::m_sweep();        // 終了スレッドを削除
}

void GC::sweepIncremental_2(void)
{
    hys32 countdown = m_num_perPart_inc;
    CellIterator last = gMemPool->end();
    for ( ; m_itr < last; m_itr++) {
        if (countdown-- <= 0) {
            gMemPool->registerCellIterator(&m_itr);
            return;
        }
        MemCell* cell = *m_itr;
        if (cell->isGCObject()) {
            if (! ((Object*)cell)->isMarked()) {
#ifdef TEST__CPPUNIT
                // (HClass*)2000 以下はテスト用の仮クラス
                if ((hyu32)((Object*)cell)->type() > 2000)
#endif
                {
                    HMD__PRINTF_GC("GC collect obj %x %s size %x\n", cell, ((Object*)cell)->type()->name(), cell->size());
                    ((Object*)cell)->m_finalize(m_finalizeContext);
                }
#ifdef TEST__CPPUNIT
                else {
                    HMD__PRINTF_GC("GC collect obj %x size %x\n", cell, cell->size());
                }
#endif
                gMemPool->free(cell);
            }
        }
        if (cell->m_bFree()) {
            size_t size = cell->size();
#ifndef EXACT_FREEMEMSIZE
            m_freeSize += size;
#endif
#ifdef HMD_DEBUG
            if (gMemPool->m_maxFreeCellSize < size)
                gMemPool->m_maxFreeCellSize = size;
#endif
        }
    }
    gCodeManager.sweepStringBox();
    gMemPool->registerCellIterator(NULL);
    m_subPhase_end = true;
}

void GC::sweepIncremental_3(void)
{
    gCodeManager.deleteUnnecessaryBytecode();
#ifdef EXACT_FREEMEMSIZE
    int freeSize = 0;
    CellIterator last = gMemPool->end();
    for (CellIterator itr = gMemPool->begin(); itr < last; itr++) {
        MemCell* cell = *itr;
        if (cell->m_bFree())
            freeSize += cell->size();
    }
    gMemPool->m_freeCellSizeTotal = freeSize;
#else
    gMemPool->m_freeCellSizeTotal = m_freeSize;
#endif
    DEBUG_CHECK_MEMORY(gMemPool);
    m_subPhase_end = true;
}




// void GC::step(void)
// {
//     switch (m_phase) {
//     case PHASE_SWEPT:
//     case PHASE_UNMARKING:
//         // HMD__PRINTF_GC("********** ummark\n");
//         unmark();
//         m_phase = PHASE_UNMARKED;
//         break;
//     case PHASE_UNMARKED:
//     case PHASE_MARKING:
//         // HMD__PRINTF_GC("********** mark\n");
//         markAll();
//         m_phase = PHASE_MARKED;
//         break;
//     case PHASE_MARKED:
//     case PHASE_SWEEPING:
//         // HMD__PRINTF_GC("********** sweep\n");
//         sweep();
//         m_phase = PHASE_SWEPT;
//         break;
//     }
// }


void GC::incremental(void)
{
    switch (m_phase) {
    case PHASE_SWEPT:
        startUnmarkIncremental();
        m_phase = PHASE_UNMARKING;
        m_subPhase_end = false;
        // not break
    case PHASE_UNMARKING:
        unmarkIncremental();
        if (m_subPhase_end)
            m_phase = PHASE_UNMARKED;
        break;

    case PHASE_UNMARKED:
        m_phase = PHASE_MARKING;
        m_subPhase = SUBPHASE_GLOBAL;
        gGlobalVar.startMarkIncremental();
#ifdef HMD__DEBUG_MARK_TREE
        if (Debug::isPrintMarkTreeOn())
            HMD_PRINTF("gGlobalVar mark\n");
#endif
        m_subPhase_end = false;
        // not break
    case PHASE_MARKING:
        switch (m_subPhase) {
        case SUBPHASE_GLOBAL:
            gGlobalVar.markIncremental();
            if (m_subPhase_end) {
                m_subPhase = SUBPHASE_CODEMANAGER;
                gCodeManager.startMarkIncremental();
#ifdef HMD__DEBUG_MARK_TREE
                if (Debug::isPrintMarkTreeOn())
                    HMD_PRINTF("gCodeManager mark\n");
#endif
                m_subPhase_end = false;
            }
            break;
            
        case SUBPHASE_CODEMANAGER:
            gCodeManager.markIncremental();
            if (m_subPhase_end) {
                m_subPhase = SUBPHASE_CODEMANAGER_2;
                gCodeManager.startMarkIncremental_2();
#ifdef HMD__DEBUG_MARK_TREE
                if (Debug::isPrintMarkTreeOn())
                    HMD_PRINTF("gCodeManager mark 2\n");
#endif
                m_subPhase_end = false;
            }
            break;

        case SUBPHASE_CODEMANAGER_2:
            gCodeManager.markIncremental_2();
            if (m_subPhase_end) {
                m_subPhase = SUBPHASE_THREAD;
                ThreadManager::startMarkIncremental();
#ifdef HMD__DEBUG_MARK_TREE
                if (Debug::isPrintMarkTreeOn())
                    HMD_PRINTF("ThreadManager mark\n");
#endif
                m_subPhase_end = false;
            }
            break;

        case SUBPHASE_THREAD:
            ThreadManager::markIncremental();
            if (m_subPhase_end) {
                DEBUG_CHECK_MEMORY(gMemPool);
                m_subPhase = SUBPHASE_CODEMANAGER_3;
            }
            break;

        case SUBPHASE_CODEMANAGER_3:
            // スタック上のオブジェクトに write barrier を仕掛けると動作が
            // 重いので、markフェーズの最後にまとめて改めてスタック上の
            // オブジェクトをマークする事で write barrier しなくても
            // 良い様にした。
#ifdef HMD__DEBUG_MARK_TREE
                if (Debug::isPrintMarkTreeOn())
                    HMD_PRINTF("gCodeManager mark all stack\n");
#endif
            gCodeManager.markAllStack();
#ifdef HMD__DEBUG_MARK_TREE
                if (Debug::isPrintMarkTreeOn())
                    HMD_PRINTF("ThreadManager mark all stack\n");
#endif
            ThreadManager::markAllStack();
            m_phase = PHASE_MARKED;
#ifdef HMD__DEBUG_MARK_TREE
                if (Debug::isPrintMarkTreeOn())
                    HMD_PRINTF("mark end.\n");
#endif
            break;

        default:
            HMD_FATAL_ERROR("unknown GC subphase %d", m_subPhase);
        }
        break;

    case PHASE_MARKED:
        startSweepIncremental();
        m_phase = PHASE_SWEEPING;
        m_subPhase = SUBPHASE_SWEEP_1;
        m_subPhase_end = false;
        // not break
    case PHASE_SWEEPING:
        switch (m_subPhase) {
        case SUBPHASE_SWEEP_1:
            sweepIncremental_1();
            m_subPhase = SUBPHASE_SWEEP_2;
            break;

        case SUBPHASE_SWEEP_2:
            sweepIncremental_2();
            if (m_subPhase_end) {
                m_subPhase = SUBPHASE_SWEEP_3;
                m_subPhase_end = false;
            }
            break;

        case SUBPHASE_SWEEP_3:
            sweepIncremental_3();
            if (m_subPhase_end) {
                m_phase = PHASE_SWEPT;
                gMemPool->calcAlertAbsorbLevel();
                Debug::printMarkTreeOff();
            }
            break;

        default:
            HMD_FATAL_ERROR("unknown GC subphase %d", m_subPhase);
        }
        break;

    default:
        HMD_FATAL_ERROR("unknown GC phase %d", m_phase);
    }
}

void GC::unmark(void)
{
    m_phase = PHASE_SWEPT;
    while (m_phase != PHASE_UNMARKED)
        incremental();
}
void GC::markAll(void)
{
    m_phase = PHASE_UNMARKED;
    while (m_phase != PHASE_MARKED)
        incremental();
}
void GC::sweep(void)
{
    m_phase = PHASE_MARKED;
    while (m_phase != PHASE_SWEPT)
        incremental();
}


void GC::full(void)
{
    // incremental()を何回も呼ばないように変数の値を一時的に差し替え
    hys32 unmark_part_bak = m_num_unmark_part;
    hys32 sweep_part_bak = m_num_sweep_part;
    m_num_unmark_part = 1;
    m_num_sweep_part = 1;
    m_num_perPart_inc = 0x7fffffff;
    m_phase = PHASE_SWEPT;
    gMemPool->registerCellIterator(NULL);
    startUnmarkIncremental();
    do {
        incremental();
    } while (m_phase != PHASE_SWEPT);
    m_num_unmark_part = unmark_part_bak;
    m_num_sweep_part = sweep_part_bak;
}

void GC::coalesce(void)
{
    HMD_ASSERT(m_phase != PHASE_UNMARKING && m_phase != PHASE_SWEEPING);
    DEBUG_CHECK_MEMORY(gMemPool);
    gMemPool->coalesce();
    DEBUG_CHECK_MEMORY(gMemPool);
}


int GC::countCellsUsing(const MemPool* pool)
{
    if (pool == NULL)
        pool = gMemPool;
    CellIterator last = pool->end();
    int count = 0;
    for (CellIterator itr = pool->begin(); itr < last; itr++) {
        MemCell* cell = *itr;
        if (!cell->m_bFree()) {
            ++count;
            /*
              if (cell->isGCObject()) {
              HMD_PRINTF("%s ",((Object*)cell)->type()->name());
              } else {
              HMD_PRINTF("[%x] ",cell);
              }
            */
        }
    }
    //HMD_PRINTF("\n");
    return count;
}

int GC::countObjects(const MemPool* pool)
{
    if (pool == NULL)
        pool = gMemPool;
    CellIterator last = pool->end();
    int count = 0;
    for (CellIterator itr = pool->begin(); itr < last; itr++) {
        MemCell* cell = *itr;
        if (!cell->m_bFree()) {
            if (cell->isGCObject()) {
                ++count;
                //HMD_PRINTF("%s ",((Object*)cell)->type()->name());
            } else {
                //HMD_PRINTF("[%x] ",cell);
            }
        }
    }
    //HMD_PRINTF("\n");
    return count;
}


void GC::printGCMark(Value& val)
{
    Object* obj = NULL;
    if (val.type == HC_REF) {
        obj = val.objPtr;
    } else if (val.type == HC_INDIRECT_REF) {
        obj = val.objPtr;
    } else if (val.type->symCheck(HSym_List)) {
        if (val.ptrData != NULL)
            obj = Object::fromCppObj((ValueList*) val.ptrData);
    }

    if (obj == NULL) {
        HMD_PRINTF("(N)");
    } else {
        HMD_PRINTF(obj->isMarked() ? "(M)" : "(U)");
        //HMD_PRINTF(":%x",obj);
    }
}

const char* GC::getPhaseStr(void)
{
    const char* str = NULL;
    switch (m_phase) {
    case PHASE_UNMARKING:
        str = "[unmarking]";
        break;
    case PHASE_UNMARKED:
        str = "[unmarked]";
        break;
    case PHASE_MARKING:
        switch (m_subPhase) {
        case SUBPHASE_GLOBAL: str = "[marking:global]"; break;
        case SUBPHASE_CODEMANAGER: str = "[marking:codeManager]"; break;
        case SUBPHASE_CODEMANAGER_2: str = "[marking:codeManager2]"; break;
        case SUBPHASE_CODEMANAGER_3: str = "[marking:codeManager3]"; break;
        case SUBPHASE_THREAD: str = "[marking:thread]"; break;
        default:;
        }
        break;
    case PHASE_MARKED:
        str = "[marked]";
        break;
    case PHASE_SWEEPING:
        str = "[sweeping]";
        switch (m_subPhase) {
        case SUBPHASE_SWEEP_1: str = "[sweeping:sweep1]"; break;
        case SUBPHASE_SWEEP_2: str = "[sweeping:sweep2]"; break;
        case SUBPHASE_SWEEP_3: str = "[sweeping:sweep3]"; break;
        default:;
        }
        break;
    case PHASE_SWEPT:
        str = "[swept]";
        break;
    default:;
    }
    return str;
}

void GC::printGCPhase(void)
{
    const char* str = getPhaseStr();
    if (str == NULL) {
        HMD_FATAL_ERROR("unknown GC phase %d:%d", m_phase, m_subPhase);
    }
    HMD_PRINTF("%s\n", str);
}
