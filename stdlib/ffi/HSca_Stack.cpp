/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Stack.h"

FFI_DEFINITION_START {


// new(n)で生成したインスタンスの初期化 
ValueStack* FFI_FUNC(initialize) (Value selfVal, hys32 initCapacity)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    ValueStack* self = selfVal.toCppObj<ValueStack>(HSym_Stack);
    self->initialize((hyu32)initCapacity);
    return self;
}

hys32 FFI_FUNC(hashCode) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    // 中身を見ない手抜き関数
    return ((hys32)selfVal.data) >> 2;
}

ValueStack* FFI_FUNC(clean) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    ValueStack* self = selfVal.toCppObj<ValueStack>();
    self->clean();
    return self;
}

ValueStack* FFI_FUNC(push) (Value selfVal, Value x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    ValueStack* self = selfVal.toCppObj<ValueStack>();
    self->push(x);
    GC::writeBarrier(x);
    return self;
}

FFI_GCMARK_FUNC()
{
    ValueStack* s = object->cppObj<ValueStack>();
    for (hyu32 i = 1; i <= s->size(); i++) {
        Value& v = s->getNth(i);
        GC::markValue(v);
    }
}

} FFI_DEFINITION_END
