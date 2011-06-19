/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "grammar.tab.h"
#include "hyCCompileFile.h"
#include "hpPrintSyntaxTree.h"
#include "hyMemPool.h"
#include "hyCSymbolTable.h"
#include "hyCCompileError.h"
#include "hyCFileOut.h"
#include "hyCCommandLine.h"
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif


#define THIS_NAME       "hayatc"
#define VERSION         "version 0.1"
#if defined(__CYGWIN__) || defined(WIN32)
#define EXE_FILENAME    "hayatc.exe"
#else
#define EXE_FILENAME    "hayatc"
#endif


namespace Hayat {
    namespace Compiler {
        // グローバルなシンボルテーブル
        // engineで同じシンボルを使用する
        SymbolTable     gSymTable;
    }
}
using namespace Hayat::Common;
using namespace Hayat::Compiler;


const char* gFfiOutDir;
const char* gFfiDir;
const char* gOutDir;
const char* gInpCharSet = "UTF-8";
const char* gOutCharSet = "UTF-8";


void* hayatMemory;


// ロードパス
const char** HMD_LOADPATH = NULL;
hyu32 numLoadPath = 0;





void SyntaxTree::compile(Context*)
{
#if 1
    // for debugging
    fprintf(stderr, "undefined action of parser %s : childs=[%d:",parser->name(),numChild());
    for (int i = 0; i < numChild(); i++) {
        fprintf(stderr, "%s,", get(i)->parser->name());
    }
    fprintf(stderr, "]\nat '");
    gpInp->fprintSummary(stderr, str);
    fprintf(stderr, "'\n");
    fflush(stderr);
    exit(1);
#endif
}

void errorCutMessage(hyu32 pos, const char* message)
{
    outError(pos, message);
}


void usage(void)
{
    printf("usage: " EXE_FILENAME " [options] hyfile\n");
    printf("option:\n");
    printf("  -I path : add include path\n");
    printf("  -C path : change directory to path before compile\n");
    printf("  -o path : set output path\n");
    printf("  -zi path : set ffi path\n");
    printf("  -zo path : set ffiout path\n");
    printf("  -zw : always overwrite ffiout files\n");
    printf("  -ci charset : set input file character set\n");
    printf("  -co charset : set output file character set\n");
    printf("  -m : not use memoization\n");
    printf("  -s : print syntax tree\n");
    printf("  -d : print intermediate parsing result\n");
    printf("  -d2 : print detail intermediate parsing result\n");
    printf("  -Wlabel=N : set goto label check level (default 2!)\n");
    printf("  -Wnumsay=N : set limit of sayCommand (default 0)\n");
    printf("  -Wunknown=N : unknown class or method check level to N (default 1)\n");
    printf("  -Wconstscope=N : constant variable override check : 0 = OK, 1 = warning, 1! = error\n");
    printf("  -M size : memory allocate size for compiler (defalut 4M)\n");
    printf("  -T path : read tlanslated message file\n");
    printf("  -V : print version info\n");
}



