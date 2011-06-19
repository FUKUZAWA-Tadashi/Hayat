/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HTPRINTOPCODE_H_
#define m_HTPRINTOPCODE_H_

#include "machdep.h"

namespace Hayat {
    namespace Common {
        class BytecodeIf;
    }
    namespace Tool {
        const hyu8* printOpcode(const hyu8* addr, const Hayat::Common::BytecodeIf* bif);
    }
}

#endif /* m_HTPRINTOPCODE_H_ */
