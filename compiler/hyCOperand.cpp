/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCOperand.h"
#include "hyCBytecode.h"

using namespace Hayat::Common;
using namespace Hayat::Compiler;

void OPR_SCOPE::addCodeTo(Bytecode& b)
{
    hyu32 n = val->size();
    b.addCode<hyu8>((hyu8)n);
    for (hyu32 i = 0; i < n; i++) {
        b.addCode<SymbolID_t>(val->nth(i));
    }
}
