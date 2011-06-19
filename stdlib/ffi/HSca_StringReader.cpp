/*  -*- coding: sjis-dos -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_StringReader.h"

FFI_DEFINITION_START {


DEEP_FFI_FUNC(initialize)
{
    HMD_DEBUG_ASSERT(numArgs == 1);
    Value selfVal = context->pop();
    Value str = context->pop();

    StringReader* sr = selfVal.toCppObj<StringReader>();
    if (str.getType() == HC_String)
        sr->initialize(str.toString());
    else if (str.getType() == HC_StringBuffer)
        sr->initialize(str.toObj());
    else {
        StringBuffer* sb = new StringBuffer();
        sb->concat(context, str, 0);
        sr->initialize(sb->getObj());
    }
    context->push(selfVal);
}

hys32 FFI_FUNC(pos) (Value selfVal)
{
    StringReader* sr = selfVal.toCppObj<StringReader>();
    return sr->getPos();
}

hys32 FFI_FUNC_S(pos) (Value selfVal, hys32 newPos)
{
    StringReader* sr = selfVal.toCppObj<StringReader>();
    return sr->setPos(newPos);
}

bool FFI_FUNC(search) (Value selfVal, Value c)
{
    StringReader* sr = selfVal.toCppObj<StringReader>();
    if (c.getType() == HC_Int) {
        char s[2];
        s[0] = c.toInt(); s[1] = '\0';
        return sr->search(s);
    } else if (c.getType() == HC_String || c.getType() == HC_StringBuffer) {
        const char* p = c.toString();
        return sr->search(p);
    }
    return false;
}

hys32 FFI_FUNC(skip) (Value selfVal, Value c)
{
    StringReader* sr = selfVal.toCppObj<StringReader>();
    if (c.getType() == HC_Int) {
        return sr->skip(c.toInt());
    } else if (c.getType() == HC_String || c.getType() == HC_StringBuffer) {
        return sr->skip(c.toString());
    }
    return sr->getPos();
}

hys32 FFI_FUNC(getInt) (Value selfVal)
{
    StringReader* sr = selfVal.toCppObj<StringReader>();
    return sr->getInt();
}

hyf32 FFI_FUNC(getFloat) (Value selfVal)
{
    StringReader* sr = selfVal.toCppObj<StringReader>();
    return sr->getFloat();
}

FFI_GCMARK_FUNC()
{
    object->cppObj<StringReader>(HSym_StringReader)->m_GC_mark();
}

} FFI_DEFINITION_END
