/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYBYTECODEIF_H_
#define m_HYBYTECODEIF_H_

#include "hySignatureBase.h"

namespace Hayat {
    namespace Common {
        class BytecodeIf {
        public:
            virtual ~BytecodeIf() {}
            virtual void debugGetInfo(char* buf, hyu32 bufSize, const hyu8* ptr) const = 0;
            virtual const hyu8* codeStartAddr(void) const { return 0; }
            virtual const hyu8* getString(hyu32 offs) const = 0;
            virtual const SignatureBase::Sig_t* getSignatureBytes(hyu16 id) const = 0;
        };

    }
}

#endif /* m_HYBYTECODEIF_H_ */
