/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_SUBSTR_H_
#define m_SUBSTR_H_

namespace Hayat {
    namespace Parser {

        struct Substr_st {
            hyu32 startPos;
            hyu32 endPos;
            hyu32 len(void) { return endPos - startPos; }
        };

        class Substr : public Substr_st {
        public:
            Substr(void) { startPos = endPos = 0; }
            Substr(hyu32 st, hyu32 ed) { startPos = st; endPos = ed; }
            bool isEmpty(void) { return ((startPos == 0) && (endPos == 0)); }
        };      

    }
}

#endif /* m_SUBSTR_H_ */
