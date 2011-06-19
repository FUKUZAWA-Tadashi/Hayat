/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYMETHOD_H_
#define m_HYMETHOD_H_

#include "hyValue.h"


/*
  Method�N���X

  �I�u�W�F�N�g�ƁA���\�b�h���̃y�A��ێ�����B

  call() �ɂ���āA�I�[�o�[���[�h�A�I�[�o�[���C�h���l�������K�؂�
  ���\�b�h���Ăяo���B
*/


namespace Hayat {
    namespace Engine {

        class Method {
        protected:
            Value       m_self;
            SymbolID_t  m_methodSym;

        public:
            Method(Value& self, SymbolID_t methodSym);
            ~Method() {}

            void initialize(Value& self, SymbolID_t methodSym);

            void call(Context* context, hyu8 numArgs);

        public:
            void m_GC_mark(void);

        };

    }
}


#endif /* m_HYMETHOD_H_ */