int main(int argc, const char* argv[])
{
    int result = 99;
    const char** hyFileName = NULL;
    int capacityHyFileName = 0;
    int numHyFileName = 0;
    const char* incPath[32];
    int numIncPath = 0;
    int wl;

    compileOption.printIntermediateLevel = 0;
    compileOption.bPrintSyntaxTree = false;
    compileOption.bUseMemoize = true;
    compileOption.bVerbose = false;
    compileOption.Wlabel = 2;
    compileOption.WlabelIsError = true;
    compileOption.Wunknown = 1;
    compileOption.Wnumsay = 0;
    compileOption.WnumsayIsError = false;
    compileOption.Wconstscope = 1;
    compileOption.mmesFileName = NULL;


    hyu32 memSize = 4 * 1024 * 1024;

    gFfiDir = "ffi";
    gFfiOutDir = "ffiout";
    gOutDir = "out";

    MMes::initialize();

    for (int i = 1; i < argc; i++) {
        const char* param;
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
            case 's':
                compileOption.bPrintSyntaxTree = true;
                break;
            case 'd':
                // デバッグ表示レベル
                if (argv[i][2] >= '0' && argv[i][2] <= '9')
                    compileOption.printIntermediateLevel = argv[i][2] - '0';
                else
                    compileOption.printIntermediateLevel = 1;
                break;
            case 'm':
                // memoizeしない
                compileOption.bUseMemoize = false;
                break;
            case 'I':
                // includeパス
                if (numIncPath >= 32) {
                    printf("too many include path (must <=32)\n");
                    HMD_FREE(hyFileName);
                    exit(1);
                }
                param = getParam(i, argc, argv, "-I");
                if (param != NULL)
                    incPath[numIncPath++] = param;
                break;
            case 'C':
                // chdir
                param = getParam(i, argc, argv, "-C");
                if (param != NULL)
                    hmd_chdir(param);
                break;
            case 'o':
                // 出力ディレクトリ
                param = getParam(i, argc, argv, "-o");
                if (param != NULL)
                    gOutDir = param;
                break;
            case 'z':
                switch (argv[i][2]) {
                case 'i':
                    param = getParam(i, argc, argv, "-zi");
                    if (param != NULL)
                        gFfiDir = param;
                    break;
                case 'o':
                    param = getParam(i, argc, argv, "-zo");
                    if (param != NULL)
                        gFfiOutDir = param;
                    break;
                case 'w':
                    DiffWrite::setDefaultMode(DiffWrite::DW_OVERWRITE);
                    break;
                default:
                    printf("unknown option: %s\n", argv[i]);
                    usage();
                    HMD_FREE(hyFileName);
                    exit(1);
                }
                break;
            case 'c':
                switch (argv[i][2]) {
                case 'i':
                    param = getParam(i, argc, argv, "-ci");
                    if (param != NULL)
                        gInpCharSet = param;
                    break;
                case 'o':
                    param = getParam(i, argc, argv, "-co");
                    if (param != NULL)
                        gOutCharSet = param;
                    break;
                default:
                    printf("unknown option: %s\n", argv[i]);
                    usage();
                    HMD_FREE(hyFileName);
                    exit(1);
                }
                break;
            case 'M':
                param = getParam(i, argc, argv, "-M");
                if (param != NULL) {
                    memSize = 0;
                    while('0' <= *param && *param <= '9') {
                        memSize *= 10;
                        memSize += (*param++ - '0');
                    }
                    if (*param == 'k' || *param == 'K') {
                        memSize *= 1024;
                    } else if (*param == 'm' || *param == 'M') {
                        memSize *= 1024 * 1024;
                    } else if (*param != '\0') {
                        printf("bad parameter for -M\n");
                        memSize = 4 * 1024 * 1024;
                    }
                }
                break;
            case 'v':
                compileOption.bVerbose = true;
                break;
            case 'V':
                printf("Hayat compiler " THIS_NAME " " VERSION "\n");
                HMD_FREE(hyFileName);
                exit(0);
                break;

            case 'T':
                compileOption.mmesFileName = getParam(i, argc, argv, "-T");
                break;

            case 'W':
                if (0 <= (wl = getWarnLevel("label", &argv[i][2]))) {
                    compileOption.Wlabel = wl / 2;
                    compileOption.WlabelIsError = ((wl & 1) != 0);
                    if (compileOption.Wlabel > 2) {
                        printf("-Wlabel option allows from 0 to 2\n");
                        usage();
                        HMD_FREE(hyFileName);
                        exit(1);
                    }
                } else if (0 <= (wl = getWarnLevel("numsay", &argv[i][2]))) {
                    compileOption.Wnumsay = wl / 2;
                    compileOption.WnumsayIsError = ((wl & 1) != 0);
                } else if (0 <= (wl = getWarnLevel("unknown", &argv[i][2]))) {
                    if (wl == 1 || wl > 3) {
                        printf("-Wunknown option allows from 0 or 1 or 1!\n");
                        usage();
                        HMD_FREE(hyFileName);
                        exit(1);
                    }
                    if (wl > 1) --wl;
                    compileOption.Wunknown = wl;
                } else if (0 <= (wl = getWarnLevel("constscope", &argv[i][2]))) {
                    if (wl == 1 || wl > 3) {
                        printf("-Wconstscope option allows from 0 or 1 or 1!\n");
                        usage();
                        HMD_FREE(hyFileName);
                        exit(1);
                    }
                    if (wl > 1) --wl;
                    compileOption.Wconstscope = wl;
                } else {
                    printf("unknown -W option: %s\n", &argv[i][2]);
                    usage();
                    HMD_FREE(hyFileName);
                    exit(1);
                }
                break;

            default:
                printf("unknown option: %s\n", argv[i]);
                usage();
                HMD_FREE(hyFileName);
                exit(1);
            }
        } else {
            if (capacityHyFileName == 0) {
                hyFileName = (const char**)HMD_ALLOC(sizeof(const char**) * 16);
                capacityHyFileName = 16;
            } else if (numHyFileName >= capacityHyFileName) {
                const char** p = (const char**)HMD_ALLOC(sizeof(const char*) * 2 * capacityHyFileName);
                memcpy(p, hyFileName, sizeof(const char*) * capacityHyFileName);
                HMD_FREE(hyFileName);
                hyFileName = p;
                capacityHyFileName *= 2;
            }
            hyFileName[numHyFileName++] = argv[i];
        }
    }

    if (numHyFileName == 0) {
        printf("error: input file required\n");
        exit(1);
    }




    hmd_mkdir(gOutDir, 0775);
    size_t dirLen = HMD_STRLEN(gOutDir);

    char* lockFileName; // 並列コンパイル禁止ロックファイル名
    char* symFileName;  // シンボルファイル名


    // 並列コンパイル禁止ロック
    lockFileName = (char*)HMD_ALLOCA(dirLen + 20);
    HMD_STRNCPY(lockFileName, gOutDir, dirLen+1);
    HMD_STRSCAT(lockFileName, PATH_DELIM_S, dirLen + 20);
    HMD_STRSCAT(lockFileName, "_hayatc_lock_", dirLen + 20);
    int timeout = 10;
    while (! hmd_lockFile(lockFileName)) {
        if (--timeout < 1)
            compileError(M_M("cannot obtain compile lock (lock file: %s)"), lockFileName);
        hmd_sleep(1);
    }


    hayatMemory = HMD_ALLOC(memSize);
    MemPool::initGMemPool(hayatMemory, memSize);


    try {

        for (int i = 0; i < numIncPath; ++i) {
            addLoadPath(incPath[i]);
        }
        addLoadPath(".", gOutDir);

        // 翻訳メッセージファイル読み込み
        MMes::readTable(compileOption.mmesFileName);

        // シンボル読み込み
        static const char* RESERVED_SYMS[] = {      // 予約シンボルテーブル
            "nil",              // "nil"シンボルを0番にする
            "NilClass",
            "*REF",             // 参照
            "*MAIN",            // メインルーチン
            "*INDIRECT_REF",    // 間接ローカル変数参照
            "*INDIRECT_ENT",    // 間接ローカル変数実体
            "*RELOCATED_OBJ",   // Object移動情報
            "*cppSize",        // c++オブジェクトのサイズを求める関数
            NULL
        };
        gSymTable.initialize(RESERVED_SYMS);
        size_t symFNameLen = HMD_STRLEN(SymbolTable::SYMBOL_FILENAME);
        symFileName = gMemPool->allocT<char>(dirLen + symFNameLen + 1, "fnsy");
        HMD_STRNCPY(symFileName, gOutDir, dirLen+1);
        HMD_STRSCAT(symFileName, SymbolTable::SYMBOL_FILENAME, dirLen + symFNameLen + 1);

        for (hyu32 i = 0; i < numLoadPath; ++i) {
            size_t dl = HMD_STRLEN(HMD_LOADPATH[i]);
            char* fn = (char*)HMD_ALLOCA(dl + symFNameLen + 1);
            HMD_STRNCPY(fn, HMD_LOADPATH[i], dl+1);
            HMD_STRSCAT(fn, SymbolTable::SYMBOL_FILENAME, dl + symFNameLen + 1);
            if (! gSymTable.mergeFile(fn)) {
                gSymTable.finalize();
                compileError(M_M("symbol table %s have inconsistency\nyou need recompile dependent scripts"), fn);
            }
        }

    } catch (CException e) {
        hmd_unlockFile();
        HMD_FREE(hayatMemory);
        HMD_PRINTF(M_M("program exited by compile error"));
        exit(1);
    }

    hyu32 ffiOutDirLen = HMD_STRLEN(gFfiOutDir);
    hyu32 fnBufLen = ffiOutDirLen + HMD_STRLEN(SymbolTable::SYMBOL_H_NAME) + 1;
    char* HSymbolFileName = gMemPool->allocT<char>(fnBufLen, "fnsh");
    HMD_STRNCPY(HSymbolFileName, gFfiOutDir, ffiOutDirLen + 1);
    HMD_STRSCAT(HSymbolFileName, SymbolTable::SYMBOL_H_NAME, fnBufLen);

#if 0
HMD_PRINTF("------ pre compile\n");
gMemPool->printCellInfo();
HMD_PRINTF("------\n");
#endif
    for (int i = 0; i < numHyFileName; ++i) {
        try {
            result = compileFile(hyFileName[i]);
        } catch (const char* m) {
            HMD_PRINTF("compile error: %s\n", m);
            result = 1;
        } catch (CException e) {
            result = 1;
        }
        if (result != 0)
            break;
        gMemPool->coalesce();
#if 0
HMD_PRINTF("------ post compile %s\n", hyFileName[i]);
gMemPool->printCellInfo();
HMD_PRINTF("------\n");
#endif
    }

    gSymTable.writeFile(symFileName);
    gSymTable.writeSymbolH(HSymbolFileName, compileOption.bVerbose);
    gSymTable.finalize();
    gMemPool->free(symFileName);
    gMemPool->free(HSymbolFileName);

    MMes::finalize();
    finalizeLoadPath();

    hmd_unlockFile();

    HMD_FREE(hayatMemory);

    return result;
}
