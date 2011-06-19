/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCJumpControlTree.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Compiler;

class Test_hyCJumpControlTree : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyCJumpControlTree);
    CPPUNIT_TEST(test_label);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(20480);
        MemPool::initGMemPool(hayatMemory, 20480);
    }

    void tearDown(void)
    {
        HMD_FREE(hayatMemory);
    }

    void test_label(void)
    {
        JumpControlTree* root = new JumpControlTree(NULL);
        CPPUNIT_ASSERT_EQUAL(JumpControlTree::INVALID_ADDR, root->getLocalLabelAddr((SymbolID_t)100));
        root->addLabel((SymbolID_t)100, (hyu32)10100);
        CPPUNIT_ASSERT_EQUAL((hyu32)10100, root->getLocalLabelAddr((SymbolID_t)100));
        JumpControlTree* child = root->newChild();
        CPPUNIT_ASSERT_EQUAL(root, child->parent());
        child->addLabel((SymbolID_t)110, (hyu32)10110);
        CPPUNIT_ASSERT_EQUAL((hyu32)10110, child->getLocalLabelAddr((SymbolID_t)110));
        CPPUNIT_ASSERT_EQUAL(JumpControlTree::INVALID_ADDR, child->getLocalLabelAddr((SymbolID_t)100));
        CPPUNIT_ASSERT_EQUAL((hyu32)10100, child->getLabelAddr((SymbolID_t)100));
        CPPUNIT_ASSERT_EQUAL(JumpControlTree::INVALID_ADDR, root->getLocalLabelAddr((SymbolID_t)110));
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyCJumpControlTree);
