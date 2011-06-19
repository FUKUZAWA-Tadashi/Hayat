
#include "hpSyntaxTree.h"
#include <cppunit/extensions/HelperMacros.h>



using namespace Hayat::Parser;


class SyntaxTree_Test : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(SyntaxTree_Test);
    CPPUNIT_TEST(test_shrink);
    CPPUNIT_TEST_SUITE_END();
public:
    SyntaxTree_Test(void) {}
        
    void* houkenMemory;

    void setUp(void)
    {
        houkenMemory = HMD_ALLOC(1024 * 16);
        MemPool::initGMemPool(houkenMemory, 1024 * 16);
        SyntaxTree::initializePool(true);
    }

    void tearDown(void)
    {
        SyntaxTree::finalizePool();
        HMD_FREE(houkenMemory);
    }

    void test_shrink(void)
    {
        SyntaxTree* st1 = new SyntaxTree(NULL, 101,103);
        SyntaxTree* st2 = new SyntaxTree(NULL, 102,104);
        SyntaxTree* st3 = new SyntaxTree(NULL, 103,105);
        SyntaxTree* st4 = new SyntaxTree(NULL, 104,106);
        SyntaxTree* st5 = new SyntaxTree(NULL, 105,107);
        SyntaxTree* st6 = new SyntaxTree(NULL, 106,108);
        st1->childs = new SyntaxTree::Childs(3);
        st1->childs->add(st2);
        st1->childs->add(st3);
        st1->childs->add(st4);
        st3->childs = new SyntaxTree::Childs(2);
        st3->childs->add(st5);
        st3->childs->add(st6);

        CPPUNIT_ASSERT_EQUAL(3, st1->numChild());
        CPPUNIT_ASSERT(st1->get(0) == st2);
        CPPUNIT_ASSERT(st1->get(1) == st3);
        CPPUNIT_ASSERT(st1->get(2) == st4);
        CPPUNIT_ASSERT_EQUAL(2, st3->numChild());
        CPPUNIT_ASSERT(st3->get(0) == st5);
        CPPUNIT_ASSERT(st3->get(1) == st6);

        st1->erase(0);              // st2 deleted
        CPPUNIT_ASSERT(st1->get(0) == NULL);
        CPPUNIT_ASSERT_EQUAL(3, st1->numChild());
        st1->shrink();
        CPPUNIT_ASSERT(st1->get(0) == st3);
        CPPUNIT_ASSERT(st1->get(1) == st4);
        CPPUNIT_ASSERT_EQUAL(2, st1->numChild());

        st3->erase(1);              // st6 deleted
        CPPUNIT_ASSERT(st3->get(1) == NULL);
        CPPUNIT_ASSERT_EQUAL(2, st1->numChild());

        st1->erase(0);              // st3 deleted -> st5 deleted
        CPPUNIT_ASSERT(st1->get(0) == NULL);
        CPPUNIT_ASSERT_EQUAL(2, st1->numChild());
        st1->shrink();
        CPPUNIT_ASSERT(st1->get(0) == st4);
        CPPUNIT_ASSERT_EQUAL(1, st1->numChild());
            
        delete st1;                 // st1 deleted -> st4 deleted
    }
        
};
CPPUNIT_TEST_SUITE_REGISTRATION(SyntaxTree_Test);
