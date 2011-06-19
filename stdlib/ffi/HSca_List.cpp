/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_List.h"

FFI_DEFINITION_START {

// class method "*cppSize"
DEEP_FFI_FUNC_X(2a63707053697a65)
{
#ifndef HMD_DEBUG
    (void)numArgs;
#endif
    context->pop(); // self
    HMD_DEBUG_ASSERT(numArgs == 0);
    context->pushInt(sizeof(ValueList));
}

// instance method "hashCode"
DEEP_FFI_FUNC(hashCode)
{
    HMD_ASSERT(numArgs == 0);
    hyu32 h = 0;
    for (ValueList* p = context->popList(); p != NULL; p = p->tail()) {
        h = h * 0x1f1f1f1f + p->head().hashCode(context);
    }
    context->pushInt(h);
}

// class method "cons"          List.cons(hd,tl)
// instance method "cons"       tl.cons(hd)
DEEP_FFI_FUNC(cons)
{
    Object* newObj = HC_List->createInstance_noInit(sizeof(ValueList));
    ValueList* newCell = newObj->cppObj<ValueList>();
    ValueList* tl;
    if (numArgs == 1) {
        // instance method  tl.cons(hd)
        tl = context->popList();
        newCell->setContent(context->pop());
    } else {
        HMD_ASSERT(numArgs == 2);
        // class method cons(hd, tl)
        context->pop(); // selfVal
        tl = context->popList();
        newCell->setContent(context->pop());
    }
    newCell->link(tl);

    context->pushList(newCell);
}


// instance method "concatToStringBuffer"
DEEP_FFI_FUNC(concatToStringBuffer)
{
    (void)numArgs;
    ValueList* p = context->popList();
    hys32 mode = context->popInt();
    StringBuffer* sb = context->popCppObj<StringBuffer>(HSym_StringBuffer);
    sb->concat("'(");
    while (p != NULL) {
        sb->concat(context, p->head(), mode);
        p = p->tail();
        if (p != NULL)
            sb->concat(",");
    }
    sb->concat(")");
    if (mode < 0)
        sb->concat(":<List>");
    context->pushObj(sb->getObj());
}

// instance method "empty?"
bool FFI_FUNC_Q(empty) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    return selfVal.toList() == NULL;
}

// instance method "head"
DEEP_FFI_FUNC(head)
{
    HMD_ASSERT(numArgs == 0);
    ValueList* self = context->popList();
    if (self == NULL) {
        context->throwException(HSym_empty_list, M_M("call head() of empty List"));
    } else {
        context->push(self->head());
    }
}

// instance method "tail"
DEEP_FFI_FUNC(tail)
{
    HMD_ASSERT(numArgs == 0);
    ValueList* self = context->popList();
    if (self == NULL) {
        context->throwException(HSym_empty_list, M_M("call tail() of empty List"));
    } else {
        context->pushList(self->tail());
    }
}

static ValueList* m_append(ValueList* p, ValueList* q)
{
    // pのうしろにqのコピーを連結、最後のセルのアドレスを返す
    while (q != NULL) {
        ValueList* newCell = HC_List->createInstance_noInit(sizeof(ValueList))->cppObj<ValueList>();
        newCell->setContent(q->head());
        p->link(newCell);
        p = newCell;
        q = q->tail();
    }
    p->link(NULL);
    return p;
}


// instance method "clone"
ValueList* FFI_FUNC(clone) (Value selfVal)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    ValueList* self = selfVal.toList();
    if (self == NULL)
        return NULL;

    ValueList* top = HC_List->createInstance_noInit(sizeof(ValueList))->cppObj<ValueList>();
    top->setContent(self->head());
    m_append(top, self->tail());
    return top;
}

// instance method "append"
ValueList* FFI_FUNC(append) (Value selfVal, ValueList* x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    ValueList* self = selfVal.toList();
    if ((self == NULL) && (x == NULL))
        return NULL;

    ValueList* top = HC_List->createInstance_noInit(sizeof(ValueList))->cppObj<ValueList>();
    if (self != NULL) {
        top->setContent(self->head());
        m_append(m_append(top, self->tail()), x);
    } else {
        top->setContent(x->head());
        m_append(top, x->tail());
    }
    return top;
}

// instance method "append!"
ValueList* FFI_FUNC_E(append) (Value selfVal, Value x)
{
    FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);
    ValueList* self = selfVal.toList();
    ValueList* newCell = HC_List->createInstance_noInit(sizeof(ValueList))->cppObj<ValueList>();
    newCell->setContent(x);
    if (self == NULL)
        return newCell;

    ValueList* p = self;
    ValueList* q;
    while ((q = p->tail()) != NULL)
        p = q;
    p->link(newCell);
    GC::writeBarrier(selfVal);
    return self;
}

// instance method "=="
DEEP_FFI_FUNC_X(3d3d)
{
    HMD_ASSERT(numArgs == 1);
    ValueList* p = context->popList();
    Value x = context->pop();
    if (! x.type->symCheck(HSym_List)) {
        context->push(FALSE_VALUE);
        return;
    }
    ValueList* q = (ValueList*) x.data;
    while ((p != NULL) && (q != NULL)) {
        if (p == q) {
            context->push(TRUE_VALUE);
            return;
        }
        if (! p->head().equals(context, q->head())) {
            context->push(FALSE_VALUE);
            return;
        }
        p = p->tail();
        q = q->tail();
    }
    context->pushBool(p == q);  // 等しいなら p == q == NULL
}

} FFI_DEFINITION_END
