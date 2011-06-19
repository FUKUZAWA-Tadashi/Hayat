/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYVALUEARRAY_H_
#define m_HYVALUEARRAY_H_

#include "hyArray.h"
#include "hyObject.h"


/*
  Value �� Array

  �g�p��)

    Object* obj = context->popObj(HSym_Array);
    ValueArray* va = obj->cppObj<ValueArray>();
    hys32 idx = context->popInt();
    Value* v = va->nth(idx);


  HSca_Array.cpp �ȂǂŎg�p���Ă���B
*/  

namespace Hayat {
    namespace Engine {

        class Context;
        class Value;

        class ValueArray : public TArray<Value> {
            friend class VM;        // ���� m_size �Ȃǂ�������
            
        public:
            ValueArray(hyu32 capacity = 0) { initialize(capacity); }
            ~ValueArray() { finalize(); }

            static void* operator new(size_t size);
            static void operator delete(void* p);

            // ���̔z�����������Object
            Object* getObj(void) { return Object::fromCppObj(this); }

            bool compare(Context*, ValueArray*);

            ValueArray* clone(void);
        };

    }
}

#endif /* m_HYVALUEARRAY_H_ */
