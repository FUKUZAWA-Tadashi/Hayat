/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


#include "grammar.tab.h"
#include "hyCCompileFile.h"
#include "hyMemPool.h"
#include "hyCStrTable.h"
#include "hyCContext.h"
#include "hyCCompiler.h"
#include "mbcsconv.h"
#include "hpPrintSyntaxTree.h"


namespace Hayat {
    namespace Compiler {
        // グローバルなシンボルテーブル
        // engineで同じシンボルを使用する
        extern SymbolTable     gSymTable;
        // ローカル変数名のためのシンボルテーブル
        // engineでは使用されない
        SymbolTable     gLocalVarSymbols;
        // finallyで一時使用するローカル変数領域に付ける最初の名前
        SymbolID_t      gFinallyValVar_SymTop;
        // 文字列テーブル
        StrTable        gStrTable;
        // FFIインターフェースマネージャ
        FfiTypeMgr      gFfiTypeMgr;
    }
}
using namespace Hayat::Common;
using namespace Hayat::Compiler;



Hayat::Compiler::CompileOption Hayat::Compiler::compileOption;



extern const char* gFfiOutDir;
extern const char* gFfiDir;
extern const char* gOutDir;
extern const char* gInpCharSet;
extern const char* gOutCharSet;
extern const char** HMD_LOADPATH;
extern hyu32 numLoadPath;


// パスからディレクトリ部分と拡張子部分を除いたファイル名部分を返す。
// 不要になったら gMemPool->free() する必要がある。
char* baseName(const char* path)
{
    const char* p = strrchr(path, PATH_DELIM);
    if (p == NULL)
        p = path;
    else
        ++p;
    const char* q = strrchr(path, '.');
    if (q == NULL)
        q = path + HMD_STRLEN(path);
    if (p >= q)
        return NULL;
    char* base = gMemPool->allocT<char>(q - p + 1);
    char* b = base;
    while (p < q) {
        *b++ = *p++;
    }
    *b = '\0';
    return base;
}

void printParseError(void)
{
    HMD_PRINTF("parse error\n");
    int n = Parser::numErrors();
    for (--n; n >= 0; --n) {
        Parser::Error_t err = Parser::getError(n);
        char pbuf[128];
        gpInp->sprintSourceInfo(pbuf, 128, err.pos);
        const char* nam = err.parser->name();
        if ((nam != NULL) && (*nam != '('))
            HMD_PRINTF("%s: parser=%s\n", pbuf, nam);

    }
}


