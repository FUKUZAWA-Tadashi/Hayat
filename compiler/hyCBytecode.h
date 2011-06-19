/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCBYTECODE_H_
#define m_HYCBYTECODE_H_

#include "hyPacking.h"
#include "hySymbolID.h"
#include "hyBMap.h"
#include <stdio.h>
#include "hpSourceInfo.h"

using namespace Hayat::Common;
using namespace Hayat::Parser;


class Test_hyCBytecode;

namespace Hayat {
    namespace Compiler {

        class Bytecode {
            friend class ::Test_hyCBytecode;

        public:

            Bytecode(void);
            ~Bytecode();

            void addCodes(const hyu8* codes, hyu32 size) {
                m_byteCodes.add(codes, size); }
            template<typename T> void addCode(T v) {
                packOut<T>(m_byteCodes, v); }

            bool removeLastCode(hyu8 code);
            void replaceCodes(hyu32 offs, const hyu8* codes, hyu32 size);

            hyu8 getCode(hyu32 offs);
            const hyu8* getCodes(hyu32 offs);
            hyu32 getSize(void);

            void setSignature(hyu16 defValOffs, hyu16 signatureID) {
                m_defValOffs = defValOffs;
                m_signatureID = signatureID; }
            void setNumLocalAlloc(hyu8 n) { m_numLocalAlloc = n; }
            void setOffset(hyu32 offset) { m_offset = offset; }
            hyu32 getOffset(void) { return m_offset; }
            hyu16 getSignatureID(void) { return m_signatureID; }
            hyu16 getDefaultValOffs(void) { return m_defValOffs; }

            void addJumpTable(TArray<hyu8>* table);
            void write(TArray<hyu8>* out);

            const SourceInfo* lastSourceInfo();
            const SourceInfo* getSourceInfoAt(hyu32 offs);
            SourceInfo* getSourceInfoToAdd();

            void fwriteDebugInfo(FILE* fp, TArray<const char*>& paths, hyu32 offs);

        protected:
            static const hyu32 m_INIT_CAPACITY = 1024;
            hyu32 m_offset;
            hyu16 m_signatureID;
            hyu16 m_defValOffs;
            hyu8  m_numLocalAlloc;
            hyu16 m_jumpTableCodeSize;
            TArray<hyu8>  m_byteCodes;
            TArray< TArray<hyu8>* >       m_jumpTables;
            BMap<hyu32, SourceInfo>       m_sourceInfos; // offs => SourceInfo

        };

        template<> inline void Bytecode::addCode<hyu8>(hyu8 v) {
            m_byteCodes.add(&v, 1); }
        template<> inline void Bytecode::addCode<hys8>(hys8 v) {
            m_byteCodes.add((hyu8*)&v, 1); }            
    }
}

#endif /* m_HYCBYTECODE_H_ */
