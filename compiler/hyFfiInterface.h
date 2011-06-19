/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCFFITYPE_H_
#define m_HYCFFITYPE_H_

#include "hyArray.h"
#include "hpSubstr.h"
#include "hySymbolID.h"


using namespace Hayat::Common;
using namespace Hayat::Parser;


namespace Hayat {
    namespace Compiler {

        class FfiType {
            friend class FfiTypeMgr;

        public:
            static void* operator new(size_t);
            static void operator delete(void*);

            FfiType(const char* hClassName, const char* cppClassName, hyu32 fieldSize);
            ~FfiType();
            
            const char* name(void) const { return m_name; }
            const char* type(void) const { return m_type; }
            const char* to(void) const { return m_to; }
            const char* from(void) const { return m_from; }

        protected:
            typedef struct {
                const char* name;
                const char* type;
                const char* to;
                const char* from;
            } m_InitStruct;
            FfiType(const m_InitStruct&);

        protected:
            bool        m_memAllocFlag;
            const char* m_name; // not allocated
            char*       m_type;
            char*       m_to;
            char*       m_from;

        };


        class FfiTypeMgr {
        public:
            FfiTypeMgr(void);
            ~FfiTypeMgr();

            void initialize();
            void finalize();

            void createInterface(const char* hClassName, const char* cppClassName, hyu32 fieldSize);
            FfiType* find(Substr_st key);
            FfiType* find(const char* key);
            const FfiType& get(Substr_st key);
            const FfiType& get(const char* key);

        protected:
            static FfiType::m_InitStruct m_ini[];
            TArray<FfiType*>    m_interfaces;
        };

    }
}

#endif /* m_HYCFFITYPE_H_ */
