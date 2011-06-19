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


class Test_hyCClassInfo : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyCClassInfo);
    CPPUNIT_TEST(test_var);
    CPPUNIT_TEST(test_class);
    CPPUNIT_TEST(test_funcName);
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

    void test_var(void)
    {
        Package* pkg = new Package(SymS_MAIN);
        Package::registration(pkg);
        ClassInfo* ci = new ClassInfo((SymbolID_t)101, pkg);
        CPPUNIT_ASSERT_EQUAL((SymbolID_t)101, ci->classSymbol());

        CPPUNIT_ASSERT_EQUAL((hyu16)0, ci->numMembVar());
        CPPUNIT_ASSERT(! ci->isMembVar((SymbolID_t)111));
        CPPUNIT_ASSERT(! ci->isMembVar((SymbolID_t)112));
        CPPUNIT_ASSERT_EQUAL((hyu32)0, ci->fieldSize());
        CPPUNIT_ASSERT(ci->isPrimitive());
        ci->addMembVar((SymbolID_t)111);
        CPPUNIT_ASSERT_EQUAL((hyu16)1, ci->numMembVar());
        CPPUNIT_ASSERT(ci->isMembVar((SymbolID_t)111));
        CPPUNIT_ASSERT_EQUAL((hyu32)8, ci->fieldSize());
        CPPUNIT_ASSERT(! ci->isPrimitive());
        ci->addMembVar((SymbolID_t)112);
        CPPUNIT_ASSERT_EQUAL((hyu16)2, ci->numMembVar());
        CPPUNIT_ASSERT(ci->isMembVar((SymbolID_t)112));
        ci->addMembVar((SymbolID_t)111);
        CPPUNIT_ASSERT_EQUAL((hyu16)2, ci->numMembVar());

        CPPUNIT_ASSERT_EQUAL((hyu16)0, ci->numClassVar());
        CPPUNIT_ASSERT(! ci->isClassVar((SymbolID_t)122));
        CPPUNIT_ASSERT(! ci->isClassVar((SymbolID_t)125));
        ci->addClassVar((SymbolID_t)125);
        CPPUNIT_ASSERT_EQUAL((hyu16)1, ci->numClassVar());
        CPPUNIT_ASSERT(ci->isClassVar((SymbolID_t)125));
        ci->addClassVar((SymbolID_t)122);
        CPPUNIT_ASSERT_EQUAL((hyu16)2, ci->numClassVar());
        CPPUNIT_ASSERT(ci->isClassVar((SymbolID_t)125));
        ci->addClassVar((SymbolID_t)122);
        CPPUNIT_ASSERT_EQUAL((hyu16)2, ci->numClassVar());
        ci->addClassVar((SymbolID_t)125);
        CPPUNIT_ASSERT_EQUAL((hyu16)2, ci->numClassVar());

        CPPUNIT_ASSERT_EQUAL((hyu16)0, ci->numConstVar());
        ConstVar_t* cc1 = ci->getConstVar((SymbolID_t)133);
        CPPUNIT_ASSERT_EQUAL((hyu16)0, ci->numConstVar());
        CPPUNIT_ASSERT_EQUAL((ConstVar_t*)NULL, cc1);
        cc1 = ci->createConstVar((SymbolID_t)133);
        ConstVar_t cin1 = *cc1;
        CPPUNIT_ASSERT_EQUAL((hyu16)1, ci->numConstVar());
        ConstVar_t* cc2 = ci->createConstVar((SymbolID_t)111); // may move cc1 addr
        ConstVar_t cin2 = *cc2;
        CPPUNIT_ASSERT_EQUAL((hyu16)2, ci->numConstVar());
        ConstVar_t* cc3 = ci->getConstVar((SymbolID_t)133);
        ConstVar_t cin3 = *cc3;
        CPPUNIT_ASSERT_EQUAL((hyu16)2, ci->numConstVar());
        CPPUNIT_ASSERT(memcmp(&cin1, &cin3, sizeof(ConstVar_t)) == 0);
        ConstVar_t* cc4 = ci->getConstVar((SymbolID_t)111);
        ConstVar_t cin4 = *cc4;
        CPPUNIT_ASSERT(memcmp(&cin2, &cin4, sizeof(ConstVar_t)) == 0);
    }

    void test_class(void)
    {
        Package* pkg = new Package(SymS_MAIN);
        Package::registration(pkg);
        ClassInfo* ci = new ClassInfo((SymbolID_t)1010, pkg);
        CPPUNIT_ASSERT_EQUAL((hyu16)0, ci->numSuper());
        CPPUNIT_ASSERT_EQUAL((hyu32)0, ci->fieldSize());

        CPPUNIT_ASSERT_EQUAL((ClassInfo*)NULL, ci->getInnerClassInfo((SymbolID_t)1020));
        ClassInfo* in1 = ci->addInnerClassCreate((SymbolID_t)1020);
        CPPUNIT_ASSERT_EQUAL(in1, ci->getInnerClassInfo((SymbolID_t)1020));
        CPPUNIT_ASSERT_EQUAL(ci, in1->m_outerInfo);
        ClassInfo* in1in = in1->addInnerClassCreate((SymbolID_t)1030);
        CPPUNIT_ASSERT_EQUAL(in1in, in1->getInnerClassInfo((SymbolID_t)1030));
        CPPUNIT_ASSERT_EQUAL((ClassInfo*)NULL, ci->getInnerClassInfo((SymbolID_t)1030));
        CPPUNIT_ASSERT_EQUAL(in1, in1in->m_outerInfo);
        ClassInfo* in2 = ci->addInnerClassCreate((SymbolID_t)1021);

        CPPUNIT_ASSERT_EQUAL(in1, ci->searchRelativeClassInfo((SymbolID_t)1020));
        CPPUNIT_ASSERT_EQUAL(in1, in2->searchRelativeClassInfo((SymbolID_t)1020));
        CPPUNIT_ASSERT_EQUAL((ClassInfo*)NULL, in1in->searchRelativeClassInfo((SymbolID_t)1020));
        CPPUNIT_ASSERT_EQUAL((ClassInfo*)NULL, in2->searchRelativeClassInfo((SymbolID_t)2010));

        ClassInfo* sup = new ClassInfo((SymbolID_t)2010, pkg);
        CPPUNIT_ASSERT_EQUAL((hyu32)0, in2->fieldSize());
        CPPUNIT_ASSERT(in2->isPrimitive());
        sup->addMembVar((SymbolID_t)123);
        in2->addSuperClass(sup);
        CPPUNIT_ASSERT_EQUAL((hyu32)8, in2->fieldSize());
        in2->addMembVar((SymbolID_t)124);
        CPPUNIT_ASSERT_EQUAL((hyu32)16, in2->fieldSize());
        CPPUNIT_ASSERT(! in2->isPrimitive());
        CPPUNIT_ASSERT_EQUAL(sup, in2->searchRelativeClassInfo((SymbolID_t)2010));

        ClassInfo* co = new ClassInfo((SymbolID_t)3010, pkg);
        CPPUNIT_ASSERT_EQUAL((ClassInfo*)NULL, co->searchUsingClassInfo((SymbolID_t)1020));
        CPPUNIT_ASSERT_EQUAL((ClassInfo*)NULL, co->searchUsingClassInfo((SymbolID_t)1021));
        co->m_addUsingClassInfo(ci);
        CPPUNIT_ASSERT_EQUAL(in1, co->searchUsingClassInfo((SymbolID_t)1020));
        CPPUNIT_ASSERT_EQUAL(in2, co->searchUsingClassInfo((SymbolID_t)1021));
    }
    
    void test_funcName(void)
    {
        SymbolID_t s;
        char buf[256];
        s = gSymTable.symbolID("asdqwe");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfa_asdqwe", buf));
        s = gSymTable.symbolID("Asd_Qwe");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfa_Asd_Qwe",  buf));
        s = gSymTable.symbolID("FOO_1_BAR");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfa_FOO_1_BAR", buf));
        s = gSymTable.symbolID("_foo_2_BAR");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfa__foo_2_BAR", buf));
        s = gSymTable.symbolID("*foo");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfx_2a666f6f", buf));
        s = gSymTable.symbolID("bar?");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfq_bar", buf));
        s = gSymTable.symbolID("baz!");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfe_baz", buf));
        s = gSymTable.symbolID("*zxc!");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfx_2a7a786321", buf));
        s = gSymTable.symbolID("?");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfx_3f", buf));
        s = gSymTable.symbolID("!");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfx_21", buf));
        s = gSymTable.symbolID("*");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfx_2a", buf));
        s = gSymTable.symbolID("<=>");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfx_3c3d3e", buf));
        s = gSymTable.symbolID("@hoge");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfx_40686f6765", buf));
        s = gSymTable.symbolID("fuga-");
        ClassInfo::m_funcName(s, buf, 256);
        CPPUNIT_ASSERT(!HMD_STRCMP("HSfx_667567612d", buf));
    }

    void test_serialize(void)
    {
#define NEWSYM(symName)  SymbolID_t symName = gSymTable.symbolID( #symName );
        
        // ci構築
        NEWSYM(pkgSym);
        Package* pkg = new Package(pkgSym);
        Package::registration(pkg);
        NEWSYM(s_base);
        ClassInfo* ci = new ClassInfo(s_base, pkg);
        pkg->addClassInfo(ci);

        NEWSYM(s_sup1);
        ClassInfo sup1(s_sup1, pkg);
        pkg->addClassInfo(&sup1);
        ci->addSuperClass(&sup1);
        NEWSYM(s_sup2);
        ClassInfo sup2(s_sup2, pkg);
        pkg->addClassInfo(&sup2);
        ci->addSuperClass(&sup2);
        NEWSYM(s_in1);
        ci->addInnerClassCreate(s_in1);
        NEWSYM(s_in2);
        ci->addInnerClassCreate(s_in2);
        NEWSYM(s_insv1);
        ci->addMembVar(s_insv1);
        NEWSYM(s_clsv1);
        ci->addClassVar(s_clsv1);
        NEWSYM(s_clsv2);
        ci->addClassVar(s_clsv2);
        NEWSYM(s_clsv3);
        ci->addClassVar(s_clsv3);
        NEWSYM(s_consv1);
        ConstVar_t* cv1 = ci->createConstVar(s_consv1);
        cv1->type = CONST_TYPE_NONE;
        cv1->intValue = 11;
        NEWSYM(s_consv2);
        ConstVar_t* cv2 = ci->createConstVar(s_consv2);
        cv2->type = CONST_TYPE_INT;
        cv2->intValue = 222;
        NEWSYM(s_consv3);
        ConstVar_t* cv3 = ci->createConstVar(s_consv3);
        cv3->type = CONST_TYPE_FLOAT;
        cv3->floatValue = 333.25f;
        Scope_t* scp = new Scope_t(2);
        NEWSYM(s_scp1);
        scp->add(s_scp1);
        NEWSYM(s_scp2);
        scp->add(s_scp2);
        ci->addUsing(scp);

        Bytecode mb1, mb2;
        mb1.setSignature(0,0);
        mb2.setSignature(0,1);
        NEWSYM(s_method1);
        ci->addMethod(s_method1, &mb1);
        NEWSYM(s_method2);
        ci->addMethod(s_method2, &mb2);
        NEWSYM(sy_si1);
        FfiSigDecl_t fd1;
        fd1.retType = ci->m_nameTable.getOffs("rt1");
        fd1.argDecls = NULL;
        ci->addFfi(sy_si1, fd1);
        NEWSYM(sy_sc1);
        FfiSigDecl_t fd2;
        fd2.retType = NULL_STR_OFFS;
        fd2.argDecls = new TArray<ArgD_t>(2);
        ArgD_t ad;
        ad.argName = ci->m_nameTable.getOffs("arg0");
        ad.typeName = ci->m_nameTable.getOffs("Int");
        fd2.argDecls->subst(0, ad, ad);
        ad.argName = ci->m_nameTable.getOffs("arg1");
        ad.typeName = ci->m_nameTable.getOffs("Type1");
        fd2.argDecls->subst(1, ad, ad);
        ci->addFfi(sy_sc1, fd2);

        // ci 出力
        TArray<hyu8>* out = new TArray<hyu8>();
        ci->serialize(out);

        // dci入力
        ClassInfo* dci = new ClassInfo(s_base, pkg);
        const hyu8* p = out->top();
        const hyu8* q = p + out->size();
        const hyu8* p2 = dci->deserialize(p);

        // チェック
        CPPUNIT_ASSERT_EQUAL(q, p2);
        CPPUNIT_ASSERT_EQUAL(ci->m_outerInfo, dci->m_outerInfo);
        hyu32 n = ci->m_superClasses.size();
        CPPUNIT_ASSERT_EQUAL(n, dci->m_superClasses.size());
        for (hyu32 i = 0; i < n; ++i)
            CPPUNIT_ASSERT_EQUAL(ci->m_superClasses[i], dci->m_superClasses[i]);
        n = ci->m_innerClasses.size();
        CPPUNIT_ASSERT_EQUAL(n, dci->m_innerClasses.size());
        for (hyu32 i = 0; i < n; ++i) {
            CPPUNIT_ASSERT_EQUAL(ci->m_innerClasses.keys()[i], dci->m_innerClasses.keys()[i]);
            CPPUNIT_ASSERT_EQUAL(ci->m_innerClasses.values()[i], dci->m_innerClasses.values()[i]);
        }
        n = ci->m_membVars.size();
        CPPUNIT_ASSERT_EQUAL(n, dci->m_membVars.size());
        for (hyu32 i = 0; i < n; ++i)
            CPPUNIT_ASSERT_EQUAL(ci->m_membVars[i], dci->m_membVars[i]);
        n = ci->m_classVars.size();
        CPPUNIT_ASSERT_EQUAL(n, dci->m_classVars.size());
        for (hyu32 i = 0; i < n; ++i)
            CPPUNIT_ASSERT_EQUAL(ci->m_classVars[i], dci->m_classVars[i]);
        n = ci->m_constVars.size();
        CPPUNIT_ASSERT_EQUAL(n, dci->m_constVars.size());
        for (hyu32 i = 0; i < n; ++i) {
            CPPUNIT_ASSERT_EQUAL(ci->m_constVars.keys()[i], dci->m_constVars.keys()[i]);
            ConstVar_t& cv1 = ci->m_constVars.values()[i];
            ConstVar_t& cv2 = dci->m_constVars.values()[i];
            CPPUNIT_ASSERT_EQUAL(cv1.type, cv2.type);
            CPPUNIT_ASSERT_EQUAL(cv1.intValue, cv2.intValue);
        }
        n = ci->m_usingList.size();
        for (hyu32 i = 0; i < n; ++i) {
            Scope_t* scp1 = ci->m_usingList[i];
            Scope_t* scp2 = dci->m_usingList[i];
            hyu32 m = scp1->size();
            CPPUNIT_ASSERT_EQUAL(m, scp2->size());
            for (hyu32 j = 0; j < m; ++j)
                CPPUNIT_ASSERT_EQUAL(scp1->nth(j), scp2->nth(j));
        }
        n = ci->m_methods.size();
        CPPUNIT_ASSERT_EQUAL(n, dci->m_methods.size());
        for (hyu32 i = 0; i < n; ++i) {
            SymbolID_t symc = ci->m_methods.keys()[i];
            SymbolID_t symd = dci->m_methods.keys()[i];
            CPPUNIT_ASSERT_EQUAL(symc, symd);
            hyu16 sigIDc = ci->m_methods.values()[i].signatureID;
            hyu16 sigIDd = dci->m_methods.values()[i].signatureID;
            CPPUNIT_ASSERT_EQUAL(sigIDc, sigIDd);
        }
        n = ci->m_ffi.size();
        CPPUNIT_ASSERT_EQUAL(n, dci->m_ffi.size());
        for (hyu32 i = 0; i < n; ++i) {
            SymbolID_t symc = ci->m_ffi.keys()[i];
            SymbolID_t symd = dci->m_ffi.keys()[i];
            CPPUNIT_ASSERT_EQUAL(symc, symd);
        }

    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyCClassInfo);
