/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCContext.h"
#include "hyC_opcode.h"
#include "hyCSymbolTable.h"
#include "hyEndian.h"
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


class Test_hyCContext : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyCContext);
    CPPUNIT_TEST(test_stack);
    CPPUNIT_TEST(test_addCode);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;

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
        Package::initialize();
        Context::initializeAll();
    }

    void tearDown(void)
    {
        Package::finalize();
        gFfiTypeMgr.finalize();
        Context::finalizeAll();
        gSymTable.finalize();
        HMD_FREE(hayatMemory);
    }

    void test_stack(void)
    {
        CPPUNIT_ASSERT_EQUAL((hyu32)0, Context::m_contextStack.size());
        CPPUNIT_ASSERT_EQUAL((hyu32)0, Context::m_pool.size());

        Package* pkg = new Package((SymbolID_t)12345);

        Context* top = Context::current();
        CPPUNIT_ASSERT_EQUAL((hyu32)1, Context::m_pool.size());
        CPPUNIT_ASSERT_EQUAL((hyu32)1, Context::m_contextStack.size());

        Context* child = Context::createChild();
        CPPUNIT_ASSERT_EQUAL((hyu32)2, Context::m_pool.size());
        CPPUNIT_ASSERT_EQUAL((hyu32)2, Context::m_contextStack.size());
        CPPUNIT_ASSERT_EQUAL(child, Context::current());

        Context::pop();
        CPPUNIT_ASSERT_EQUAL((hyu32)1, Context::m_contextStack.size());
        CPPUNIT_ASSERT_EQUAL(top, Context::current());

        Context* inner = Context::createInnerClass((SymbolID_t)10);
        CPPUNIT_ASSERT_EQUAL((hyu32)3, Context::m_pool.size());
        CPPUNIT_ASSERT_EQUAL((hyu32)2, Context::m_contextStack.size());
        CPPUNIT_ASSERT_EQUAL(inner, Context::current());

        Context* innerChild = Context::createChild();
        CPPUNIT_ASSERT_EQUAL((hyu32)4, Context::m_pool.size());
        CPPUNIT_ASSERT_EQUAL((hyu32)3, Context::m_contextStack.size());
        CPPUNIT_ASSERT_EQUAL(innerChild, Context::current());

        Context::pop();
        CPPUNIT_ASSERT_EQUAL((hyu32)2, Context::m_contextStack.size());
        CPPUNIT_ASSERT_EQUAL(inner, Context::current());

        Context::pop();
        CPPUNIT_ASSERT_EQUAL((hyu32)1, Context::m_contextStack.size());
        CPPUNIT_ASSERT_EQUAL(top, Context::current());

        Context::push(child);
        CPPUNIT_ASSERT_EQUAL((hyu32)2, Context::m_contextStack.size());
        CPPUNIT_ASSERT_EQUAL(child, Context::current());

        CPPUNIT_ASSERT_EQUAL((hyu32)4, Context::m_pool.size());

        delete pkg;
    }

    void test_addCode(void)
    {
        Context context;
        context.addCode<OP_nop>();
        context.addCode<OP_push_i0>();
        context.addCode<OP_push_i>(OPR_INT(12345));
        context.addCode<OP_push_symbol>(OPR_SYMBOL((SymbolID_t)4321));
        context.addCode<OP_insMethod>(OPR_SIGNATURE(51), OPR_METHOD((SymbolID_t)8642));
        context.addCode<OP_jumpControlStart>(OPR_MAYLOCALVAR(-642), OPR_MAYLOCALVAR(-531), OPR_UINT8(234));

        const hyu8* p = context.bytecode().getCodes(0);
        CPPUNIT_ASSERT_EQUAL((hyu8)OPL_nop, *p);

        CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_i0, *(p+1));

        CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_i, *(p+2));
        CPPUNIT_ASSERT_EQUAL((hys32)12345, Endian::unpack<hys32>(p+3));

        CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_symbol, *(p+7));
        CPPUNIT_ASSERT_EQUAL((SymbolID_t)4321, Endian::unpack<SymbolID_t>(p+8));

        CPPUNIT_ASSERT_EQUAL((hyu8)OPL_insMethod, *(p+8+sizeof(SymbolID_t)));
        CPPUNIT_ASSERT_EQUAL((hyu16)51, Endian::unpack<hyu16>(p+9+sizeof(SymbolID_t)));
        CPPUNIT_ASSERT_EQUAL((SymbolID_t)8642, Endian::unpack<SymbolID_t>(p+11+sizeof(SymbolID_t)));

        CPPUNIT_ASSERT_EQUAL((hyu8)OPL_jumpControlStart, *(p+11+2*sizeof(SymbolID_t)));
        CPPUNIT_ASSERT_EQUAL((hys16)-642, Endian::unpack<hys16>(p+12+2*sizeof(SymbolID_t)));
        CPPUNIT_ASSERT_EQUAL((hys16)-531, Endian::unpack<hys16>(p+14+2*sizeof(SymbolID_t)));
        CPPUNIT_ASSERT_EQUAL((hyu8)234, *(p+16+2*sizeof(SymbolID_t)));
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyCContext);
