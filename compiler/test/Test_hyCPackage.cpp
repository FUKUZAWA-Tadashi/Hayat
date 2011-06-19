/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCClassInfo.h"
#include "hyCSymbolTable.h"
#include "hyCPackage.h"
#include "hyCContext.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Compiler;

namespace Hayat {
    namespace Compiler {
        extern SymbolTable gSymTable;
        extern FfiTypeMgr gFfiTypeMgr;
    }
}
extern void init_gSymTable(void);


class Test_hyCPackage : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyCPackage);
    CPPUNIT_TEST(test_serialize);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;
    SymbolID_t SymS_MAIN;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(0x100000);
        MemPool::initGMemPool(hayatMemory, 0x100000);
        init_gSymTable();
#ifndef WIN32
        gSymTable.readFile("../../stdlib/out/symbols.sym");
#else
        gSymTable.readFile("..\\..\\stdlib\\out\\symbols.sym");
#endif
        gFfiTypeMgr.initialize();
        Context::initializeAll();
        Package::initialize();
        SymS_MAIN = gSymTable.symbolID("*MAIN");
    }

    void tearDown(void)
    {
        Package::finalize();
        Context::finalizeAll();
        gFfiTypeMgr.finalize();
        gSymTable.finalize();
        HMD_FREE(hayatMemory);
    }

    void test_serialize(void)
    {
#define NEWSYM(symName)  SymbolID_t symName = gSymTable.symbolID( #symName );
        NEWSYM(s_testPkg);
        Package* pkg = new Package(s_testPkg);
        Package::registration(pkg);
        NEWSYM(s_ci1);
        ClassInfo* ci1 = new ClassInfo(s_ci1, pkg);
        pkg->addClassInfo(ci1);

        // pkg 出力
        TArray<hyu8>* out = new TArray<hyu8>();
        pkg->serialize(out);
        
        // dpkg 入力
        Package* dpkg = new Package();
        const hyu8* p = out->top();
        const hyu8* q = p + out->size();
        const hyu8* p2 = dpkg->deserialize(p);

        // チェック        
        CPPUNIT_ASSERT_EQUAL(q, p2);
        CPPUNIT_ASSERT_EQUAL(s_testPkg, dpkg->getSymbol());
        hyu32 n = pkg->m_linkPackages.size();
        CPPUNIT_ASSERT_EQUAL(n, dpkg->m_linkPackages.size());
        for (hyu32 i = 0; i < n; ++i)
            CPPUNIT_ASSERT_EQUAL(pkg->m_linkPackages[i], dpkg->m_linkPackages[i]);
        n = pkg->m_classInfos.size();
        CPPUNIT_ASSERT_EQUAL(n, dpkg->m_classInfos.size());
        for (hyu32 i = 0; i < n; ++i)
            CPPUNIT_ASSERT_EQUAL(pkg->m_classInfos[i]->classSymbol(), dpkg->m_classInfos[i]->classSymbol());

    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyCPackage);
