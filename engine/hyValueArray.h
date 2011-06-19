/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYVALUEARRAY_H_
#define m_HYVALUEARRAY_H_

#include "hyArray.h"
#include "hyObject.h"


/*
  Value の Array

  使用例)

    Object* obj = context->popObj(HSym_Array);
    ValueArray* va = obj->cppObj<ValueArray>();
    hys32 idx = context->popInt();
    Value* v = va->nth(idx);


  HSca_Array.cpp などで使用している。
*/  

namespace Hayat {
    namespace Engine {

        class Context;
        class Value;

        class ValueArray : public TArray<Value> {
            friend class VM;        // 直接 m_size などをいじる
            
        public:
            ValueArray(hyu32 capacity = 0) { initialize(capacity); }
            ~ValueArray() { finalize(); }

            static void* operator new(size_t size);
            static void operator delete(void* p);

            // この配列を所持するObject
            Object* getObj(void) { return Object::fromCppObj(this); }

            bool compare(Context*, ValueArray*);

            ValueArray* clone(void);
        };

    }
}

#endif /* m_HYVALUEARRAY_H_ */