int Hayat::Compiler::compileFile(const char* hyFileName)
{
    if (! hmd_isFileExist(hyFileName)) {
        HMD_PRINTF("file %s not found\n", hyFileName);
        return -1;
    }

    int result = -1;

    char* hyBaseName = baseName(hyFileName);
    size_t hyBaseNameLen = HMD_STRLEN(hyBaseName);

    hyu32 ffiOutDirLen = HMD_STRLEN(gFfiOutDir);
    hyu32 stfnBufLen = ffiOutDirLen + hyBaseNameLen + 20;
    char* ffiTblFileName = gMemPool->allocT<char>(stfnBufLen);
    HMD_STRNCPY(ffiTblFileName, gFfiOutDir, ffiOutDirLen + 1);
    if (gFfiOutDir[ffiOutDirLen-1] != PATH_DELIM)
        HMD_STRSCAT(ffiTblFileName, PATH_DELIM_S, stfnBufLen);
    HMD_STRSCAT(ffiTblFileName, "HSft_", stfnBufLen);
    HMD_STRSCAT(ffiTblFileName, hyBaseName, stfnBufLen);
    HMD_STRSCAT(ffiTblFileName, "_tbl.cpp", stfnBufLen);



    bool b = MBCSConv::initialize(gInpCharSet, gOutCharSet);
    HMD_ASSERTMSG(b, M_M("charset specification failed: in=%s, out=%s"),gInpCharSet,gOutCharSet);
    gStrTable.initialize();
    gFfiTypeMgr.initialize();
    SyntaxTree::initializePool(compileOption.bUseMemoize);
    Package::initialize();
    Context::initializeAll();
    initializeCompiler();

    Hayat::Compiler::jumpLabelCheckLevel = compileOption.Wlabel + (compileOption.WlabelIsError ? 2 : 0);


    size_t dirLen = HMD_STRLEN(gOutDir);
    char* outFileName = gMemPool->allocT<char>(dirLen + hyBaseNameLen + 6);
    HMD_STRNCPY(outFileName, gOutDir, dirLen + 1);
    char* q = outFileName + dirLen;
    *q++ = PATH_DELIM;
    HMD_STRNCPY(q, hyBaseName, hyBaseNameLen + 1);
    char* outFileExt = q + hyBaseNameLen;
    HMD_STRNCPY(outFileExt, ".hyb", 5);
    SymbolID_t packageSym = gSymTable.symbolID(q);

    FileInputBuffer inp(hyFileName);
    gpInp = &inp;
    for (hyu32 i = 0; i < numLoadPath; ++i)
        inp.addIncludePath(HMD_LOADPATH[i]);
    Hayat::Parser::Parser::initialize(40);
    Hayat::Parser::Parser::setUseMemoize(compileOption.bUseMemoize);
    Hayat::Parser::Parser::setPrintIntermediateLevel(compileOption.printIntermediateLevel);


    SyntaxTree* st;
    try {
        st = p_main->parse();
    } catch (const char* m) {
        HMD_PRINTF(M_M("parse error: %s"), m);
        st = NULL;
    }

    if (st == NULL) {
        result = 2;
    } else if(st->isValidTree()) {
        if (compileOption.bPrintSyntaxTree)
            printSyntaxTree(st, &inp, 0);

        Package* myPackage = new Package(packageSym);
        Package::registration(myPackage);
        try {
            //HMD_DEBUG_ASSERT(myPackage->packageClass() == Contect::current());
            st->compile(Context::current());
            Context::current()->postCompileProcess(0, 0);

            if (! myPackage->finalCheck(compileOption.Wunknown)) {
                if (compileOption.Wunknown > 1)
                    throw "error check";
            }
            Context::jumpLabelCheck();

            if (compileOption.bVerbose)
                HMD_PRINTF("output: %s\n", outFileName);
            hmd_mkdir(gOutDir, 0775);
            FILE* fp = hmd_fopen(outFileName, "wb");
            if (fp == NULL) {
                HMD_PRINTF("cannot write %s\n", outFileName);
                result = 1;
            } else {
                // バイトコード出力
                myPackage->fwriteLinks(fp);
                gStrTable.writeFile(fp);
                myPackage->fwriteSignatureTable(fp);
                myPackage->fwriteArityTable(fp);
                long foffs = ftell(fp);
                Context::current()->fwriteByteCodes(fp);
                fclose(fp);

                // デバッグ情報ファイル出力
                HMD_STRNCPY(outFileExt, ".hdb", 5);
                fp = hmd_fopen(outFileName, "wb");
                if (fp == NULL) {
                    HMD_PRINTF("cannot write %s\n", outFileName);
                    result = 1;
                } else {
                    Context::current()->fwriteDebugInfo(fp, (hyu32)foffs);
                    fclose(fp);
                }

                // パッケージ情報ファイル出力
                HMD_STRNCPY(outFileExt, ".hyp", 5);
                fp = hmd_fopen(outFileName, "wb");
                if (fp == NULL) {
                    HMD_PRINTF("cannot write %s\n", outFileName);
                    result = 1;
                } else {
                    myPackage->fwritePackage(fp);
                    fclose(fp);
                }

                if (compileOption.bVerbose)
                    HMD_PRINTF("done.\n");
                result = 0;
            }

            // FFI関数テーブル出力  HSft_*m_tbl.cpp
            Context::current()->writeFfiTbl(ffiTblFileName);

            if (compileOption.Wnumsay != 0 && Context::sayCommandIndex() >= compileOption.Wnumsay) {
                // sayCommand数オーバー
                const char* en;
                if (compileOption.WnumsayIsError) {
                    result = 1;
                    en = "error";
                } else {
                    en = "warning";
                }
                HMD_PRINTF("%s : %s: number of sayCommand %d >= limit %d\n", hyFileName, en, Context::sayCommandIndex(), compileOption.Wnumsay);
            }

        } catch (CException ex) {
            HMD_PRINTF("%s\n", ex.message);
            result = 1;
        } catch (const char* e) {
            HMD_PRINTF("compile error: %s\n", e);
            result = 1;
        }

        if (! compileOption.bUseMemoize) {
            SyntaxTree::deleteRecursively(st);
        }

    } else {
        printParseError();
        result = 1;
    }

    if (result != 0) {
        HMD_STRNCPY(outFileExt, ".hyb", 5);
        hmd_rm_file(outFileName);
        HMD_STRNCPY(outFileExt, ".hdb", 5);
        hmd_rm_file(outFileName);
        HMD_STRNCPY(outFileExt, ".hyp", 5);
        hmd_rm_file(outFileName);
    }

    gMemPool->free(outFileName);
    gMemPool->free(hyBaseName);
    gMemPool->free(ffiTblFileName);

    finalizeCompiler();
    Hayat::Parser::Parser::finalize();
    Package::finalize();
    SyntaxTree::finalizePool();
    gStrTable.finalize();
    MBCSConv::finalize();
    gFfiTypeMgr.finalize();
    Context::finalizeAll();

    return result;
}
