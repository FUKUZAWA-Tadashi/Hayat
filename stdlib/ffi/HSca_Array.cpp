/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Array.h"
#include "hyPack.h"

FFI_DEFINITION_START {


// ../unittest/HSca_Array_Test.cpp でも同じマクロを使用しているので注意
//{
#define DEFAULT_ARRAYSIZE 16
//}


// new(n)で生成したインスタンスの初期化 
ValueArray* FFI_FUNC(initialize) (Value selfVal, hys32 initCapacity)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    ValueArray* self = selfVal.toCppObj<ValueArray>(HSym_Array);
    self->initialize(initCapacity);
    return self;
}

DEEP_FFI_FUNC(hashCode)
{
    HMD_ASSERT(numArgs == 0);
    ValueArray* self = context->popCppObj<ValueArray>(HSym_Array);
    hyu32 hash = 0;
    hyu32 size = self->size();
    for (hyu32 i = 0; i < size; i++) {
        hash = hash * 0x1f1f1f1f + self->nth(i).hashCode(context);
    }
    context->pushInt(hash);
}

// 中身を全部削除
ValueArray* FFI_FUNC(clear) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    ValueArray* self = selfVal.toCppObj<ValueArray>();
    self->clear();
    return self;
}

// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
DEEP_FFI_FUNC(concatToStringBuffer)
{
    HMD_ASSERT((numArgs == 1) || (numArgs == 2));
    ValueArray* self = context->popCppObj<ValueArray>(HSym_Array);
    hys32 mode;
    if (numArgs == 2)
        mode = context->popInt();
    else
        mode = 0;
    StringBuffer* sb = context->popCppObj<StringBuffer>(HSym_StringBuffer);

    hyu32 size = self->size();
    sb->concat("[");
    for (hyu32 i = 0; i < size; i++) {
        sb->concat(context, self->nth(i), mode);
        if (i < size - 1)
            sb->concat(",");
    }
    sb->concat("]");
    if (mode < 0)
        sb->concat(":<Array>");
    context->pushObj(sb->getObj());
}

// "[]" 値取り出し 
DEEP_FFI_FUNC_X(5b5d)
{
    HMD_ASSERT(numArgs == 1);
    ValueArray* self = context->popCppObj<ValueArray>(HSym_Array);
    hys32 idx = context->popInt();

    if (! self->checkIndex(idx)) {
        context->throwException(HSym_array_index_out_of_bounds,
                                M_M("Array index out of bounds: index = %d, array size = %d"),
                                idx, self->size());
        return;
    }
    if (idx >= (hys32)self->size())
        context->pushNil();
    else
        context->push(self->nth(idx));
}

// "[]=" 値代入 
DEEP_FFI_FUNC_X(5b5d3d)
{
    HMD_ASSERT(numArgs == 2);
    ValueArray* self = context->popCppObj<ValueArray>(HSym_Array);
    hys32 idx = context->popInt();
    Value x = context->pop();

    if (! self->checkIndex(idx)) {
        context->throwException(HSym_array_index_out_of_bounds,
                                M_M("Array index out of bounds: index = %d, array size = %d"),
                                idx, self->size());
        return;
    }
    self->subst(idx, x, NIL_VALUE);
    GC::writeBarrier(x);
    context->push(x);
}

// 入っているデータの個数
DEEP_FFI_FUNC(size)
{
    HMD_ASSERT(numArgs == 0);
    ValueArray* self = context->popCppObj<ValueArray>(HSym_Array);

    context->pushInt(self->size());
}

// "=="
DEEP_FFI_FUNC_X(3d3d)
{
    HMD_ASSERT(numArgs == 1);
    ValueArray* self = context->popCppObj<ValueArray>(HSym_Array);
    Value x = context->pop();
    if (! x.getType()->symCheck(HSym_Array)) {
        context->push(FALSE_VALUE);
        return;
    }
    ValueArray* o = x.toObj()->cppObj<ValueArray>();
    context->pushBool(self->compare(context, o));
}

// include?
DEEP_FFI_FUNC_Q(include)
{
    HMD_ASSERT(numArgs == 1);
    ValueArray* self = context->popCppObj<ValueArray>(HSym_Array);
    Value x = context->pop();
    bool result = false;
    for (int i = self->size(); --i >= 0; ) {
        if (x.equals(context, self->nth(i))) {
            result = true;
            break;
        }
    }
    context->pushBool(result);
}


