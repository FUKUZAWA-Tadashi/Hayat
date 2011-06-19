
#ifndef m_MEMOIZE_H_
#define m_MEMOIZE_H_

#include "hyArray.h"


using namespace Hayat::Common;

namespace Hayat {
    namespace Parser {

        class SyntaxTree;
    
        class Memoize {
        public:
            Memoize(void);
            ~Memoize();

            SyntaxTree*     getAt(hyu32 pos);
            void            setAt(hyu32 pos, SyntaxTree* st);
            void            remove(SyntaxTree* st);  // remove memo including st
            int             size(void) { return m_memoArr.size(); }

        protected:
            int     m_search(hyu32 pos);

        protected:
            struct Memo {
                hyu32 pos;
                SyntaxTree* st;
            };
            TArray<Memo>    m_memoArr;       // sorted by pos
        };

    }
}

#endif /* m_MEMOIZE_H_ */
