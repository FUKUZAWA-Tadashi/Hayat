/*  -*- coding: sjis-dos; -*-  */


#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include "hayat.h"


using namespace Hayat::Common;
using namespace Hayat::Engine;


// hmd_loadFile_inPathで使うロードパス定義
const char* m_HMD_LOADPATH[] = {
#ifndef WIN32
    "../../engine/test/out",
    "../../stdlib/out",
    NULL
#else
    "..\\..\\engine\\test\\out",
    "..\\..\\stdlib\\out",
    NULL
#endif
};
const char** HMD_LOADPATH = m_HMD_LOADPATH;


int main(int argc, char* argv[])
{
#ifdef HMD_DEBUG
    void* debugMem = HMD_ALLOC(40960);
    MemPool* debugMemPool = MemPool::manage(debugMem, 40960); 
    gMemPool = NULL;
    Debug::setDebugMemPool(debugMemPool);
    gSymbolTable.readFile("symbols.sym");
    Bytecode::setFlagReadDebugInfo(true); // デバッグ情報ファイル *.hyb を読む
#endif

    CppUnit::TextUi::TestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    CppUnit::Outputter* outputter = 
        new CppUnit::CompilerOutputter(&runner.result(),std::cout);
    runner.setOutputter(outputter);
    int result =  runner.run() ? 0 : 1;
    
#ifdef HMD_DEBUG
    gSymbolTable.finalize();
    Debug::setDebugMemPool(NULL);
    HMD_FREE(debugMem);
#endif

    return result;
}