// 値削除: 削除した値を返す
Value FFI_FUNC(removeAt) (Value selfVal, hys32 i)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    ValueArray* self = selfVal.toCppObj<ValueArray>();
    Value v = self->nth(i);
    self->remove(i);
    return v;
}


// == で比較して等しい値を削除: インデックスを返す。無かったら -1
DEEP_FFI_FUNC(remove)
{
    HMD_ASSERT(numArgs == 1);
    ValueArray* self = context->popCppObj<ValueArray>(HSym_Array);
    Value x = context->pop();
    hys32 size = (hys32) self->size();
    for (hys32 i = 0; i < size; ++i) {
        if (x.equals(context, self->nth(i))) {
            self->remove(i);
            context->pushInt(i);
            return;
        }
    }
    context->pushInt(-1);
}

// 値挿入
Value FFI_FUNC(insertAt) (Value selfVal, hys32 i, Value x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    ValueArray* self = selfVal.toCppObj<ValueArray>();
    self->insert(i) = x;
    GC::writeBarrier(x);
    return x;
}


ValueArray* FFI_FUNC(clone) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    ValueArray* self = selfVal.toCppObj<ValueArray>(HSym_Array);
    return self->clone();
}



static Context* compContext;    // multi thread unsafe
static Value* pCompFunc;        // multi thread unsafe
// x<=>y の比較関数
static int compar1(const Value* x, const Value* y)
{
    compContext->push(*y);
    compContext->execMethod(*x, HSymx_3c3d3e, 1); // <=>演算
    return compContext->popInt();
}
// compFunc(x,y) の比較関数
static int compar2(const Value* x, const Value* y)
{
    compContext->push(*x);
    compContext->push(*y);
    compContext->execMethod(*pCompFunc, HSymx_2829, 2); // ()演算
    return compContext->popInt();
}
DEEP_FFI_FUNC_E(sort)
{
    HMD_ASSERT(numArgs == 0 || numArgs == 1);
    ValueArray* self = context->popCppObj<ValueArray>(HSym_Array);
    compContext = context;
    if (numArgs == 0) {
        self->qsort(compar1);
    } else if (numArgs == 1) {
        Value compFunc = context->pop();
        pCompFunc = &compFunc;
        self->qsort(compar2);
    } else {
        HMD_FATAL_ERROR("Array#sort(): number of arguments must 0 or 1\n");
    }
    context->push(Value::fromObj(Object::fromCppObj(self)));
}

typedef struct { Value s; Value v; } V2;
static int comparV2(const V2* x, const V2* y)
{
    compContext->push(y->v);
    compContext->execMethod(x->v, HSymx_3c3d3e, 1); // <=>演算
    return compContext->popInt();
}

DEEP_FFI_FUNC_E(sortBy)
{
    HMD_ASSERT(numArgs == 1);
    ValueArray* self = context->popCppObj<ValueArray>(HSym_Array);
    Value fun = context->pop();

    hyu32 size = self->size();
    if (size > 0) {

        TArray<V2>* va = new TArray<V2>(size);
        va->addSpaces(size);

        for (hyu32 i = 0; i < size; ++i) {
            V2& c = va->nth(i);
            c.s = self->nth(i);
            context->push(c.s);
            context->execMethod(fun, HSymx_2829, 1); // ()演算
            c.v = context->pop();
        }
        compContext = context;
        va->qsort(comparV2);
    
        for (hyu32 i = 0; i < size; ++i) {
            self->nth(i) = va->nth(i).s;
        }

        delete va;

    }

    context->push(Value::fromObj(Object::fromCppObj(self)));
}


StringBuffer* FFI_FUNC(pack) (Value selfVal, const char* templ)
{
    ValueArray* self = selfVal.toCppObj<ValueArray>(HSym_Array);
    StringBuffer* sb = new StringBuffer();
    Pack::pack(templ, self, sb);
    return sb;
}


FFI_GCMARK_FUNC()
{
    ValueArray* va = object->cppObj<ValueArray>(HSym_Array);
    for (int i = va->size() - 1; i >= 0; --i) {
        GC::markValue(va->nth(i));
    }
}


} FFI_DEFINITION_END
