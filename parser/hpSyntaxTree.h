
#ifndef m_SYNTAXTREE_H_
#define m_SYNTAXTREE_H_

#include "hyArray.h"
#include "hpSubstr.h"


#define m_AT_USERACTION_DECL
#include "userActionFuncDef.h"
#undef m_AT_USERACTION_DECL


using namespace Hayat::Common;

namespace Hayat {
    namespace Parser {

        class Parser;
        class InputBuffer;

        class SyntaxTree {

        public:
#define m_IN_SYNTAXTREE_DECL
#include "userActionFuncDef.h"
#undef m_IN_SYNTAXTREE_DECL


        public:
            class Childs : public TArray<SyntaxTree*> {
            public:
                Childs(hyu32 capacity = 0);
                virtual ~Childs();
                static void* operator new(size_t size);
                static void operator delete(void* p);
                // delete all SyntaxTree recursively, and then delete itself
                static void deleteRecursively(Childs* p);
            };

        public:
            enum Valid_e {
                V_VALID,     // childs is valid
                V_INVALID,   // childs is not valid
                V_ERRORCUT,  // ErrorCut, errorCutId is valid
                V_FAIL,      // parse failed
                V_ERROR,     // parse error, errorCutId is valid
                V_FATAL_ERROR, // fatal error
            };
        public:
            static void* operator new(size_t);
            static void operator delete(void*);

            SyntaxTree(Valid_e v = V_INVALID);
            SyntaxTree(Parser* p, hyu32 startPos, hyu32 endPos, Childs* ch = NULL);
            SyntaxTree(Parser* p, Substr& ss, Childs* ch = NULL);
            SyntaxTree(Parser* p, Substr& ss, int errId); // create Error Cut
            SyntaxTree(hyu32 errPos, int errId, Parser* p); // create Error
            virtual ~SyntaxTree();

            int             numChild(void) { return (childs != NULL) ? childs->size() : 0; }
            SyntaxTree*     get(int idx) { return childs->nth(idx); }
            //SyntaxTree*     extract(int idx) { return childs->replace(idx, NULL); }
            SyntaxTree*     erase(int idx);         // delete child[idx]
            SyntaxTree*     shrink(void);           // pack all NULL gap
            void    deleteAllChild(void);

            // is childs valid ?
            bool            isValidTree(void) { return m_valid == V_VALID; }
            bool            isFail(void) { return (m_valid == V_FAIL) || isError(); }
            bool            isFailNotError(void) { return m_valid == V_FAIL; }
            bool            isError(void) { return (m_valid == V_ERROR) || (m_valid == V_FATAL_ERROR); }
            bool            isFatalError(void) { return m_valid == V_FATAL_ERROR; }
            bool            isErrorCut(void) { return m_valid == V_ERRORCUT; }

            // delete all childs recursively, and then delete itself
            static void     deleteRecursively(SyntaxTree* p);

        public:
            Substr          str;
            Parser*         parser;
            union {
                Childs*     childs;
                int         errorCutId;
            };
            int             chooseNum;    // set by ordered choice

        protected:
            Valid_e         m_valid;


        public:
            static void     initializePool(bool usePool);
            static void     finalizePool(void);
        protected:
            static bool     m_usePool;
            static TArray<TArray<SyntaxTree>* > m_syntaxTreeMemPool;
            static TArray<TArray<Childs>* > m_childsMemPool;
        };


        extern SyntaxTree* const m_PARSE_FAILED;   // parse result failed
        extern SyntaxTree* const m_NO_SYNTAX_TREE; // tree disposed
        extern SyntaxTree* const m_FATAL_PARSER_ERROR; // left recursion or something
        extern SyntaxTree* const m_NOT_PARSED_YET; // Memoize; not parsed yet
        extern SyntaxTree* const m_PARSING;        // Memoize; now parsing
    
    }
}
    
#endif /* m_SYNTAXTREE_H_ */
