/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYEXCEPTION_H_
#define m_HYEXCEPTION_H_

#include "hyCodePtr.h"
#include "hyArray.h"


namespace Hayat {

    namespace Engine {

        // 例外クラス

        class Exception {
            //friend class ::Hayat::Ffi::HSca_stdlib::HSca_Exception;
        public:
            typedef struct {
                const HClass* ownerClass;
                CodePtr ptr;
            } BackTraceInfo_st;
        protected:
            typedef TArray<BackTraceInfo_st> BackTrace;
            SymbolID_t      m_type;
            SymbolID_t      m_label;     // gotoなど場合の飛び先ラベル
            Value           m_val;
            BackTrace       m_bt;

        public:
            static void* operator new(size_t);
            static void operator delete(void*);
            Exception(SymbolID_t type = HSym_exception); // バックトレースバッファをallocしない
            Exception(SymbolID_t type, const Value& val);
            ~Exception() { finalize(); }

            void initialize(SymbolID_t type = HSym_exception, const Value& val = NIL_VALUE);
            void finalize(void);

            void    setType(SymbolID_t type) { m_type = type; }
            SymbolID_t type(void) { return m_type; }
            void    setVal(Value& val);
            void    setLabel(SymbolID_t label) { m_label = label; }
            SymbolID_t getLabel(void) { HMD_DEBUG_ASSERT(m_type == HSymR_jump_control || m_type == HSym_goto); return m_label; }
            const Value&   val(void) { return m_val; }

            void    setError(void) { m_type = HSym_fatal_error; }
            bool    isError(void) { return m_type == HSym_fatal_error; }

            hys32     backTraceSize(void) { return m_bt.size(); }
            void    addBackTrace(const CodePtr& ptr, const HClass* ownerClass);
            const BackTraceInfo_st* getBackTrace(int level);

#ifdef HMD_DEBUG
            void    debugPrint(Context* context);
#endif

        public:
            void    m_GC_mark(void);
        };

    }
}
#endif /* m_HYEXCEPTION_H_ */
