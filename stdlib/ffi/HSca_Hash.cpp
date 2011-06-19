/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Hash.h"

FFI_DEFINITION_START {

// new(bucketSize, loadFactor)で生成したインスタンスの初期化
Hash* FFI_FUNC(initialize) (Value selfVal, hys32 bucketSize, hyf32 loadFactor)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    Hash* self = selfVal.toCppObj<Hash>(HSym_Hash);
    self->initialize(bucketSize, (int)(loadFactor * 100.5f));
    return self;
}

// デストラクタ
void FFI_FUNC(finalize) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    selfVal.toCppObj<Hash>()->finalize();
}

hys32 FFI_FUNC(hashCode) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toCppObj<Hash>()->calcHashCode();
}

// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
DEEP_FFI_FUNC(concatToStringBuffer)
{
    (void)numArgs;
    Hash* self = context->popCppObj<Hash>(HSym_Hash);
    hys32 mode = context->popInt();
    StringBuffer* sb = context->popCppObj<StringBuffer>(HSym_StringBuffer);
    hyu32 size = self->size();
    sb->concat("{");
    Hash::Iterator itr(self);
    for (Hash::HashCell_t* pCell = itr.next(); pCell != NULL; pCell = itr.next()) {
        sb->concat(context, pCell->content().key, mode);
        sb->concat("=>");
        sb->concat(context, pCell->content().val, mode);
        if (--size > 0)
            sb->concat(",");
    }
    sb->concat("}");
    if (mode < 0)
        sb->concat(":<Hash>");
    context->pushObj(sb->getObj());
}

// "[]" keyに関連付けられた値を取り出す
DEEP_FFI_FUNC_X(5b5d)
{
    HMD_ASSERT(numArgs == 1);
    Hash* self = context->popCppObj<Hash>(HSym_Hash);
    Value key = context->pop();
    context->push(self->get(context, key));
}

// "[]=" 関連付け
DEEP_FFI_FUNC_X(5b5d3d)
{
    HMD_ASSERT(numArgs == 2);
    Hash* self = context->popCppObj<Hash>(HSym_Hash);
    Value key = context->pop();
    Value val = context->pop();
    self->put(context, key, val);
    GC::writeBarrier(key);
    GC::writeBarrier(val);
    context->push(val);
}

// 入っているデータの個数
DEEP_FFI_FUNC(size)
{
    HMD_ASSERT(numArgs == 0);
    Hash* self = context->popCppObj<Hash>(HSym_Hash);
    context->pushInt(self->size());
}

// "=="
DEEP_FFI_FUNC_X(3d3d)
{
    HMD_ASSERT(numArgs == 1);
    Hash* self = context->popCppObj<Hash>(HSym_Hash);
    Value x = context->pop();
    if (! x.getType()->symCheck(HSym_Hash)) {
        context->push(FALSE_VALUE);
        return;
    }
    Hash* o = x.toObj()->cppObj<Hash>();
    context->pushBool(self->equals(context, o));
}

DEEP_FFI_FUNC(rehash)
{
    HMD_ASSERT(numArgs == 0);
    Hash* self = context->popCppObj<Hash>(HSym_Hash);
    self->rehash(context);
    context->pushNil();
}

FFI_GCMARK_FUNC()
{
    object->cppObj<Hash>(HSym_Hash)->m_GC_mark();
}

} FFI_DEFINITION_END
