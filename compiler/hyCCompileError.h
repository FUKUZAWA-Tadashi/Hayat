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

        // �G���[�����[�j���O�����o��
        extern void outEventTitle(bool bErr);
        // �\�[�X�ʒu�����o��
        extern void outSourceInfo(hyu32 sourcePos);
        // ���b�Z�[�W���o��
        extern void outMessage(const char* fmt, ...);
        extern void v_outMessage(const char* fmt, va_list ap);


        extern void v_outEvent(bool ev, hyu32 sourcePos, const char* fmt, va_list ap);
        // ���[�j���O�o��
        extern void outWarning(hyu32 sourcePos, const char* fmt, ...);
        inline void v_outWarning(hyu32 sourcePos, const char* fmt, va_list ap) { v_outEvent(false, sourcePos, fmt, ap); }
        // �G���[�o��
        extern void outError(hyu32 sourcePos, const char* fmt, ...);
        inline void v_outError(hyu32 sourcePos, const char* fmt, va_list ap) { v_outEvent(true, sourcePos, fmt, ap); }

        // setCompileErrorPos()�ŃZ�b�g���ꂽ�ꏊ��\�����ăG���[�ɂ���
        extern void compileError(const char* fmt, ...);
        extern void compileWarning(const char* fmt, ...);
        // �G���[�̏ꏊ���Z�b�g
        extern void setCompileErrorPos(hyu32 sourcePos);
        extern void setCompileErrorPos(Hayat::Parser::SyntaxTree* st);
        extern hyu32 getCompileErrorPos(void);
        // �w��̏ꏊ��\�����ăG���[�ɂ���
        extern void compileError_pos(hyu32 sourcePos, const char* fmt, ...);
        extern void compileError_pos(Hayat::Parser::SyntaxTree* st, const char* fmt, ...);


        // �R���p�C�����Ŕ��������O�N���X
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
