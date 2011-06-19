/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hayat.h"
#include <cppunit/extensions/HelperMacros.h>



using namespace Hayat::Common;
using namespace Hayat::Engine;


static int alert_count = 0;
static int prev_alert_count = 0;
static void alertGC(void)
{
    //GC::coalesce();
    ++alert_count;
    //HMD_PRINTF("alert_count %d\n", alert_count);
    //Hayat::Engine::Debug::setPrintOpcode(true);
    //Hayat::Engine::Debug::setPrintStack(true);
}
static bool check_alert(void)
{
    bool b = (alert_count != prev_alert_count);
    prev_alert_count = alert_count;
    return b;
}


class Test_hyGC : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyGC);
    CPPUNIT_TEST(test_markSweep);
    CPPUNIT_TEST(test_globalVar);
    CPPUNIT_TEST(test_instance);
    CPPUNIT_TEST(test_threadGC);
    CPPUNIT_TEST(test_threadGC2);
    CPPUNIT_TEST(test_memover1);
    CPPUNIT_TEST(test_hash);
    CPPUNIT_TEST(test_writeBarrier);
    CPPUNIT_TEST(test_writeBarrier_2);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(0x10000);
        Hayat::Engine::initMemory(hayatMemory, 0x10000);
        Hayat::Engine::initStdlib();
    }
    
    void tearDown(void)
    {
        Hayat::Engine::finalizeAll();
        HMD_FREE(hayatMemory);
    }


    void test_markSweep(void)
    {
        MemPool* orgMemPool = gMemPool;

        void* tmp = HMD_ALLOC(2048);
        MemPool::initGMemPool(tmp, 2048);
        GC::initialize();
        MemPool* pool = gMemPool;

        Object* o1 = Object::create(HC_StringBuffer, 20);
        MemCell* c2 = pool->alloc(40);
        Object* o3 = Object::create(HC_StringBuffer, 60);
        MemCell* c4 = pool->alloc(80);
        Object* o5 = Object::create(HC_StringBuffer, 100);
        Object* o6 = Object::create(HC_StringBuffer, 120);
        Object* o7 = Object::create(HC_StringBuffer, 80);
        MemCell* c8 = pool->alloc(100);
        Object* o9 = Object::create(HC_StringBuffer, 60);
        Object* o10 = Object::create(HC_StringBuffer, 40);

        // ObjectはGCObjectで、MemCellはGCObjectではない
        CPPUNIT_ASSERT(((MemCell*)o1)->isGCObject());
        CPPUNIT_ASSERT(((MemCell*)o3)->isGCObject());
        CPPUNIT_ASSERT(((MemCell*)o5)->isGCObject());
        CPPUNIT_ASSERT(((MemCell*)o6)->isGCObject());
        CPPUNIT_ASSERT(((MemCell*)o7)->isGCObject());
        CPPUNIT_ASSERT(((MemCell*)o9)->isGCObject());
        CPPUNIT_ASSERT(((MemCell*)o10)->isGCObject());
        CPPUNIT_ASSERT(! c2->isGCObject());
        CPPUNIT_ASSERT(! c4->isGCObject());
        CPPUNIT_ASSERT(! c8->isGCObject());

        // Object生成当初はマークされている
        CPPUNIT_ASSERT(o1->isMarked());
        CPPUNIT_ASSERT(o3->isMarked());
        CPPUNIT_ASSERT(o5->isMarked());
        CPPUNIT_ASSERT(o6->isMarked());
        CPPUNIT_ASSERT(o7->isMarked());
        CPPUNIT_ASSERT(o9->isMarked());
        CPPUNIT_ASSERT(o10->isMarked());

        GC::unmark();

        CPPUNIT_ASSERT(! o1->isMarked());
        CPPUNIT_ASSERT(! o3->isMarked());
        CPPUNIT_ASSERT(! o5->isMarked());
        CPPUNIT_ASSERT(! o6->isMarked());
        CPPUNIT_ASSERT(! o7->isMarked());
        CPPUNIT_ASSERT(! o9->isMarked());
        CPPUNIT_ASSERT(! o10->isMarked());

        o3->m_mark();
        o5->m_mark();
        o6->m_mark();
        o9->m_mark();
        CPPUNIT_ASSERT(o3->isMarked());
        CPPUNIT_ASSERT(o5->isMarked());
        CPPUNIT_ASSERT(o6->isMarked());
        CPPUNIT_ASSERT(o9->isMarked());

        // 回収
        GC::sweep();

        // freeじゃないのはMemCellとmarkしたObjectとfinalize用Context
        int flag = 0;
        for (CellIterator itr = pool->begin(); itr < pool->end(); itr++) {
            MemCell* cell = *itr;
            if (! cell->m_bFree()) {
                if (cell == c2) {
                    CPPUNIT_ASSERT((flag & 0x01) == 0);
                    flag |= 0x01;
                } else if (cell == c4) {
                    CPPUNIT_ASSERT((flag & 0x02) == 0);
                    flag |= 0x02;
                } else if (cell == c8) {
                    CPPUNIT_ASSERT((flag & 0x04) == 0);
                    flag |= 0x04;
                } else if (cell == (MemCell*)o3) {
                    CPPUNIT_ASSERT((flag & 0x08) == 0);
                    flag |= 0x08;
                } else if (cell == (MemCell*)o5) {
                    CPPUNIT_ASSERT((flag & 0x10) == 0);
                    flag |= 0x10;
                } else if (cell == (MemCell*)o6) {
                    CPPUNIT_ASSERT((flag & 0x20) == 0);
                    flag |= 0x20;
                } else if (cell == (MemCell*)o9) {
                    CPPUNIT_ASSERT((flag & 0x40) == 0);
                    flag |= 0x40;
                }
            }
        }
        CPPUNIT_ASSERT(flag == 0x7f);

        GC::finalize();

        gMemPool = orgMemPool;
        HMD_FREE(tmp);
    }

    void test_globalVar(void)
    {
        Value* pv = gGlobalVar.getVarAddr((SymbolID_t)1);
        *pv = Value::fromInt(100);
        pv = gGlobalVar.getVarAddr((SymbolID_t)2);
        *pv = Value::fromFloat(200.0f);

        Object* o1 = Object::create(HC_StringBuffer, 20);
        pv = gGlobalVar.getVarAddr((SymbolID_t)3);
        *pv = Value::fromObj(o1);

        pv = gGlobalVar.getVarAddr((SymbolID_t)4);
        *pv = Value::fromFloat(400.0f);

        Object* o2 = Object::create(HC_StringBuffer, 10);
        pv = gGlobalVar.getVarAddr((SymbolID_t)5);
        *pv = Value::fromObj(o2);

        pv = gGlobalVar.getVarAddr((SymbolID_t)6);
        *pv = NIL_VALUE;

        MemCell* cell = gMemPool->alloc(50);

        pv = gGlobalVar.getVarAddr((SymbolID_t)7);
        *pv = TRUE_VALUE;


        GC::unmark();
        GC::markAll();

        CPPUNIT_ASSERT(o1->isMarked());
        CPPUNIT_ASSERT(o2->isMarked());


        pv = gGlobalVar.getVarAddr((SymbolID_t)3);
        *pv = FALSE_VALUE;

        GC::unmark();
        GC::markAll();

        CPPUNIT_ASSERT(! o1->isMarked());
        CPPUNIT_ASSERT(o2->isMarked());

        GC::sweep();

        CPPUNIT_ASSERT(! ((MemCell*)(gGlobalVar.m_varTable))->m_bFree());
        CPPUNIT_ASSERT(! ((MemCell*)(gGlobalVar.m_symbolTable))->m_bFree());
        CPPUNIT_ASSERT(((MemCell*)o1)->m_bFree());
        CPPUNIT_ASSERT(! ((MemCell*)o2)->m_bFree());
        //CPPUNIT_ASSERT(! ((MemCell*)o3)->m_bFree());
        CPPUNIT_ASSERT(! cell->m_bFree());

        CPPUNIT_ASSERT(gMemPool->check());

        pv = gGlobalVar.getVarAddr((SymbolID_t)5);
        *pv = TRUE_VALUE;
        GC::unmark();
        GC::markAll();
        CPPUNIT_ASSERT(! o2->isMarked());
        CPPUNIT_ASSERT(! ((MemCell*)o2)->m_bFree());
        GC::sweep();
        CPPUNIT_ASSERT(((MemCell*)o2)->m_bFree());

    }

    void test_instance(void)
    {
        MemPool* pool = gMemPool;

        // ffi定義が無いのでbindFfiClassTable不要
        Bytecode* myBytecode = gCodeManager.readBytecode("test.hyb");
        CPPUNIT_ASSERT(myBytecode != NULL);
        myBytecode->setInitialized();

        // gc_test_instance() をスレッドで実行
        Thread* pThread = gThreadManager.createTmpThread();
        Context* context = gCodeManager.createContext();
        pThread->initialize(context);
        context->pushClass(myBytecode->mainClass());
        context->methodCall(HSym_gc_test_instance, 0);
        gCodeManager.releaseContext(context); // contextのdeleteはGCまかせ
        pThread->start();

        GC::full();

        pThread->exec1tick();

        int n, m;
        n = GC::countObjects(pool);  // start

        pThread->exec1tick();
        GC::full();
        m = GC::countObjects(pool);
        CPPUNIT_ASSERT_EQUAL(n-1, m);       // -1

        pThread->exec1tick();
        GC::full();
        n = GC::countObjects(pool);
        CPPUNIT_ASSERT_EQUAL(m, n);       // -0

        pThread->exec1tick();
        GC::full();
        m = GC::countObjects(pool);
        CPPUNIT_ASSERT_EQUAL(n, m);       // -0

        pThread->exec1tick();
        GC::full();
        n = GC::countObjects(pool);
        CPPUNIT_ASSERT_EQUAL(m-3, n);       // -3

        pThread->exec1tick();
        GC::full();
        m = GC::countObjects(pool);
        CPPUNIT_ASSERT_EQUAL(n-1, m);       // -1

        CPPUNIT_ASSERT_EQUAL(Thread::WAITING_TICK, pThread->state());
        pThread->exec1tick();
        CPPUNIT_ASSERT_EQUAL(Thread::TERMINATED, pThread->state());
        GC::full();
        n = GC::countObjects(pool);
        CPPUNIT_ASSERT_EQUAL(m-2, n);       // -2

        // pThread,context は GC に回収されている
    }

    void test_threadGC(void)
    {
        Bytecode* myBytecode = gCodeManager.readBytecode("test.hyb");
        myBytecode->setInitialized();

        Context* context = gCodeManager.createContext();
        Thread* pThread = gThreadManager.createTmpThread();
        context->pushClass(myBytecode->mainClass());
        context->methodCall(HSym_gc_test_thread_sweep, 0);
        pThread->initialize(context);
        pThread->start();
        gCodeManager.releaseContext(context);
        while (gThreadManager.isThreadRunning())
            gThreadManager.exec1tick();
        gThreadManager.destroyTmpThread(pThread);
        GC::full();
        GC::coalesce();

        pThread = gThreadManager.createTmpThread();
        context = gCodeManager.createContext();
        context->pushClass(myBytecode->mainClass());
        context->methodCall(HSym_gc_test_thread_sweep, 0);
        pThread->initialize(context);
        pThread->start();
        gCodeManager.releaseContext(context);
        while (gThreadManager.isThreadRunning())
            gThreadManager.exec1tick();
        gThreadManager.destroyTmpThread(pThread);
        GC::unmark();
        pThread = gThreadManager.createTmpThread();
        context = gCodeManager.createContext();
        context->pushClass(myBytecode->mainClass());
        context->methodCall(HSym_gc_test_thread_sweep, 0);
        pThread->initialize(context);
        pThread->start();
        gCodeManager.releaseContext(context);
        while (gThreadManager.isThreadRunning())
            gThreadManager.exec1tick();
        gThreadManager.destroyTmpThread(pThread);
        GC::markAll();
        GC::sweep();
        GC::coalesce();

        pThread = gThreadManager.createTmpThread();
        context = gCodeManager.createContext();
        context->pushClass(myBytecode->mainClass());
        context->methodCall(HSym_gc_test_thread_sweep, 0);
        pThread->initialize(context);
        pThread->start();
        gCodeManager.releaseContext(context);
        while (gThreadManager.isThreadRunning())
            gThreadManager.exec1tick();
        gThreadManager.destroyTmpThread(pThread);
        GC::unmark();
        GC::markAll();
        pThread = gThreadManager.createTmpThread();
        context = gCodeManager.createContext();
        context->pushClass(myBytecode->mainClass());
        context->methodCall(HSym_gc_test_thread_sweep, 0);
        pThread->initialize(context);
        pThread->start();
        gCodeManager.releaseContext(context);
        while (gThreadManager.isThreadRunning())
            gThreadManager.exec1tick();
        gThreadManager.destroyTmpThread(pThread);
        GC::sweep();
        GC::coalesce();

    }
        
    void test_threadGC2(void)
    {
        Bytecode* myBytecode = gCodeManager.readBytecode("test.hyb");
        CPPUNIT_ASSERT(myBytecode != NULL);
        myBytecode->setInitialized();

        hys32 ex[] = {0, 99, 98, 97, 96, 95, 95};
        for (int n = 1; n < 5; ++n) {
            Context* context = gCodeManager.createContext();
            Thread* pThread = gThreadManager.createTmpThread();
            context->pushInt(n);
            context->pushClass(myBytecode->mainClass());
            context->methodCall(HSym_gc_test_multiThread, 1);
            pThread->initialize(context);
            pThread->start();
            gCodeManager.releaseContext(context);
            GC::full();
            int count = 0;
            while (gThreadManager.isThreadRunning()) {
                gThreadManager.exec1tick();
                Context* qc = gCodeManager.createContext();
                qc->pushClass(myBytecode->mainClass());
                qc->execMethod(HSym_getGlobalB, 0);
                hys32 b = qc->popInt();
                if (count < n)
                    CPPUNIT_ASSERT_EQUAL(0, b);
                else
                    CPPUNIT_ASSERT_EQUAL(ex[count-n], b);
                ++count;
                gCodeManager.releaseContext(qc);
                GC::incremental();
            }

            GC::full();
            GC::coalesce();
        }

    }


    void test_memover1(void)
    {
        VM::setAlertGCCallback(alertGC);

        Bytecode* myBytecode = gCodeManager.readBytecode("test.hyb");
        CPPUNIT_ASSERT(myBytecode != NULL);
        myBytecode->setInitialized();

        Value mcls(HC_Class, (void*) myBytecode->mainClass());

        alert_count = prev_alert_count = 0;
        int c = 0;
        while (c < 30) {
            Context* context = gCodeManager.createContext();
            Thread* pThread = gThreadManager.createTmpThread();
            pThread->initialize(context);
            context->pushInt(10);
            context->methodCall(mcls, HSym_create_XClass, 1);
            pThread->start();
            while (pThread->isActive())
                pThread->exec1tick();
            context->pop();
            gCodeManager.releaseContext(context);
            if (check_alert()) ++c;
        }
        c = 0;
        while (c < 30) {
            Context* context = gCodeManager.createContext();
            Thread* pThread = gThreadManager.createTmpThread();
            pThread->initialize(context);
            context->pushInt(500);
            context->methodCall(mcls, HSym_create_XClass, 1);
            pThread->start();
            while (pThread->isActive())
                pThread->exec1tick();
            context->pop();
            gCodeManager.releaseContext(context);
            if (check_alert()) ++c;
        }
        c = 0;
        while (c < 30) {
            Context* context = gCodeManager.createContext();
            Thread* pThread = gThreadManager.createTmpThread();
            pThread->initialize(context);
            context->methodCall(mcls, HSym_create_YClass, 0);
            pThread->start();
            while (pThread->isActive())
                pThread->exec1tick();
            context->pop();
            gCodeManager.releaseContext(context);
            if (check_alert()) ++c;
        }
    }

    
    void test_hash(void)
    {
        // ffi定義が無いのでbindFfiClassTable不要
        Bytecode* myBytecode = gCodeManager.readBytecode("test.hyb");
        CPPUNIT_ASSERT(myBytecode != NULL);
        myBytecode->setInitialized();

        GC::full();
        GC::coalesce();

        size_t fs = gMemPool->totalFreeSize();

        // gc_test_hash() をスレッドで実行
        Thread* pThread = gThreadManager.createTmpThread();
        Context* context = gCodeManager.createContext();
        pThread->initialize(context);
        context->pushClass(myBytecode->mainClass());
        context->methodCall(HSym_gc_test_hash, 0);
        gCodeManager.releaseContext(context); // contextのdeleteはGCまかせ
        pThread->start();

        size_t fs2 = gMemPool->totalFreeSize();

        int i = 0;
        size_t ls;
        size_t ls2;
        while (pThread->isActive()) {
            GC::full();
            GC::coalesce();
            ls2 = gMemPool->totalFreeSize();
            pThread->exec1tick();
            ++i;
        }


        GC::full();
        GC::coalesce();

        ls = gMemPool->totalFreeSize();

        CPPUNIT_ASSERT_EQUAL(fs2, ls2); // pThread,contextあり
        CPPUNIT_ASSERT_EQUAL(fs, ls);   // pThread,context無し

    }

    void test_writeBarrier(void)
    {
        GC::initialize();

        Bytecode* myBytecode = gCodeManager.readBytecode("test.hyb");
        myBytecode->setInitialized();

        Context* context = gCodeManager.createContext();
        Thread* pThread = gThreadManager.createTmpThread();
        context->pushClass(myBytecode->mainClass());
        context->methodCall(HSym_gc_test_writeBarrier, 0);
        pThread->initialize(context);
        pThread->start();

        gThreadManager.exec1tick();
        GC::unmark();

        for (int i = 0; i < 300; ++i) {
            gThreadManager.exec1tick();
            GC::incremental();
        }

        GC::full();
        Value* ga = gGlobalVar.getVarAddr_notCreate(HSym_a);
        CPPUNIT_ASSERT(ga != NULL);
        ValueArray* arr = ga->toCppObj<ValueArray>(HSym_Array);
        for (int i = 0; i < 100; ++i) {
            Value v = arr->nth(i);
            Object* obj = v.toObj();
            CPPUNIT_ASSERT_EQUAL(HC_StringBuffer, obj->type());
            //StringBuffer* sb = obj->cppObj<StringBuffer>();
            //HMD_PRINTF("%d:%x %x type=%x(SB=%x)",i,obj,sb,obj->type(),HC_StringBuffer);
            //HMD_PRINTF(" bufSize=%d,length=%d,buf=%x\n",sb->bufSize(),sb->length(),sb->top());
            CPPUNIT_ASSERT(obj->isMarked());
        }

        GC::finalize();
    }

    void test_writeBarrier_2(void)
    {
        GC::initialize();

        Bytecode* myBytecode = gCodeManager.readBytecode("test.hyb");
        myBytecode->setInitialized();

        Context* context = gCodeManager.createContext();
        Thread* pThread = gThreadManager.createTmpThread();
        context->pushClass(myBytecode->mainClass());
        context->methodCall(HSym_gc_test_writeBarrier_2, 0);
        pThread->initialize(context);
        pThread->start();

        GC::unmark();
        while (gThreadManager.isThreadRunning()) {
            gThreadManager.exec1tick();
            GC::incremental();
        }

        GC::full();
        Value* ga = gGlobalVar.getVarAddr_notCreate(HSym_a);
        CPPUNIT_ASSERT(ga != NULL);
        ValueArray* arr = ga->toCppObj<ValueArray>(HSym_Array);
        for (int i = 0; i < 50; ++i) {
            Value v = arr->nth(i);
            Object* obj = v.toObj();
            CPPUNIT_ASSERT_EQUAL(HC_Array, obj->type());
            CPPUNIT_ASSERT(obj->isMarked());
            ValueArray* a2 = obj->cppObj<ValueArray>();
            obj = a2->nth(0).toObj();
            CPPUNIT_ASSERT_EQUAL(HC_StringBuffer, obj->type());
            CPPUNIT_ASSERT(obj->isMarked());
            obj = a2->nth(1).toObj();
            CPPUNIT_ASSERT_EQUAL(HC_StringBuffer, obj->type());
            CPPUNIT_ASSERT(obj->isMarked());
        }

        GC::finalize();
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyGC);
