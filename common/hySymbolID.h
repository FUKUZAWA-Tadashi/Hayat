/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYSYMBOLID_H_
#define m_HYSYMBOLID_H_

#include "machdep.h"

namespace Hayat {
    namespace Common {

#ifdef SYMBOL_ID_IS_HYU16
        typedef hyu16 SymbolID_t;
        static const SymbolID_t SYMBOL_ID_ERROR = 0xffff;
#else
#ifdef SYMBOL_ID_IS_HYU32
        typedef hyu32 SymbolID_t;
        static const SymbolID_t SYMBOL_ID_ERROR = 0xffffffff;
#else
#error SYMBOL_ID_IS_HYU16 or SYMBOL_ID_IS_HYU32 must be specified in machdep.h
#endif
#endif
    
    }
}

#endif /* m_HYSYMBOLID_H_ */
