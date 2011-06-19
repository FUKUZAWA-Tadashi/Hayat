/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYPACKING_H_
#define m_HYPACKING_H_

#include "hyArray.h"
#include "hyEndian.h"

namespace Hayat {
    namespace Common {

        template<typename T> void packOut(TArray<hyu8>& out, T val) {
            Endian::pack<T>(out.addSpaces(sizeof(T)), val);
        }
        template<typename T> void packOut(TArray<hyu8>* pOut, T val) {
            Endian::pack<T>(pOut->addSpaces(sizeof(T)), val);
        }

    }
}


#endif /* m_HYPACKING_H_ */
