
#ifndef m_PARSER_H_
#define m_PARSER_H_

#include "hpInputBuffer.h"
#include "hpSyntaxTree.h"
#include "hpMemoize.h"


class Scan_Test;

namespace Hayat {
    namespace Parser {

        // ========== base parser ==========
        class Parser {
            friend class ::Scan_Test;                 // for unittest

        public:
            static void* operator new(size_t size) { return gMemPool->alloc(size, "Pars"); }
            static void operator delete(void* p) { gMemPool->free(p); }

            Parser(void);
            virtual ~Parser() {}

            virtual SyntaxTree* createSyntaxTree(hyu32 startPos, SyntaxTree::Childs* childs = NULL);
            virtual SyntaxTree* createSyntaxTree(hyu32 startPos, hyu32 endPos, SyntaxTree::Childs* childs = NULL);

            virtual SyntaxTree* parse(void) { return m_FATAL_PARSER_ERROR; }
            virtual const char* name(void) { return NULL; }
            virtual void removeMemo(SyntaxTree*) {}

            virtual void addSequenceToParent(TArray<Parser*>* parentSeq);
            virtual void addSyntaxTreeToParent(SyntaxTree::Childs* arr, SyntaxTree* st);


            // initialize static parser settings
            static void initialize(int initHolderCapacity = 100);

            // delete everything
            static void finalize(void);

            // use memoization
            static void setUseMemoize(bool b) { m_bUseMemoize = b; }

            // print intermediate parsing result for debug
            // 0 = none, 1 = success only, >2 = detail
            static void setPrintIntermediateLevel(int level) { m_printIntermediateLevel = level; }

            static bool cmpStr(SyntaxTree* st, const char* str);


            Parser* seq(Parser* p);         // create Parser: Sequence
            Parser* choice(Parser* p);      // create Parser: Ordered Choice
            Parser* star(void);             // create Parser: Zero or More
            Parser* plus(void);             // create Parser: One or More
            Parser* optional(void);         // create Parser: Zero or One
            Parser* andPred(void);          // create Parser: And-predicate
            Parser* notPred(void);          // create Parser: Not-predicate

            Parser* noTree(void);           // create Parser: not create SyntaxTree

        public:
            static const int        MAX_MANY = 9999; // max limit n of m_parse_m2n 

        protected:
            SyntaxTree* m_parse_sequence(TArray<Parser*>* ps);
            SyntaxTree* m_parse_orderedChoice(TArray<Parser*>* ps);
            SyntaxTree* m_parse_m2n(Parser* p, int m, int n);
            SyntaxTree* m_parse_andPredicate(Parser* p);
            SyntaxTree* m_parse_notPredicate(Parser* p);
            SyntaxTree::Childs* m_parse_infix_common(Parser* exp, Parser* op, bool rep);
            static SyntaxTree::Childs* const m_INFIX_COMMON_FATAL_ERROR;
            SyntaxTree* m_parse_op_infixl(Parser* my, Parser* exp, Parser* op);
            SyntaxTree* m_parse_op_infixr(Parser* my, Parser* exp, Parser* op);
            SyntaxTree* m_parse_op_infixn(Parser* my, Parser* exp, Parser* op);
            SyntaxTree* m_parse_op_prefix(Parser* my, Parser* exp, Parser* op, bool allowRepeat = true);
            SyntaxTree* m_parse_op_postfix(Parser* my, Parser* exp, Parser* op, bool allowRepeat = true);
            SyntaxTree* m_parse_op_ternary(Parser* my, Parser* exp, Parser* op1, Parser* op2);
            SyntaxTree* m_parse_noTree(Parser* p);
            SyntaxTree* m_parse_string(const wchar_t* str);
            SyntaxTree* m_parse_char(wchar_t chr);
            SyntaxTree* m_parse_anyChar(void);
            SyntaxTree* m_parse_rangeChar(wchar_t c1, wchar_t c2);
            SyntaxTree* m_parse_EOF(void);

            void m_reduceSyntaxTreeToParent(SyntaxTree::Childs* arr, SyntaxTree* st);
            void m_reduceOpSyntaxTreeToParent(SyntaxTree::Childs* arr, SyntaxTree* st);

        protected:
            hyu32 m_curPos(void) { return gpInp->getPos(); }
            struct m_ErrorPos {
                hyu32 parsePos;
                int errIdx;
            };
            m_ErrorPos m_curErrPos(void); // current error position and parse position
            void m_back(m_ErrorPos& errPos); // back to parse start position
            void m_fail(m_ErrorPos& errPos); // add error log and m_back()
            SyntaxTree* m_fatalError(m_ErrorPos& errPos, const char* mes) {
                m_fail(errPos); HMD_PRINTF(mes); return m_FATAL_PARSER_ERROR; }

