/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_BitArray.h"

FFI_DEFINITION_START {

// method "initialize"
BitArray* FFI_FUNC(initialize) (Value selfVal, hys32 size)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    BitArray* self = selfVal.toCppObj<BitArray>(HSym_BitArray);
    new((void*)self) BitArray(size); // コンストラクタ呼び出し
    return self;
}

// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
DEEP_FFI_FUNC(concatToStringBuffer)
{
    HMD_ASSERT((numArgs == 1) || (numArgs == 2));
    BitArray* self = context->popCppObj<BitArray>(HSym_BitArray);
    hys32 mode;
    if (numArgs == 2)
        mode = context->popInt();
    else
        mode = 0;
    StringBuffer* sb = context->popCppObj<StringBuffer>(HSym_StringBuffer);

    hys32 size = self->size();
    sb->concat("[");
    for (hys32 i = 0; i < size; i++) {
        sb->concat(self->getAt(i) ? "1" : "0");
    }
    sb->concat("]");
    if (mode < 0)
        sb->concat(":<BitArray>");
    context->pushObj(sb->getObj());
}

// method "[]"
DEEP_FFI_FUNC_X(5b5d)
{
    HMD_ASSERT(numArgs == 1);
    BitArray* self = context->popCppObj<BitArray>(HSym_BitArray);
    hys32 idx = context->popInt();
    if (idx < 0 || idx >= self->size()) {
        VM::getContext()->throwException(HSym_array_index_out_of_bounds,
                                         M_M("Array index out of bounds: index = %d, array size = %d"),
                                         idx, self->size());
        return;
    }
    context->pushBool(self->getAt(idx));
}

// method "[]="
DEEP_FFI_FUNC_X(5b5d3d)
{
    HMD_ASSERT(numArgs == 2);
    BitArray* self = context->popCppObj<BitArray>(HSym_BitArray);
    hys32 idx = context->popInt();
    Value xVal = context->pop();
    bool x;
    SymbolID_t xTypeSym = xVal.getTypeSymbol();
    if (xTypeSym == HSym_Bool) {
        x = xVal.toBool();
    } else if (xTypeSym == HSym_NilClass) {
        x = false;
    } else {
        x = true;
    }
    if (idx < 0 || idx >= self->size()) {
        VM::getContext()->throwException(HSym_array_index_out_of_bounds,
                                         M_M("Array index out of bounds: index = %d, array size = %d"),
                                         idx, self->size());
        return;
    }
    self->setAt(idx, x);
    context->pushBool(x);
}

// method "=="
bool FFI_FUNC_X(3d3d) (Value selfVal, Value otherVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    BitArray* self = selfVal.toCppObj<BitArray>(HSym_BitArray);
    if (otherVal.getTypeSymbol() != HSym_BitArray)
        return false;
    BitArray* other = otherVal.toCppObj<BitArray>(HSym_BitArray);
    return self->isSame(*other);
}

// method setAll
BitArray* FFI_FUNC(setAll) (Value selfVal, bool x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    BitArray* self = selfVal.toCppObj<BitArray>(HSym_BitArray);
    self->setAll(x);
    return self;
}

// method changeSize
DEEP_FFI_FUNC(changeSize)
{
    HMD_ASSERT(numArgs == 1);
    Value selfVal = context->pop();
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    BitArray* self = selfVal.toCppObj<BitArray>(HSym_BitArray);
    hys32 newSize = context->popInt();
    if (newSize < 0) {
        VM::getContext()->throwException(HSym_invalid_argument,
                                         M_M("argument of BitArray::changeSize() must >= 0"));
        return;
    }

    self->changeSize(newSize);
    context->push(selfVal);
}

// method setRange
 BitArray* FFI_FUNC(setRange) (Value selfVal, hys32 idx1, hys32 idx2, bool x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    BitArray* self = selfVal.toCppObj<BitArray>(HSym_BitArray);
    if (! self->setRange(idx1, idx2, x)) {
        VM::getContext()->throwException(HSym_array_index_out_of_bounds,
                                         M_M("Array index out of bounds: idx1 = %d, idx2 = %d, array size = %d"),
                                         idx1,idx2,self->size());
    }
    return self;
}

DEEP_FFI_FUNC(clone)
{
    HMD_ASSERT(numArgs == 0);
    Value selfVal = context->pop();
    const HClass* hc_BitArray = selfVal.getType();
    BitArray* self = selfVal.toCppObj<BitArray>(HSym_BitArray);
    Object* obj = Object::create(hc_BitArray, sizeof(BitArray) + sizeof(hyu32));
    BitArray* newBitArray = obj->cppObj<BitArray>();
    new((void*)newBitArray) BitArray(*self); // コピーコンストラクタ呼び出し
    context->pushObj(obj);
}

 
} FFI_DEFINITION_END
