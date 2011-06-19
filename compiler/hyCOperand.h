/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCOPERAND_H_
#define m_HYCOPERAND_H_

#include "hySymbolID.h"
#include "hyCBytecode.h"

namespace Hayat {
    namespace Compiler {

#define DEF_OPERAND(opr,inittype)             \
        class OPR_##opr {                     \
        protected:                            \
            inittype val;                     \
        public:                               \
            typedef inittype BaseType_t;        \
            OPR_##opr(inittype v) { val = v; }  \
            void addCodeTo(Bytecode& b);        \
            static const size_t SIZE = sizeof(inittype);        \
        };


        DEF_OPERAND(INT, hys32);
        inline void OPR_INT::addCodeTo(Bytecode& b) { b.addCode<hys32>(val); }

        DEF_OPERAND(INT8, hys8);
        inline void OPR_INT8::addCodeTo(Bytecode& b) { b.addCode<hys8>(val); }

        DEF_OPERAND(FLOAT, hyf32);
        inline void OPR_FLOAT::addCodeTo(Bytecode& b) { b.addCode<hyf32>(val); }

        DEF_OPERAND(UINT8, hyu8);
        inline void OPR_UINT8::addCodeTo(Bytecode& b) { b.addCode<hyu8>(val); }

        DEF_OPERAND(UINT16, hyu16);
        inline void OPR_UINT16::addCodeTo(Bytecode& b) { b.addCode<hyu16>(val); }

        DEF_OPERAND(UINT32, hyu32);
        inline void OPR_UINT32::addCodeTo(Bytecode& b) { b.addCode<hyu32>(val); }

        DEF_OPERAND(STRCONST, hyu32);
        inline void OPR_STRCONST::addCodeTo(Bytecode& b) { b.addCode<hyu32>(val); }

        DEF_OPERAND(SYMBOL, SymbolID_t);
        inline void OPR_SYMBOL::addCodeTo(Bytecode& b) { b.addCode<SymbolID_t>(val); }

        DEF_OPERAND(METHOD, SymbolID_t);
        inline void OPR_METHOD::addCodeTo(Bytecode& b) { b.addCode<SymbolID_t>(val); }

        DEF_OPERAND(CLASS, SymbolID_t);
        inline void OPR_CLASS::addCodeTo(Bytecode& b) { b.addCode<SymbolID_t>(val); }

        DEF_OPERAND(LOCALVAR, hys16);
        inline void OPR_LOCALVAR::addCodeTo(Bytecode& b) { b.addCode<hys16>(val); }

        DEF_OPERAND(MAYLOCALVAR, hys16);
        inline void OPR_MAYLOCALVAR::addCodeTo(Bytecode& b) { b.addCode<hys16>(val); }

        DEF_OPERAND(MEMBVAR, SymbolID_t);
        inline void OPR_MEMBVAR::addCodeTo(Bytecode& b) { b.addCode<SymbolID_t>(val); }

        DEF_OPERAND(CLASSVAR, SymbolID_t);
        inline void OPR_CLASSVAR::addCodeTo(Bytecode& b) { b.addCode<SymbolID_t>(val); }

        DEF_OPERAND(GLOBALVAR, SymbolID_t);
        inline void OPR_GLOBALVAR::addCodeTo(Bytecode& b) { b.addCode<SymbolID_t>(val); }

        DEF_OPERAND(CONSTVAR, SymbolID_t);
        inline void OPR_CONSTVAR::addCodeTo(Bytecode& b) { b.addCode<SymbolID_t>(val); }

        DEF_OPERAND(RELATIVE, hys32);
        inline void OPR_RELATIVE::addCodeTo(Bytecode& b) { b.addCode<hys32>(val); }

        DEF_OPERAND(JUMPSYMBOL, SymbolID_t);
        inline void OPR_JUMPSYMBOL::addCodeTo(Bytecode& b) {
            b.addCode<SymbolID_t>(val);
#ifdef SYMBOL_ID_IS_HYU16
            b.addCode<hys16>(0);
#endif
        }

        DEF_OPERAND(SCOPE, TArray<SymbolID_t>*);
        // OPR_SCOPE::addCodeTo() is not inline

        DEF_OPERAND(SIGNATURE, hyu16);
        inline void OPR_SIGNATURE::addCodeTo(Bytecode& b) { b.addCode<hyu16>(val); }

#undef DEF_OPERAND

    }
}

#endif /* m_HYCOPERAND_H_ */
