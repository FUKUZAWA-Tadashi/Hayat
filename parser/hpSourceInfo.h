
#ifndef m_SOURCEINFO_H_
#define m_SOURCEINFO_H_

#include "machdep.h"


namespace Hayat {
    namespace Parser {

        class SourceInfo {
        public:
            hyu32 line;
            hyu32 col;
            const char* fname;

            SourceInfo(void);
            ~SourceInfo() {}
            static void* operator new(size_t);
            static void operator delete(void*);

            void printns(char* dest, hyu32 destSize);
        };

    }
}
    
#endif /* m_SOURCEINFO_H_ */
