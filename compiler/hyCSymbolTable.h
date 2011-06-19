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

        // �R���p�C�������ł̂ݎg�����[�J���ϐ��V���{������ʂ��₷�����邽�߂̒�`
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
            // str�ɑΉ�����Symbol ID��Ԃ��B������� SYMBOL_ID_ERROR ��Ԃ�
            SymbolID_t  check(const char* str, hyu32 len);
            // str�ɑΉ�����Symbol ID��Ԃ��B�����ɖ�����ΐV�K������
            SymbolID_t  symbolID(const char* str);
            // Substr�ɑΉ�����Symbol ID��Ԃ��B�����ɖ�����ΐV�K������
            SymbolID_t  symbolID(InputBuffer* inp, Substr& ss);
            // strStart<=.<strEnd�ɑΉ�����Symbol ID��Ԃ��B�����ɖ�����ΐV�K������
            SymbolID_t  symbolID(const char* strStart, const char* strEnd);

            // Symbol ID �ɑΉ����镶�����Ԃ�
            const char* id2str(SymbolID_t id);
            // �t�@�C������e�[�u����ǂݍ���: �t�@�C�������������� false
            bool        readFile(const char* path);
            // �t�@�C������e�[�u����ǂݍ��݃}�[�W����
            // �G���[������������false��Ԃ��B�t�@�C�����ǂ߂Ȃ������ꍇ��true
            bool        mergeFile(const char* path);
            // �o�b�t�@����e�[�u����ǂݍ��݃}�[�W����
            // �G���[��������������NULL��Ԃ��B����������e�[�u�������|�C���^
            const hyu8*   mergeTable(const hyu8* buf, hyu32 bufSize);
            // �e�[�u�����t�@�C���ɏo��
            void        writeFile(const char* path);
            // �V���{�����x���w�b�_�t�@�C�����쐬
            void        writeSymbolH(const char* path, bool verbose = false);

            // �e�[�u����FILE*�֏o��
            void        fwriteTable(FILE* fp);
            


            static const char* const SYMBOL_FILENAME;
            static const char* const SYMBOL_H_NAME;



            // �R���p�C�������Ŏg���V���{������ʂ��₷�����邽�߂̒�`
            LocalVarSymID_t  localVarSymID(const char* str) {
                return (LocalVarSymID_t)symbolID(str); }
            LocalVarSymID_t  localVarSymID(InputBuffer* inp, Substr& ss) {
                return (LocalVarSymID_t)symbolID(inp, ss); }
            const char*  localVarSymID2str(LocalVarSymID_t sym) {
                return id2str((SymbolID_t)sym); }

            hyu32 numSymbols(void) { return m_numSymbols; }


        protected:
            // �V���{�����x������
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
