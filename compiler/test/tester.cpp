/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <stdarg.h>
#include "hyCCompiler.h"
#include "hyCStrTable.h"
#include "hyCFfiType.h"

// hmd_loadFile_inPathで使うロードパス定義
const char* m_HMD_LOADPATH[] = {
#ifndef WIN32
    "out",
    "../../stdlib/out",
    NULL
#else
    "out",
    "..\\..\\stdlib\\out",
    NULL
#endif
};
const char** HMD_LOADPATH = m_HMD_LOADPATH;


const char* gFfiDir;
const char* gFfiOutDir;

Hayat::Compiler::SymbolTable Hayat::Compiler::gLocalVarSymbols;

namespace Houken {
    class SyntaxTree;
}
namespace Hayat {
    namespace Compiler {
        SymbolTable     gSymTable;
        SymbolID_t      gFinallyValVar_SymTop;
        SymbolID_t      HyCSymS_cppSize;
        SymbolID_t      HyCSym_Object;
        SymbolID_t      HyCSym_nil;
        FfiTypeMgr gFfiTypeMgr;
        int  jumpLabelCheckLevel = 0;
        bool isJumpControlLabel(SymbolID_t) { return true; }
    }
}


void compileError(const char* msg, ...) {
    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    fflush(stderr);
    throw "compileError";
}
void compileError_pos(Houken::SyntaxTree*, const char* msg, ...) {
    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    fflush(stderr);
    throw "compileError";
}

void init_gSymTable(void)
{
    static const char* RESERVED_SYMS[] = {      // 予約シンボルテーブル
        "nil",              // "nil"シンボルを0番にする
        "NilClass",
        "*REF",             // 参照
        "*MAIN",            // メインクラス、メインルーチン
        "*INDIRECT_REF",    // 間接ローカル変数参照
        "*INDIRECT_ENT",    // 間接ローカル変数実体
        "*cppSize",        // c++オブジェクトのサイズを求める関数
        "Object",
        NULL
    };
    Hayat::Compiler::gSymTable.initialize(RESERVED_SYMS);
    Hayat::Compiler::HyCSymS_cppSize = Hayat::Compiler::gSymTable.symbolID("*cppSize");
    Hayat::Compiler::HyCSym_Object = Hayat::Compiler::gSymTable.symbolID("Object");
    Hayat::Compiler::HyCSym_nil = Hayat::Compiler::gSymTable.symbolID("nil");
}


int main(int argc, char* argv[])
{
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    CppUnit::Outputter* outputter = 
        new CppUnit::CompilerOutputter(&runner.result(),std::cout);
    runner.setOutputter(outputter);
    int result =  runner.run() ? 0 : 1;
    
    return result;
}
