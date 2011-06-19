/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCCOMPILER_H_
#define m_HYCCOMPILER_H_

#include "hyCSymbolTable.h"

namespace Hayat {
    namespace Compiler {

        extern SymbolTable gLocalVarSymbols;
        extern void initializeCompiler(void);
        extern void finalizeCompiler(void);

        extern bool isJumpControlLabel(SymbolID_t label);

        // 0 = no warning
        // 1 = not found in package is WARNING
        // 2 = not found in same context is WARNING
        // 3 = not found in package is ERROR
        // 4 = not found in same context is ERROR
        extern int  jumpLabelCheckLevel;
    }
}

#endif /* m_HYCCOMPILER_H_ */
