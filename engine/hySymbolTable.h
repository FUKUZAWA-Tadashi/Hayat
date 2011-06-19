/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


//
// Engine�Ŏg�p����V���{���e�[�u��
//
// �e�[�u����Debug::setDebugMemPool()�Ŏw�肳�ꂽ�������̈�ɓǂݍ��ށB
// �w�肪�Ȃ����gMemPool�ɓǂݍ��ށB


#ifndef m_HYSYMBOLTABLE_H_
#define m_HYSYMBOLTABLE_H_

#include "hySymbolID.h"

using namespace Hayat::Common;

namespace Hayat {
    namespace Engine {

        class SymbolTable {
        public:
            SymbolTable(void);
            int             numSymbols(void) { return m_numSymbols; }
            int             tableSize(void) { return m_tableSize; }
            const hyu8*     id2str(SymbolID_t id);
            void            id2str_n(hyu8* buf, int bufSize, SymbolID_t id);  // symbol{id} �`��
            SymbolID_t      symbolID(const char* str, hyu32 len);
            SymbolID_t      symbolID(const char* str);
            void            readTable(const hyu8* buf);
            void            takeTable(const hyu8* buf, hyu32 size);
            bool            readFile(const char* fpath);
            void            finalize(void);
            bool            isValidID(SymbolID_t id) { return (id < m_numSymbols); }
        
            /// �����V���{�������邩�ǂ����`�F�b�N�����ɃV���{����ǉ�
            SymbolID_t      addSymbol(const hyu8* str);

        protected:
            int             m_numSymbols;
            int             m_tableSize;
            const hyu8*       m_table;
            hyu8*             m_tableBottom;
            hyu8*             m_fileBuf;
            const hyu8**    m_id2str;

        };



        extern SymbolTable  gSymbolTable;

    }
}

#endif /* m_HYSYMBOLTABLE_H_ */

