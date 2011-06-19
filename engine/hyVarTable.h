/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYVARTABLE_H_
#define m_HYVARTABLE_H_

#include "hyValue.h"

/*
  +-------------------------
  | hyu16 m_bufSize
  +-------------------------
  | hyu16 m_numVars
  +-------------------------
  | SymbolID_t* m_symbolTable
  +-------------------------
  | Value* m_varTable
  +-------------------------

  m_symbolTable
  +-------------------------
  | SymbolID_t
  |   × m_bufSize
  +-------------------------

  m_varTable
  +-------------------------
  | Value
  |   × m_bufSize
  +-------------------------
*/


class Test_hyGC;                // for test
class Test_hyGlobalVar;         // for test

namespace Hayat {
    namespace Engine {

        class VarTable {
            friend class GC;                    // for Garbage Colletion
            friend class ::Test_hyGC;           // for test
            friend class ::Test_hyGlobalVar;    // for test

        protected:
            hyu16             m_bufSize;
            hyu16             m_numVars;
            SymbolID_t*     m_symbolTable;
            Value*          m_varTable;

        public:
            static const int SIZE_INIT = 20; // 変数個数初期値

            VarTable(void);
            ~VarTable();

            void    initialize(int initSize = SIZE_INIT);
            void    finalize(void);
            Value*  getVarAddr_notCreate(SymbolID_t varSym);
            Value*  getVarAddr(SymbolID_t varSym);
        
        protected:
            static const int m_EXPAND_SIZE = 16; // 変数領域不足時の拡張サイズ
            void    m_insert(int idx, SymbolID_t varSym);
        public:
            void    m_GC_mark(void);

            // GCのmarkフェーズのインクリメンタル実行
            void            startMarkIncremental(void);
            void            markIncremental(void);
        };


        extern VarTable     gGlobalVar;

    }
}

#endif /* m_HYVARTABLE_H_ */
