/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYSTRINGBOX_H_
#define m_HYSTRINGBOX_H_

/*
  String �̕ۊǏꏊ���Ǘ�����B
  �o�C�g�R�[�h�̃����[�h�ŁA�u�������o�C�g�R�[�h����String��
  �Q�Ƃ���Ă����ꍇ�A�����Ɉړ������B

  ���̒��ɕۊǂ��ꂽ������́A�t��GC���̂�GC�̑ΏۂƂȂ�B
*/


#include "hyArray.h"
#include "hyBitArray.h"


class Test_StringBox; // for unittest


using namespace Hayat::Common;

namespace Hayat {
    namespace Engine {

        class  StringBox {
            friend class ::Test_StringBox; // for unittest

        protected:
            TArray<const char*> m_strStorage;
            BitArray            m_markFlags;

        public:
            StringBox(void);
            ~StringBox() { finalize(); }

            void finalize(void);

            const char* store(const char* str);

            void unmark(void);  // GC����
            void mark(const char* str); // GC mark
            void sweep(void);

        };

    }
}

#endif /* m_HYSTRINGBOX_H_ */
