/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYDEBUGSOURCEINFO_H_
#define m_HYDEBUGSOURCEINFO_H_

#include "hyDebug.h"
#include "hySymbolID.h"

using namespace Hayat::Common;

namespace Hayat {
    namespace Engine {


        struct DebugSourceInfo_t {
            const char* path;
            hyu16         line;
            hyu16         col;
        };

        extern DebugSourceInfo_t NULL_DebugSourceInfo;

        class DebugInfos {

        public:
            DebugInfos(void);
            DebugInfos(const char* fpath);
            DebugInfos(const hyu8* buf, hyu32 size);
            ~DebugInfos();
            void* operator new(size_t size);
            void operator delete(void* p);
            
            void    setBytecodeSymbol(SymbolID_t sym) { m_bytecodeSymbol = sym; }
            void    debugGetInfo(char* buf, hyu32 bufSize, hyu32 pos) const;
            DebugSourceInfo_t searchDebugInfo(hyu32 pos) const;
            void    readDebugInfo(const char* fpath);
            void    setDebugInfo(const hyu8* buf, hyu32 size);
            bool    wasInfoRead(void) const { return NULL != m_debInfos; }
            hyu32   size(void) const { return m_numDebInfos; }
            void    getNthInfo(char* buf, hyu32 bufSize, hyu32 n) const;

        protected:
            SymbolID_t  m_bytecodeSymbol;
            struct DebInfo_t {
                hyu32         addr;
                hyu16         pathId;
                hyu16         line;
                hyu16         col;
                hyu16         padding;
            } *m_debInfos;
            hyu8*     m_debFileBuf;
            hyu16     m_numDebInfos;
            hyu16     m_numPaths;
            const char*     m_paths;
            bool    m_haveDebugInfo;
        };

    }
}

#endif /* m_HYDEBUGSOURCEINFO_H_ */
