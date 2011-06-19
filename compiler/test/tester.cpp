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

// hmd_loadFile_inPath�Ŏg�����[�h�p�X��`
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
    static const char* RESERVED_SYMS[] = {      // �\��V���{���e�[�u��
        "nil",              // "nil"�V���{����0�Ԃɂ���
        "NilClass",
        "*REF",             // �Q��
        "*MAIN",            // ���C���N���X�A���C�����[�`��
        "*INDIRECT_REF",    // �Ԑڃ��[�J���ϐ��Q��
        "*INDIRECT_ENT",    // �Ԑڃ��[�J���ϐ�����
        "*cppSize",        // c++�I�u�W�F�N�g�̃T�C�Y�����߂�֐�
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
