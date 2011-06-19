//  -*- coding: sjis-unix; -*-


#include "hpParser.h"
#include <cppunit/extensions/HelperMacros.h>


using namespace Hayat::Parser;

class Scan_Test : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Scan_Test);
    CPPUNIT_TEST(test_Scan);
    CPPUNIT_TEST(test_sjis);
    CPPUNIT_TEST(test_euc);
    CPPUNIT_TEST(test_utf8);
    CPPUNIT_TEST(test_skip);
    CPPUNIT_TEST_SUITE_END();
public:
    Scan_Test(void) {}
        
    void* houkenMemory;

    void setUp(void)
    {
        houkenMemory = HMD_ALLOC(1024 * 256);
        MemPool::initGMemPool(houkenMemory, 1024 * 256);
        SyntaxTree::initializePool(true);
        Parser::initialize();
    }

    void tearDown(void)
    {
        Parser::finalize();
        SyntaxTree::finalizePool();
        HMD_FREE(houkenMemory);
    }

    void test_Scan(void)
    {
        StringInputBuffer inp("123");
        gpInp = &inp;

        CPPUNIT_ASSERT_EQUAL(0, (int) inp.getPos());
            
        Parser* p = new Parser();
            
        CPPUNIT_ASSERT(p->m_parse_char(L'1')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(1, (int) inp.getPos());
        CPPUNIT_ASSERT(! p->m_parse_char(L'1')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(1, (int) inp.getPos());
            
        CPPUNIT_ASSERT(p->m_parse_char(L'2')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(2, (int) inp.getPos());
        CPPUNIT_ASSERT(! p->m_parse_char(L'2')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(2, (int) inp.getPos());
            
        inp.setPos(0);
        CPPUNIT_ASSERT_EQUAL(0, (int) inp.getPos());
        CPPUNIT_ASSERT(p->m_parse_string(L"12")->isValidTree());
        CPPUNIT_ASSERT_EQUAL(2, (int) inp.getPos());
        CPPUNIT_ASSERT(! p->m_parse_string(L"12")->isValidTree());
        CPPUNIT_ASSERT_EQUAL(2, (int) inp.getPos());
            
        inp.setPos(0);
        CPPUNIT_ASSERT(! p->m_parse_string(L"13")->isValidTree());
        CPPUNIT_ASSERT_EQUAL(0, (int) inp.getPos());
            
        CPPUNIT_ASSERT(p->m_parse_string(L"12")->isValidTree());
        CPPUNIT_ASSERT_EQUAL(2, (int) inp.getPos());
        CPPUNIT_ASSERT(p->m_parse_anyChar()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(3, (int) inp.getPos());
        CPPUNIT_ASSERT(! p->m_parse_anyChar()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(3, (int) inp.getPos());
        CPPUNIT_ASSERT(! p->m_parse_anyChar()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(3, (int) inp.getPos());
            
        inp.setPos(0);
        CPPUNIT_ASSERT(p->m_parse_rangeChar(L'1', L'2')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(1, (int) inp.getPos());
        CPPUNIT_ASSERT(p->m_parse_rangeChar(L'1', L'2')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(2, (int) inp.getPos());
        CPPUNIT_ASSERT(! p->m_parse_rangeChar(L'1', L'2')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(2, (int) inp.getPos());
    }

    void test_sjis(void)
    {
        StringInputBuffer inp("1\202\1202\202\1213\202\122\\1\201\165\201\166");
        gpInp = &inp;
        CPPUNIT_ASSERT(MBCSConv::initialize("SJIS"));
        CPPUNIT_ASSERT_EQUAL(0, (int) inp.getPos());
        Parser* p = new Parser();
        const char* cp;
        wchar_t wc, wc2;
        hyu32 len;

        CPPUNIT_ASSERT(p->m_parse_char(L'1')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(1, (int) inp.getPos());
        cp = "\202\120";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(2, (int)len);
        CPPUNIT_ASSERT(p->m_parse_char(wc));
        CPPUNIT_ASSERT_EQUAL(3, (int) inp.getPos());

        CPPUNIT_ASSERT(p->m_parse_anyChar()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(4, (int) inp.getPos());
        cp = "\202\121";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(2, (int)len);
        CPPUNIT_ASSERT(p->m_parse_anyChar()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(6, (int) inp.getPos());

        CPPUNIT_ASSERT(p->m_parse_rangeChar(L'1', L'5')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(7, (int) inp.getPos());
        cp = "\202\120";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(2, (int)len);
        cp = "\202\124";
        wc2 = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(2, (int)len);
        CPPUNIT_ASSERT(p->m_parse_rangeChar(wc,wc2)->isValidTree());
        CPPUNIT_ASSERT_EQUAL(9, (int) inp.getPos());

        cp = "\\1";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(1, (int)len);
        CPPUNIT_ASSERT(wc == L'\\');
        wc = inp.getChar();
        CPPUNIT_ASSERT(wc == L'\\');

        cp = "1";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(1, (int)len);
        CPPUNIT_ASSERT(wc == L'1');
        wc = inp.getChar();
        CPPUNIT_ASSERT(wc == L'1');

        cp = "\201\165";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(2, (int)len);
        CPPUNIT_ASSERT_EQUAL(L'「', wc);
        wc = inp.getChar();
        CPPUNIT_ASSERT_EQUAL(L'「', wc);

        cp = "\201\166";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(2, (int)len);
        CPPUNIT_ASSERT_EQUAL(L'」', wc);
        wc = inp.getChar();
        CPPUNIT_ASSERT_EQUAL(L'」', wc);

        MBCSConv::finalize();
    }

    void test_euc(void)
    {
        StringInputBuffer inp("1\243\2612\243\2623\243\263\\1\241\326\241\327");
        gpInp = &inp;
        CPPUNIT_ASSERT(MBCSConv::initialize("EUCJP"));
        CPPUNIT_ASSERT_EQUAL(0, (int) inp.getPos());
        Parser* p = new Parser();
        const char* cp;
        wchar_t wc, wc2;
        hyu32 len;

        CPPUNIT_ASSERT(p->m_parse_char(L'1')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(1, (int) inp.getPos());
        cp = "\243\261";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(2, (int)len);
        CPPUNIT_ASSERT(p->m_parse_char(wc)->isValidTree());
        CPPUNIT_ASSERT_EQUAL(3, (int) inp.getPos());

        CPPUNIT_ASSERT(p->m_parse_anyChar()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(4, (int) inp.getPos());
        cp = "\243\262";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(2, (int)len);
        CPPUNIT_ASSERT(p->m_parse_anyChar()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(6, (int) inp.getPos());

        CPPUNIT_ASSERT(p->m_parse_rangeChar(L'1', L'5')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(7, (int) inp.getPos());
        cp = "\243\261";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(2, (int)len);
        cp = "\243\265";
        wc2 = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(2, (int)len);
        CPPUNIT_ASSERT(p->m_parse_rangeChar(wc,wc2)->isValidTree());
        CPPUNIT_ASSERT_EQUAL(9, (int) inp.getPos());

        wc = inp.getChar();
        CPPUNIT_ASSERT(wc == L'\\');
        wc = inp.getChar();
        CPPUNIT_ASSERT(wc == L'1');

        cp = "\241\326";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(2, (int)len);
        CPPUNIT_ASSERT_EQUAL(L'「', wc);
        wc = inp.getChar();
        CPPUNIT_ASSERT_EQUAL(L'「', wc);

        cp = "\241\327";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(2, (int)len);
        CPPUNIT_ASSERT_EQUAL(L'」', wc);
        wc = inp.getChar();
        CPPUNIT_ASSERT_EQUAL(L'」', wc);

        MBCSConv::finalize();
    }

    void test_utf8(void)
    {
        StringInputBuffer inp("1\357\274\2212\357\274\2223\357\274\223\\1\343\200\214\343\200\215");
        gpInp = &inp;
        CPPUNIT_ASSERT(MBCSConv::initialize("UTF-8"));
        CPPUNIT_ASSERT_EQUAL(0, (int) inp.getPos());
        Parser* p = new Parser();
        const char* cp;
        wchar_t wc, wc2;
        hyu32 len;

        CPPUNIT_ASSERT(p->m_parse_char(L'1')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(1, (int) inp.getPos());
        cp = "\357\274\221";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(3, (int)len);
        CPPUNIT_ASSERT(p->m_parse_char(wc)->isValidTree());
        CPPUNIT_ASSERT_EQUAL(4, (int) inp.getPos());

        CPPUNIT_ASSERT(p->m_parse_anyChar()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(5, (int) inp.getPos());
        cp = "\357\274\222";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(3, (int)len);
        CPPUNIT_ASSERT(p->m_parse_anyChar()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(8, (int) inp.getPos());

        CPPUNIT_ASSERT(p->m_parse_rangeChar(L'1', L'5')->isValidTree());
        CPPUNIT_ASSERT_EQUAL(9, (int) inp.getPos());
        cp = "\357\274\221";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(3, (int)len);
        cp = "\357\274\225";
        wc2 = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(3, (int)len);
        CPPUNIT_ASSERT(p->m_parse_rangeChar(wc,wc2)->isValidTree());
        CPPUNIT_ASSERT_EQUAL(12, (int) inp.getPos());

        wc = inp.getChar();
        CPPUNIT_ASSERT(wc == L'\\');
        wc = inp.getChar();
        CPPUNIT_ASSERT(wc == L'1');

        cp = "\343\200\214";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(3, (int)len);
        CPPUNIT_ASSERT_EQUAL(L'「', wc);
        wc = inp.getChar();
        CPPUNIT_ASSERT_EQUAL(L'「', wc);

        cp = "\343\200\215";
        wc = MBCSConv::getCharAt(&cp, &len);
        CPPUNIT_ASSERT_EQUAL(3, (int)len);
        CPPUNIT_ASSERT_EQUAL(L'」', wc);
        wc = inp.getChar();
        CPPUNIT_ASSERT_EQUAL(L'」', wc);

        MBCSConv::finalize();
    }


    void m_test_skip_sub(const char* str,
                        int ident,int low,int cap,int up)
    {
        StringInputBuffer inp(str);
        CPPUNIT_ASSERT_EQUAL(0, (int) inp.getPos());
        if (ident > 0) {
            CPPUNIT_ASSERT(inp.skip_ident());
            CPPUNIT_ASSERT_EQUAL(ident, (int) inp.getPos());
        } else {
            CPPUNIT_ASSERT(! inp.skip_ident());
            CPPUNIT_ASSERT_EQUAL(0, (int) inp.getPos());
        }
        inp.setPos(0);
        if (low > 0) {
            CPPUNIT_ASSERT(inp.skip_identL());
            CPPUNIT_ASSERT_EQUAL(low, (int) inp.getPos());
        } else {
            CPPUNIT_ASSERT(! inp.skip_identL());
            CPPUNIT_ASSERT_EQUAL(0, (int) inp.getPos());
        }
        inp.setPos(0);
        if (cap > 0) {
            CPPUNIT_ASSERT(inp.skip_identC());
            CPPUNIT_ASSERT_EQUAL(cap, (int) inp.getPos());
        } else {
            CPPUNIT_ASSERT(! inp.skip_identC());
            CPPUNIT_ASSERT_EQUAL(0, (int) inp.getPos());
        }
        inp.setPos(0);
        if (up > 0) {
            CPPUNIT_ASSERT(inp.skip_identU());
            CPPUNIT_ASSERT_EQUAL(up, (int) inp.getPos());
        } else {
            CPPUNIT_ASSERT(! inp.skip_identU());
            CPPUNIT_ASSERT_EQUAL(0, (int) inp.getPos());
        }                
    }

    void test_skip(void)
    {
        StringInputBuffer inp("");
        gpInp = &inp;

        //              012345678901234567
        m_test_skip_sub("low_abc123_45zzABC+-", 18, 18, 0, 0);
        m_test_skip_sub("CapAbc123deF_g4_()", 16, 0, 16, 0);
        m_test_skip_sub("UPPER_12_ABC FOO", 12, 0, 0, 12);
        m_test_skip_sub("_under_9AB==1", 10, 10, 0, 0);
        m_test_skip_sub("_UNDER_9AB==1", 10, 10, 0, 0);
        m_test_skip_sub("azAZ0_9$",    7, 7, 0, 0);
        m_test_skip_sub("_0Zza#",      5, 5, 0, 0);
        m_test_skip_sub("_ABCdef",     7, 7, 0, 0);
        m_test_skip_sub("FooBar09",    8, 0, 8, 0);
        m_test_skip_sub("BAR_BAZ_9$",  9 ,0, 0, 9);
        m_test_skip_sub("BAR_BAZ_9z",  10,0, 10,0);
        m_test_skip_sub("Bar_BAZ_0$",  9 ,0, 9, 0);
        m_test_skip_sub("1290afAFgh",  0, 0, 0, 0);
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Scan_Test);
