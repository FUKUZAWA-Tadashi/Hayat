/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCSYMBOLTABLE_H_
#define m_HYCSYMBOLTABLE_H_

#include "hpInputBuffer.h"
#include "hySymbolID.h"
#include <stdio.h>

using namespace Hayat::Common;
using namespace Hayat::Parser;


class Test_hyCSymbolTable;

namespace Hayat {
    namespace Compiler {

        // コンパイラ内部でのみ使うローカル変数シンボルを区別しやすくするための定義
        typedef SymbolID_t      LocalVarSymID_t;


        class SymbolTable {
            friend class ::Test_hyCSymbolTable;    // unittest 

        protected:
            static const hyu32 m_INIT_SYMOFFS_SIZE = 256;
            static const hyu32 m_INIT_SYMBOLBUF_SIZE = 5120;

        public:
            SymbolTable(void);
            ~SymbolTable();

            void        initialize(const char* const* initSymTable, hyu32 initSymOffsSize = m_INIT_SYMOFFS_SIZE, hyu32 initSymbolBufSize = m_INIT_SYMBOLBUF_SIZE);
            void        finalize(void);
            // strに対応するSymbol IDを返す。無ければ SYMBOL_ID_ERROR を返す
            SymbolID_t  check(const char* str, hyu32 len);
            // strに対応するSymbol IDを返す。既存に無ければ新規割当て
            SymbolID_t  symbolID(const char* str);
            // Substrに対応するSymbol IDを返す。既存に無ければ新規割当て
            SymbolID_t  symbolID(InputBuffer* inp, Substr& ss);
            // strStart<=.<strEndに対応するSymbol IDを返す。既存に無ければ新規割当て
            SymbolID_t  symbolID(const char* strStart, const char* strEnd);

            // Symbol ID に対応する文字列を返す
            const char* id2str(SymbolID_t id);
            // ファイルからテーブルを読み込む: ファイルが無かったら false
            bool        readFile(const char* path);
            // ファイルからテーブルを読み込みマージする
            // エラーが発生したらfalseを返す。ファイルが読めなかった場合はtrue
            bool        mergeFile(const char* path);
            // バッファからテーブルを読み込みマージする
            // エラーが発生した時はNULLを返す。成功したらテーブル末尾ポインタ
            const hyu8*   mergeTable(const hyu8* buf, hyu32 bufSize);
            // テーブルをファイルに出力
            void        writeFile(const char* path);
            // シンボルラベルヘッダファイルを作成
            void        writeSymbolH(const char* path, bool verbose = false);

            // テーブルをFILE*へ出力
            void        fwriteTable(FILE* fp);
            


            static const char* const SYMBOL_FILENAME;
            static const char* const SYMBOL_H_NAME;



            // コンパイラ内部で使うシンボルを区別しやすくするための定義
            LocalVarSymID_t  localVarSymID(const char* str) {
                return (LocalVarSymID_t)symbolID(str); }
            LocalVarSymID_t  localVarSymID(InputBuffer* inp, Substr& ss) {
                return (LocalVarSymID_t)symbolID(inp, ss); }
            const char*  localVarSymID2str(LocalVarSymID_t sym) {
                return id2str((SymbolID_t)sym); }

            hyu32 numSymbols(void) { return m_numSymbols; }


        protected:
            // シンボルラベル生成
            static void m_symLabel(char* buf, size_t bufSize, const char* str);

            hyu32         m_numSymbols;
            int*        m_symOffs;
            hyu32         m_symOffsSize;
            char*       m_symbolBuf;
            hyu32         m_symbolBufSize;

        };

    }
}
#endif /* m_HYCSYMBOLTABLE_H_ */
