/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCCOMPILEERROR_H_
#define m_HYCCOMPILEERROR_H_

#include "machdep.h"


namespace Hayat {
    namespace Parser {
        class SyntaxTree;
    }

    namespace Compiler {

        // エラーかワーニングかを出力
        extern void outEventTitle(bool bErr);
        // ソース位置情報を出力
        extern void outSourceInfo(hyu32 sourcePos);
        // メッセージを出力
        extern void outMessage(const char* fmt, ...);
        extern void v_outMessage(const char* fmt, va_list ap);


        extern void v_outEvent(bool ev, hyu32 sourcePos, const char* fmt, va_list ap);
        // ワーニング出力
        extern void outWarning(hyu32 sourcePos, const char* fmt, ...);
        inline void v_outWarning(hyu32 sourcePos, const char* fmt, va_list ap) { v_outEvent(false, sourcePos, fmt, ap); }
        // エラー出力
        extern void outError(hyu32 sourcePos, const char* fmt, ...);
        inline void v_outError(hyu32 sourcePos, const char* fmt, va_list ap) { v_outEvent(true, sourcePos, fmt, ap); }

        // setCompileErrorPos()でセットされた場所を表示してエラーにする
        extern void compileError(const char* fmt, ...);
        extern void compileWarning(const char* fmt, ...);
        // エラーの場所をセット
        extern void setCompileErrorPos(hyu32 sourcePos);
        extern void setCompileErrorPos(Hayat::Parser::SyntaxTree* st);
        extern hyu32 getCompileErrorPos(void);
        // 指定の場所を表示してエラーにする
        extern void compileError_pos(hyu32 sourcePos, const char* fmt, ...);
        extern void compileError_pos(Hayat::Parser::SyntaxTree* st, const char* fmt, ...);


        // コンパイラ内で発生する例外クラス
        class CException {
        public:
            CException(void) : message(NULL) {}
            CException(const char* mes) : message(mes) {}
            virtual ~CException() {}
            static void throwEx(const char* mes) {
                CException ex(mes);
                throw ex;
            }
        public:
            const char* message;
        };

    }
}

#endif /* m_HYCCOMPILEERROR_H_ */
