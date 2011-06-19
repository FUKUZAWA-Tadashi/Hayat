/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hayat.h"

using namespace Hayat;
using namespace Hayat::Engine;


// stdlibのffiテーブル
EXTERN_BYTECODE_FFI(stdlib);



// メモリ、GC等初期化
void
Engine::initMemory(void* pMemory, size_t memorySize)
{
    Common::MemPool::initGMemPool(pMemory, memorySize);
    gCodeManager.initialize();
    gGlobalVar.initialize();
    GC::initialize();
    ThreadManager::firstOfAll();
    gThreadManager.initialize();
    HClass::initializeRootFfiTable();
}


// stdlib初期化
// バイトコードは内部で管理されるので、最後のfinalizeも内部で行なわれる
void
Engine::initStdlib(const char* stdlibPath)
{
    LINK_BYTECODE_FFI(stdlib);
    Bytecode* stdlibBytecode = gCodeManager.readBytecode(stdlibPath);
    HMD_ASSERT(stdlibBytecode != NULL);
    Value::initStdlib(*stdlibBytecode);   // 標準値初期化
    Context* saveContext = VM::getContext();
    Context* context = gCodeManager.createContext();
    stdlibBytecode->initLinkBytecode(context, true); // 初期化実行
    gCodeManager.releaseContext(context);
    VM::setContext(saveContext);
    delete context; // stdlibの初期化ではContextが他から参照される事はない
}


// ライブラリ類バイトコード読み込み、初期化
// バイトコードは内部で管理されるので、最後のfinalizeも内部で行なわれる
SymbolID_t
Engine::readLibrary(const char* filename)
{
    Bytecode* pBytecode = gCodeManager.readBytecode(filename);
    Context* saveContext = VM::getContext();
    Context* context = gCodeManager.createContext();
    pBytecode->initLinkBytecode(context, true); // 初期化
    VM::setContext(saveContext);
    gCodeManager.releaseContext(context);
    return pBytecode->bytecodeSymbol();
}


// バイトコードの先頭から、スレッドで開始
ThreadID_t
Engine::startThread(Bytecode* pBytecode)
{
    ThreadID_t tid = gThreadManager.createThread();
    Thread* pThread = gThreadManager.id2thread(tid);
    Context* context = gCodeManager.createContext();
    pBytecode->initLinkBytecode(context, false);
    context->canSubstConst = true;
    context->callBytecodeTop(pBytecode);
    pThread->initialize(context);
    pThread->start();
    gCodeManager.releaseContext(context);
    return tid;
}


// スレッド強制終了
void
Engine::terminateThread(ThreadID_t tid)
{
    Thread* pThread = gThreadManager.id2thread(tid);
    if (pThread) pThread->terminate();
}


// 後始末
void
Engine::finalizeAll(void)
{
    GC::finalize();
    gThreadManager.finalize();
    gCodeManager.finalize();
    HClass::finalizeRootFfiTable();
    // gSymbolTable.finalize();
    Value::destroyStdlib();
}



void
Engine::initializeDebug(void* debugMemory, size_t debugMemorySize)
{
    if (debugMemory == NULL || debugMemorySize == 0)
        return;
    Debug::setDebugMemPool(MemPool::manage(debugMemory, debugMemorySize));
    // バイトコードのデバッグ情報(*.hdb)読み込み設定をデフォルトでtrueに
    Hayat::Engine::Bytecode::setFlagReadDebugInfo(true);
    MMes::initialize();
}

void
Engine::finalizeDebug(void)
{
    MMes::finalize();
    gSymbolTable.finalize();
}
