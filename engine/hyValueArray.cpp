/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyValue.h"
#include "hyValueArray.h"


using namespace Hayat::Common;
using namespace Hayat::Engine;


void* ValueArray::operator new(size_t size)
{
#ifndef HMD_DEBUG
    (void)size;
#endif
    HMD_DEBUG_ASSERT(size == sizeof(ValueArray));
    Object* obj = Object::create(HC_Array, sizeof(ValueArray) + sizeof(hyu32));
    return (void*) obj->cppObj<ValueArray>(HSym_Array);
}

void ValueArray::operator delete(void* p)
{
    Object::fromCppObj(p)->destroy();
}


bool ValueArray::compare(Context* context, ValueArray* o)
{
    if (m_size != o->m_size)
        return false;

    Value* v1 = top();
    Value* v2 = o->top();
    for (hyu32 i = 0; i < m_size; ++i) {
        if (! (v1++)->equals(context, *v2++))
            return false;
    }
    return true;
}


ValueArray* ValueArray::clone(void)
{
    Object* obj = Object::create(HC_Array, sizeof(ValueArray) + sizeof(hyu32));
    ValueArray* x = obj->cppObj<ValueArray>(HSym_Array);
    x->initialize(m_capacity);
    memcpy(x->m_contents, m_contents, sizeof(Value)*m_size);
    x->m_size = m_size;
    return x;
}
