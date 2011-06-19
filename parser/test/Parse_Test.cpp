
#include "hpParser.h"
#include <cppunit/extensions/HelperMacros.h>


using namespace Hayat::Parser;

class Parse_Test : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Parse_Test);
    CPPUNIT_TEST(test_Parse_seq_choice);
    CPPUNIT_TEST(test_Parse_many);
    CPPUNIT_TEST(test_Parse_range);
    CPPUNIT_TEST(test_Parse_prefix);
    CPPUNIT_TEST(test_Parse_postfix);
    CPPUNIT_TEST_SUITE_END();
public:
    Parse_Test(void) {}
        
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

    void test_Parse_seq_choice(void)
    {
        StringInputBuffer inp("abc");
        gpInp = &inp;

        Parser_Char* pc_a = new Parser_Char(L'a');
        Parser_Char* pc_b = new Parser_Char(L'b');
        Parser_Char* pc_c = new Parser_Char(L'c');
        Parser_String* ps_ab = new Parser_String(L"ab");

        Parser_Sequence* seq1 = new Parser_Sequence(pc_a, pc_b);
        CPPUNIT_ASSERT(seq1->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(2, (int)inp.getPos());

        inp.setPos(0);
        seq1 = new Parser_Sequence(pc_a, ps_ab);
        CPPUNIT_ASSERT(! seq1->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(0, (int)inp.getPos());

        inp.setPos(0);
        Parser_OrderedChoice* cho1 = new Parser_OrderedChoice(pc_a, pc_c);
        CPPUNIT_ASSERT(cho1->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(1, (int)inp.getPos());
        CPPUNIT_ASSERT(! cho1->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(1, (int)inp.getPos());
        CPPUNIT_ASSERT(pc_b->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(2, (int)inp.getPos());
        CPPUNIT_ASSERT(cho1->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(3, (int)inp.getPos());
    }

    void test_Parse_many(void)
    {
        StringInputBuffer inp("aaabbbbcccc");
        gpInp = &inp;

        Parser_Char* pc_a = new Parser_Char(L'a');
        Parser_Char* pc_b = new Parser_Char(L'b');
        Parser_Char* pc_c = new Parser_Char(L'c');

        Parser_Optional* opt = new Parser_Optional(pc_a);
        CPPUNIT_ASSERT(opt->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(1, (int) inp.getPos());
        CPPUNIT_ASSERT(opt->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(2, (int) inp.getPos());
        CPPUNIT_ASSERT(opt->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(3, (int) inp.getPos());
        CPPUNIT_ASSERT(opt->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(3, (int) inp.getPos());

        Parser_OneOrMore* plus = new Parser_OneOrMore(pc_b);
        CPPUNIT_ASSERT(plus->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(7, (int) inp.getPos());
        CPPUNIT_ASSERT(! plus->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(7, (int) inp.getPos());

        Parser_ZeroOrMore* star = new Parser_ZeroOrMore(pc_a);
        CPPUNIT_ASSERT(star->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(7, (int) inp.getPos());
        star = new Parser_ZeroOrMore(pc_c);
        CPPUNIT_ASSERT(star->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(11, (int) inp.getPos());
        CPPUNIT_ASSERT(star->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(11, (int) inp.getPos());

        Parser_AnyChar* dot = new Parser_AnyChar();
        CPPUNIT_ASSERT(! dot->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(11, (int) inp.getPos());
    }

    void test_Parse_range(void)
    {
        StringInputBuffer inp("1302468579");
        gpInp = &inp;

        Parser_RangeChar* r04 = new Parser_RangeChar(L'0', L'4');
        Parser_RangeChar* r59 = new Parser_RangeChar(L'5', L'9');

        CPPUNIT_ASSERT(r04->parse()->isValidTree());
        CPPUNIT_ASSERT(r04->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(2, (int) inp.getPos());
        Parser_OneOrMore* plus = new Parser_OneOrMore(r04);
        CPPUNIT_ASSERT(plus->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(5, (int) inp.getPos());
        CPPUNIT_ASSERT(!r04->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(5, (int) inp.getPos());

        CPPUNIT_ASSERT(r59->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(6, (int) inp.getPos());
        plus = new Parser_OneOrMore(r59);
        CPPUNIT_ASSERT(plus->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(10, (int) inp.getPos());

        Parser_EOF* p_eof = new Parser_EOF();
        CPPUNIT_ASSERT(p_eof->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(10, (int) inp.getPos());
    }

    void test_Parse_prefix(void)
    {
        StringInputBuffer inp("-+-1 2--3 +");
        gpInp = &inp;
        Parser_RangeChar* num = new Parser_RangeChar(L'0', L'9');
        Parser_OrderedChoice* op = new Parser_OrderedChoice
            (new Parser_Char(L'+'), new Parser_Char(L'-'));
                                                                
        Parser* my = new Parser();
        Parser_Prefix* p = new Parser_Prefix(my, num, op);
        CPPUNIT_ASSERT(p->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(4, (int) inp.getPos());
        CPPUNIT_ASSERT(! p->parse()->isValidTree());
        inp.advance(1);
        CPPUNIT_ASSERT_EQUAL(5, (int) inp.getPos());
        CPPUNIT_ASSERT(p->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(6, (int) inp.getPos());
        CPPUNIT_ASSERT(p->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(9, (int) inp.getPos());
        CPPUNIT_ASSERT(!p->parse()->isValidTree());
    }

    void test_Parse_postfix(void)
    {
        StringInputBuffer inp("-1+- 2--3 +");
        gpInp = &inp;
        Parser_RangeChar* num = new Parser_RangeChar(L'0', L'9');
        Parser_OrderedChoice* op = new Parser_OrderedChoice
            (new Parser_Char(L'+'), new Parser_Char(L'-'));
                                                                
        Parser* my = new Parser();
        Parser_Postfix* p = new Parser_Postfix(my, num, op);
        CPPUNIT_ASSERT(! p->parse()->isValidTree());
        inp.advance(1);
        CPPUNIT_ASSERT_EQUAL(1, (int) inp.getPos());
        CPPUNIT_ASSERT(p->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(4, (int) inp.getPos());
        CPPUNIT_ASSERT(! p->parse()->isValidTree());
        inp.advance(1);
        CPPUNIT_ASSERT_EQUAL(5, (int) inp.getPos());
        CPPUNIT_ASSERT(p->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(8, (int) inp.getPos());
        CPPUNIT_ASSERT(p->parse()->isValidTree());
        CPPUNIT_ASSERT_EQUAL(9, (int) inp.getPos());
        CPPUNIT_ASSERT(!p->parse()->isValidTree());
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Parse_Test);
