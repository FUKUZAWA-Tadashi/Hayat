
#include "hpInputBuffer.h"
#include <cppunit/extensions/HelperMacros.h>


using namespace Hayat::Parser;

class InputBuffer_Test : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(InputBuffer_Test);
    CPPUNIT_TEST(test_pos2linecol);
    CPPUNIT_TEST_SUITE_END();
public:
    InputBuffer_Test(void) {}
        
    void* houkenMemory;

    void setUp(void)
    {
        houkenMemory = HMD_ALLOC(1024 * 16);
        MemPool::initGMemPool(houkenMemory, 1024 * 16);
    }

    void tearDown(void)
    {
        HMD_FREE(houkenMemory);
    }

    void test_pos2linecol(void)
    {
        StringInputBuffer inp("1\n2\r\n3\n\r5\r6\n\n8\r\r10\n\r\r\n13\n");
        //                     0               1               2   
        //                     01 23 4 56 7 89 01 2 34 5 678 9 0 1 234

        static int a[] = {1,1,2,2,2,3,3,4,5,5,
                          6,6,7,8,8,9,10,10,10,11,
                          12,12,13,13,13};
        static int b[] = {0,1,0,1,2,0,1,0,0,1,
                          0,1,0,0,1,0,0,1,2,0,
                          0,1,0,1,2};

        hyu32 line, col;
        for (int i = 0; i < (int)(sizeof(a)/sizeof(a[0])); i+=5) {
            CPPUNIT_ASSERT(inp.pos2linecol(i, &line, &col));
            CPPUNIT_ASSERT_EQUAL(a[i], (int)line);
            CPPUNIT_ASSERT_EQUAL(b[i], (int)col);
        }
        CPPUNIT_ASSERT(! inp.pos2linecol((sizeof(a)/sizeof(a[0])), &line, &col));
        for (int i = 0; i < (int)(sizeof(a)/sizeof(a[0])); i++) {
            CPPUNIT_ASSERT(inp.pos2linecol(i, &line, &col));
            CPPUNIT_ASSERT_EQUAL(a[i], (int)line);
            CPPUNIT_ASSERT_EQUAL(b[i], (int)col);
        }
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(InputBuffer_Test);