        public:
            struct Error_t {
                Parser*     parser;
                hyu32         pos;
            };
            static int      numErrors(void) { return m_errors->size(); }
            static Error_t  getError(int n) { return m_errors->nth(n); }

        protected:
            static TArray<Parser*>*         m_holder;
            static TArray<Error_t>*         m_errors;
            static bool                     m_bUseMemoize;
            static int                      m_printIntermediateLevel;

            static const char* m_sequenceName;
            static const char* m_orderedChoiceName;
        };


        // ========== user defined parser ==========
        class UserParser : public Parser {
        public:
            UserParser(const char* nam) : m_memo(), m_name(nam) {}
                virtual SyntaxTree* uParse(void) { return m_PARSE_FAILED; }

                SyntaxTree* parse(void);
                virtual void errorMessage(int errorCutId, hyu32 pos);
                virtual void actionAtParse(SyntaxTree*) {}
                const char* name(void) { return (m_name == NULL) ? "(noname)" : m_name; }

                void removeMemo(SyntaxTree*);
        protected:
                SyntaxTree* m_parse1(m_ErrorPos startPos);
                virtual SyntaxTree* m_parse2(m_ErrorPos startPos, SyntaxTree* st);

                Memoize m_memo;
                const char* m_name;
        };


        // ========== generated parser ==========
        class GenParser : public UserParser {
        public:
            GenParser(const char* nam) : UserParser(nam), m_parser(NULL) {}
                SyntaxTree* uParse(void);
                // virtual void construct(void); // setup m_parser
        protected:
                SyntaxTree* m_parse2(m_ErrorPos startPos, SyntaxTree* st);
                Parser* m_parser;
        };


        // ========== operator parser ==========
        class OperatorParser : public Parser {
        public:
            OperatorParser(const char* nam) : m_parser(NULL), m_name(nam) {}
                SyntaxTree* parse(void);
                const char* name(void) { return m_name; }
        protected:
                Parser* m_parser;
                const char* m_name;
        };


        // ========== error cut ==========
        class Parser_ErrorCut : public Parser {
        public:
            Parser_ErrorCut(int errorCutId) { m_errorCutId = errorCutId; }
            SyntaxTree* parse(void);
            const char* name(void) { return "(error cut)"; }
        protected:
            int     m_errorCutId;
        };

        // ========== sequence ==========
        class Parser_Sequence : public Parser {
        public:
            Parser_Sequence(Parser* ps[], int n);
            Parser_Sequence(Parser* p1, Parser* p2);
            ~Parser_Sequence() { delete m_ps; }
            void addSequenceToParent(TArray<Parser*>* parentSeq);
            void addSyntaxTreeToParent(SyntaxTree::Childs* arr, SyntaxTree* st) {
                m_reduceSyntaxTreeToParent(arr, st); }
            SyntaxTree* parse(void) { return m_parse_sequence(m_ps); }
            const char* name(void) { return m_sequenceName; }
            void add(Parser* p) { p->addSequenceToParent(m_ps); }
        protected:
            TArray<Parser*>* m_ps;
        };


        // ========== ordered choice ==========
        class Parser_OrderedChoice : public Parser {
        protected:
            static const char* m_choiceName;
        public:
            Parser_OrderedChoice(Parser* ps[], int n);
            Parser_OrderedChoice(Parser* p1, Parser* p2);
            ~Parser_OrderedChoice() { delete m_ps; }
            SyntaxTree* parse(void) { return m_parse_orderedChoice(m_ps); }
            const char* name(void) { return m_orderedChoiceName; }
            void add(Parser* p) { m_ps->add(p); }
        protected:
            TArray<Parser*>* m_ps;
        };


        // ========== many ==========
        template <int m, int n> class Parser_Many : public Parser {
        public:
            Parser_Many(Parser* p) : Parser(), m_p(p) {}
                void addSyntaxTreeToParent(SyntaxTree::Childs* arr, SyntaxTree* st) {
                    m_reduceSyntaxTreeToParent(arr, st); }
                SyntaxTree* parse(void) { return m_parse_m2n(m_p, m, n); }
                const char* name(void) { return "(many)"; }
        protected:
                Parser* m_p;
        };

        typedef Parser_Many<0,Parser::MAX_MANY> Parser_ZeroOrMore;
        typedef Parser_Many<1,Parser::MAX_MANY> Parser_OneOrMore;
        typedef Parser_Many<0,1>    Parser_Optional;


        // ========== predicate ==========
        class Parser_AndPredicate : public Parser {
        public:
            Parser_AndPredicate(Parser* p) : Parser(), m_p(p) {}
                SyntaxTree* parse(void) { return m_parse_andPredicate(m_p); }
                const char* name(void) { return "(andPred)"; }
        protected:
                Parser* m_p;
        };

        class Parser_NotPredicate : public Parser {
        public:
            Parser_NotPredicate(Parser* p) : Parser(), m_p(p) {}
                SyntaxTree* parse(void) { return m_parse_notPredicate(m_p); }
                const char* name(void) { return "(notPred)"; }
        protected:
                Parser* m_p;
        };	  


