/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyVarTable.h"
#include <cppunit/extensions/HelperMacros.h>
#include "hyMemPool.h"
#include "hyException.h"
#include "hyThreadManager.h"

using namespace Hayat::Common;
using namespace Hayat::Engine;

class Test_hyGlobalVar : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyGlobalVar);
    CPPUNIT_TEST(test_GlobalVar);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(20480);
        MemPool::initGMemPool(hayatMemory, 20480);
        ThreadManager::firstOfAll();
        gThreadManager.initialize();
    }

    void tearDown(void)
    {
        gThreadManager.finalize();
        HMD_FREE(hayatMemory);
    }


    void test_GlobalVar(void)
    {
        gGlobalVar.initialize(6);

        CPPUNIT_ASSERT_EQUAL((hyu16)6, gGlobalVar.m_bufSize);
        CPPUNIT_ASSERT_EQUAL((hyu16)0, gGlobalVar.m_numVars);

        Value* pv = gGlobalVar.getVarAddr((SymbolID_t)10); // 10
        CPPUNIT_ASSERT_EQUAL(NIL_VALUE,*pv);
        *pv = Value::fromInt(-10);
        CPPUNIT_ASSERT_EQUAL(pv, &(gGlobalVar.m_varTable[0]));

        pv = gGlobalVar.getVarAddr((SymbolID_t)15);
        CPPUNIT_ASSERT_EQUAL(NIL_VALUE,*pv);
        *pv = Value::fromInt(-15);
        CPPUNIT_ASSERT_EQUAL(pv, &(gGlobalVar.m_varTable[1])); // 10, 15

        pv = gGlobalVar.getVarAddr((SymbolID_t)15);
        CPPUNIT_ASSERT_EQUAL(-15,pv->toInt());
        CPPUNIT_ASSERT_EQUAL(pv, &(gGlobalVar.m_varTable[1]));

        pv = gGlobalVar.getVarAddr((SymbolID_t)13); // 10, 13, 15
        CPPUNIT_ASSERT_EQUAL(NIL_VALUE,*pv);
        *pv = Value::fromInt(-13);
        CPPUNIT_ASSERT_EQUAL(pv, &(gGlobalVar.m_varTable[1]));

        pv = gGlobalVar.getVarAddr((SymbolID_t)15);
        CPPUNIT_ASSERT_EQUAL(-15,pv->toInt());
        CPPUNIT_ASSERT_EQUAL(pv, &(gGlobalVar.m_varTable[2]));

        pv = gGlobalVar.getVarAddr((SymbolID_t)4); // 4,10,13,15
        CPPUNIT_ASSERT_EQUAL(NIL_VALUE,*pv);
        *pv = Value::fromInt(-4);
        CPPUNIT_ASSERT_EQUAL(pv, &(gGlobalVar.m_varTable[0]));

        pv = gGlobalVar.getVarAddr((SymbolID_t)22); // 4,10,13,15,22
        CPPUNIT_ASSERT_EQUAL(NIL_VALUE,*pv);
        *pv = Value::fromInt(-22);
        CPPUNIT_ASSERT_EQUAL(pv, &(gGlobalVar.m_varTable[4]));
    
        pv = gGlobalVar.getVarAddr((SymbolID_t)9); // 4,9,10,13,15,22
        CPPUNIT_ASSERT_EQUAL(NIL_VALUE,*pv);
        *pv = Value::fromInt(-9);
        CPPUNIT_ASSERT_EQUAL(pv, &(gGlobalVar.m_varTable[1]));

        // ïœêîóÃàÊfull
        CPPUNIT_ASSERT_EQUAL((hyu16)6, gGlobalVar.m_bufSize);
        CPPUNIT_ASSERT_EQUAL((hyu16)6, gGlobalVar.m_numVars);

        pv = gGlobalVar.getVarAddr((SymbolID_t)19);  // 4,9,10,13,15,19,22
        CPPUNIT_ASSERT_EQUAL(NIL_VALUE,*pv);
        *pv = Value::fromInt(-19);
        CPPUNIT_ASSERT_EQUAL(pv, &(gGlobalVar.m_varTable[5]));

        // ïœêîóÃàÊägí£Ç≥ÇÍÇΩ
        CPPUNIT_ASSERT(6 < gGlobalVar.m_bufSize);
        CPPUNIT_ASSERT_EQUAL((hyu16)7, gGlobalVar.m_numVars);

        pv = gGlobalVar.m_varTable;
        CPPUNIT_ASSERT_EQUAL(-4, pv++ ->toInt());
        CPPUNIT_ASSERT_EQUAL(-9, pv++ ->toInt());
        CPPUNIT_ASSERT_EQUAL(-10, pv++ ->toInt());
        CPPUNIT_ASSERT_EQUAL(-13, pv++ ->toInt());
        CPPUNIT_ASSERT_EQUAL(-15, pv++ ->toInt());
        CPPUNIT_ASSERT_EQUAL(-19, pv++ ->toInt());
        CPPUNIT_ASSERT_EQUAL(-22, pv++ ->toInt());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyGlobalVar);
