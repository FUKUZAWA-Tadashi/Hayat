/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


#ifndef m_HYVM_H_
#define m_HYVM_H_

#include "hyContext.h"
#include "hySignature.h"


namespace Hayat {
    namespace Engine {

        class VM {

        public:
            static Context* getContext(void) { return m_vm.m_context; }
            static void     setContext(Context* context) {
                // HMD__PRINTF_FK("VM::setContext(%x)\n",context);
                m_vm.m_context = context; }
            static void     exec1step(void) { m_vm.m_exec1step(); }
            static bool     haveCode(void) { return m_vm.m_haveCode(); }

            static Signature* getMultiSubstSignature(void) { return m_vm.m_multiSubstSignature; }
            static void setMultiSubstSignature(Signature* sig) { m_vm.m_multiSubstSignature = sig; }

#ifdef HMD_DEBUG
            static void     debugStackTrace(void);
#else
            static void     debugStackTrace(void) {}
#endif
#ifdef HY_ENABLE_BYTECODE_HISTORY
            static void     printOpcodeHistory(void);
            static void     clearOpcodeHistory(void);
            void            m_clearOpcodeHistory(void);
#else
            static void     printOpcodeHistory(void) {}
            static void     clearOpcodeHistory(void) {}
#endif

            static void     setAlertGCCallback(void (*callback)(void)) { m_vm.m_alertGCCallback = callback; }


        protected:
            VM(void);
            ~VM();
            static VM       m_vm;
            Context*        m_context;
            Signature*      m_multiSubstSignature;
            void            (*m_alertGCCallback)(void);

        protected:
            void    m_exec1step(void);
            void    m_waitTicks(hys32 ticks);
            bool    m_haveCode(void);

            typedef void    (VM::*OpcodeFunc_t)(void);
            static OpcodeFunc_t m_opcodeFuncs[];
#include "opcode_func_defs.inc"


        protected:
            void    m_printOpcode(const CodePtr& codePtr, const HClass* scope);

#ifdef HY_ENABLE_BYTECODE_HISTORY
            static const int m_MAX_OPCODE_HISTORY = 100;
            typedef struct {
                CodePtr codePtr;
                const HClass* scope;
            } HistoryInfo_st;
            HistoryInfo_st m_opcodeHistory[m_MAX_OPCODE_HISTORY];
            int     m_opcodeHistory_start;
            int     m_opcodeHistory_end;
            void    m_addOpcodeHistory(const CodePtr& codePtr, const HClass* scope);
            void    m_printOpcodeHistory(void);
#endif

        };

    }
}
#endif /* m_HYVM_H_ */