        // ========== operator ==========
        class Parser_InfixL : public Parser {
        public:
            Parser_InfixL(Parser* my, Parser* exp, Parser* op) : Parser(), m_my(my), m_exp(exp), m_op(op) {}
                SyntaxTree* parse(void) { return m_parse_op_infixl(m_my, m_exp, m_op); }
                const char* name(void) { return "(infixl)"; }
        protected:
                Parser* m_my;
                Parser* m_exp;
                Parser* m_op;
        };

        class Parser_InfixR : public Parser {
        public:
            Parser_InfixR(Parser* my, Parser* exp, Parser* op) : Parser(), m_my(my), m_exp(exp), m_op(op) {}
                SyntaxTree* parse(void) { return m_parse_op_infixr(m_my, m_exp, m_op); }
                const char* name(void) { return "(infixr)"; }
        protected:
                Parser* m_my;
                Parser* m_exp;
                Parser* m_op;
        };

        class Parser_InfixN : public Parser {
        public:
            Parser_InfixN(Parser* my, Parser* exp, Parser* op) : Parser(), m_my(my), m_exp(exp), m_op(op) {}
                SyntaxTree* parse(void) { return m_parse_op_infixn(m_my, m_exp, m_op); }
                const char* name(void) { return "(infixn)"; }
        protected:
                Parser* m_my;
                Parser* m_exp;
                Parser* m_op;
        };

        class Parser_Prefix : public Parser {
        public:
            Parser_Prefix(Parser* my, Parser* exp, Parser* op) : Parser(), m_my(my), m_exp(exp), m_op(op) {}
                SyntaxTree* parse(void) { return m_parse_op_prefix(m_my, m_exp, m_op); }
                const char* name(void) { return "(prefix)"; }
        protected:
                Parser* m_my;
                Parser* m_exp;
                Parser* m_op;
        };

        class Parser_Postfix : public Parser {
        public:
            Parser_Postfix(Parser* my, Parser* exp, Parser* op) : Parser(), m_my(my), m_exp(exp), m_op(op) {}
                SyntaxTree* parse(void) { return m_parse_op_postfix(m_my, m_exp, m_op); }
                const char* name(void) { return "(postfix)"; }
        protected:
                Parser* m_my;
                Parser* m_exp;
                Parser* m_op;
        };

        class Parser_Ternary : public Parser {
        public:
            Parser_Ternary(Parser* my, Parser* exp, Parser* op1, Parser* op2) : Parser(), m_my(my), m_exp(exp), m_op1(op1), m_op2(op2) {}
                SyntaxTree* parse(void) { return m_parse_op_ternary(m_my, m_exp, m_op1, m_op2); }
                const char* name(void) { return "(ternary)"; }
        protected:
                Parser* m_my;
                Parser* m_exp;
                Parser* m_op1;
                Parser* m_op2;
        };


        // ========== pseudo ==========
        class Parser_NoTree : public Parser {
        public:
            Parser_NoTree(Parser* p) : Parser(), m_p(p) {}
                SyntaxTree* parse(void) { return m_parse_noTree(m_p); }
                const char* name(void) { return "(noTree)"; }
        protected:
                Parser* m_p;
        };


        // ========== scanner ==========
        class Parser_String : public Parser {
        public:
            Parser_String(const wchar_t* wstr) : Parser(), m_wstr(wstr) {}
                SyntaxTree* parse(void) { return m_parse_string(m_wstr); }
                const char* name(void) { return "String"; }
        protected:
                const wchar_t* m_wstr;
        };

        class Parser_Char : public Parser {
        public:
            Parser_Char(wchar_t chr) : Parser() { m_chr = chr; }
                SyntaxTree* parse(void) { return m_parse_char(m_chr); }
                const char* name(void) { return "Char"; }
        protected:
                wchar_t m_chr;
        };

        class Parser_AnyChar : public Parser {
        public:
            Parser_AnyChar(void) : Parser() {}
                SyntaxTree* parse(void) { return m_parse_anyChar(); }
                const char* name(void) { return "AnyChar"; }
        };

        class Parser_EOF : public Parser {
        public:
            Parser_EOF(void) : Parser() {}
                SyntaxTree* parse(void) { return m_parse_EOF(); }
                const char* name(void) { return "EOF"; }
        };

        class Parser_RangeChar : public Parser {
        public:
            Parser_RangeChar(wchar_t c1, wchar_t c2) : Parser() {
                m_c1 = c1; m_c2 = c2; }
                SyntaxTree* parse(void) { return m_parse_rangeChar(m_c1, m_c2); }
                const char* name(void) { return "RangeChar"; }
        protected:
                wchar_t m_c1, m_c2;
        };

    }
}

#endif /* m_PARSER_H_ */
