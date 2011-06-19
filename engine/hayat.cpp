/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hayat.h"

using namespace Hayat;
using namespace Hayat::Engine;


// stdlib��ffi�e�[�u��
EXTERN_BYTECODE_FFI(stdlib);



// �������AGC��������
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


// stdlib������
// �o�C�g�R�[�h�͓����ŊǗ������̂ŁA�Ō��finalize�������ōs�Ȃ���
void
Engine::initStdlib(const char* stdlibPath)
{
    LINK_BYTECODE_FFI(stdlib);
    Bytecode* stdlibBytecode = gCodeManager.readBytecode(stdlibPath);
    HMD_ASSERT(stdlibBytecode != NULL);
    Value::initStdlib(*stdlibBytecode);   // �W���l������
    Context* saveContext = VM::getContext();
    Context* context = gCodeManager.createContext();
    stdlibBytecode->initLinkBytecode(context, true); // ���������s
    gCodeManager.releaseContext(context);
    VM::setContext(saveContext);
    delete context; // stdlib�̏������ł�Context��������Q�Ƃ���鎖�͂Ȃ�
}


// ���C�u�����ރo�C�g�R�[�h�ǂݍ��݁A������
// �o�C�g�R�[�h�͓����ŊǗ������̂ŁA�Ō��finalize�������ōs�Ȃ���
SymbolID_t
Engine::readLibrary(const char* filename)
{
    Bytecode* pBytecode = gCodeManager.readBytecode(filename);
    Context* saveContext = VM::getContext();
    Context* context = gCodeManager.createContext();
    pBytecode->initLinkBytecode(context, true); // ������
    VM::setContext(saveContext);
    gCodeManager.releaseContext(context);
    return pBytecode->bytecodeSymbol();
}


// �o�C�g�R�[�h�̐擪����A�X���b�h�ŊJ�n
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


// �X���b�h�����I��
void
Engine::terminateThread(ThreadID_t tid)
{
    Thread* pThread = gThreadManager.id2thread(tid);
    if (pThread) pThread->terminate();
}


// ��n��
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
    // �o�C�g�R�[�h�̃f�o�b�O���(*.hdb)�ǂݍ��ݐݒ���f�t�H���g��true��
    Hayat::Engine::Bytecode::setFlagReadDebugInfo(true);
    MMes::initialize();
}

void
Engine::finalizeDebug(void)
{
    MMes::finalize();
    gSymbolTable.finalize();
}
