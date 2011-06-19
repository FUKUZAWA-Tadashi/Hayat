/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Debug.h"
#include "hyDebug.h"

FFI_DEFINITION_START {

void FFI_FUNC(nop) (Value) {
    // ここにデバッガでブレークポイントを仕掛けるといった使い方がある
    (void)0;
}

// C++レベルで HMD_DEBUG がdefineされていれば true
bool FFI_FUNC_Q(debug) (Value) {
#ifdef HMD_DEBUG
    return true;
#else
    return false;
#endif
}

// 引数オブジェクトを並べてデバッグ表示
DEEP_FFI_FUNC(p)
{
#ifdef HMD_DEBUG
    context->pop(); // self
    context->debugPrintValues(numArgs, -1, ", ");
    HMD_PRINTF("\n");
#else
    context->stack.drop(numArgs+1);
#endif
    context->pushNil();         // 戻り値はnil
}

void FFI_FUNC(assert)  (Value, bool cond) {
#ifdef HMD_DEBUG
    if (!cond)
        VM::getContext()->throwError(M_M("assertion failed"));
#else
    (void)cond;
#endif
}

DEEP_FFI_FUNC(assertMsg) {
#ifdef HMD_DEBUG
    context->pop(); // self
    HMD_ASSERT(numArgs > 0);
    bool x = context->stack.getNth(numArgs).toBool();
    if (!x) {
        context->debugPrintValues(numArgs-1, 0, "");
        HMD_PRINTF("\n");
        context->throwError(M_M("assertion failed"));
    } else {
        context->stack.drop(numArgs);
        context->pushNil();
    }
#else
    context->stack.drop(numArgs+1);
    context->pushNil();
#endif
}

DEEP_FFI_FUNC(print) {
#ifdef HMD_DEBUG
    context->pop(); // self
    HMD_ASSERT(numArgs > 0);
    context->debugPrintValues(numArgs, 0, "");
    HMD_PRINTF("\n");
    context->pushNil();
#else
    context->stack.drop(numArgs+1);
    context->pushNil();
#endif
}

void FFI_FUNC(sayCommandStart) (Value, hys32, hys32)
{
}

DEEP_FFI_FUNC(sayCommand)
{
#ifdef HMD_DEBUG
    HMD_DEBUG_ASSERT(numArgs == 1);
    context->pop(); // self
    Value x = context->pop();
    StringBuffer* sb = new StringBuffer();
    sb->concat(context, x, 1);
    HMD_PRINTF(sb->top());
    delete sb;
#else
    (void)numArgs;
    context->stack.drop(2);
#endif
    context->pushNil();
}

void FFI_FUNC(sayCommandEnd) (Value)
{
#ifdef HMD_DEBUG
    HMD_PRINTF("\n");
#endif
}

void FFI_FUNC(setPrintOpcode) (Value, bool b) {
    Hayat::Engine::Debug::setPrintOpcode(b);
}

void FFI_FUNC(setPrintStack) (Value, bool b) {
    Hayat::Engine::Debug::setPrintStack(b);
}

void FFI_FUNC(printAllStackTrace) (Value) {
    Hayat::Engine::Debug::printAllStackTrace();
}

// GC用markフラグの状況を表示
void FFI_FUNC(printGCMark) (Value, Value val)
{
#ifdef HMD_DEBUG
    Hayat::Engine::GC::printGCMark(val);
#else
    (void)val;
#endif
}

void FFI_FUNC(printGCPhase) (Value)
{
#ifdef HMD_DEBUG
    Hayat::Engine::GC::printGCPhase();
#endif
}

void FFI_FUNC(dumpMemory) (Value)
{
#ifdef HMD_DEBUG
    gMemPool->printCellInfo();
#endif
}


} FFI_DEFINITION_END
