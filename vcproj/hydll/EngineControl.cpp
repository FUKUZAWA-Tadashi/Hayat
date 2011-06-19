/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "stdafx.h"
#include "EngineControl.h"
#include "hayat.h"


// DLLロード時にメッセージ出力
#define VERBOSE_LOAD_DLL

#ifdef MS_DLL_EXPORT_DECL
# define DLL_EXPORT     _declspec(dllexport)
#else
# define DLL_EXPORT
#endif


using namespace Hayat::Common;
using namespace Hayat::Engine;

// ロードパスのアドレス
DLL_EXPORT const char** HMD_LOADPATH = NULL;


namespace hydll {

    // 実行フレームカウント
    hyu32   g_nFrame = 0;

    // メインメモリ
    size_t  mainMemSize = 4 * 1024 * 1024;
    void*   mainMem = NULL;

    // デバッグメモリ
    size_t  debugMemSize = 0;
    void*   debugMem = NULL;
    // デバッグ情報を読むかどうか
    bool    bReadDebugInfos = false;

    // ロードパスを格納しておく配列
    TArray<const char*> loadPathArr(0);

    // アップデート関数を登録しておく配列
    TArray<RFunc_t>     updateFuncArr(0);

    // 描画関数を登録しておく配列
    TArray<RFunc_t>     drawFuncArr(0);

    // 終了時処理関数を登録しておく配列
    TArray<RFunc_t>     termFuncArr(0);

    // DLLのハンドラを登録しておく配列
    TArray<HMODULE>     dllHandleArr(0);



    void    printLastError(void)
    {
        LPVOID lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      GetLastError(),
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 既定の言語
                      (LPTSTR) &lpMsgBuf,
                      0,
                      NULL
                      );
        HMD_PRINTF((const char*)lpMsgBuf);
        LocalFree(lpMsgBuf);
    }


    // Hayatで使用するメモリサイズを指定する
    // 指定しなければ、4MiBのメモリを確保する
    void    setMemSize(size_t memSize)
    {
        mainMemSize = memSize;
    }

    // Hayatで使用するデバッグ用メモリサイズを指定する
    // 指定しなければ、デバッグ用メモリは確保しない
    void    setDebugMemSize(size_t memSize)
    {
        debugMemSize = memSize;
    }

    void    setFlagReadDebugInfos(bool flag)
    {
        bReadDebugInfos = flag;
    }

    // ロードパスを追加する
    void    addLoadPath(const char* path)
    {
        //printf("addLoadPath(%s)\n", path);
        hyu32 n = loadPathArr.size();
        if (n == 0) {
            loadPathArr.add(NULL);
            n = 1;
        }
        hyu32 len = HMD_STRLEN(path);
        char* apath = gMemPool->allocT<char>(len+1);
        HMD_STRNCPY(apath, path, len+1);
        loadPathArr[n-1] = apath;  // end mark に上書き
        loadPathArr.add(NULL);     // 新しい end mark
        HMD_LOADPATH = loadPathArr.top();
        // n=0;for(const char** p = HMD_LOADPATH; *p != NULL; ++p)HMD_PRINTF("path %d:'%s'\n",n++,*p);
    }

    // 1フレームに1回呼ばれるアップデート関数を登録する
    void    addUpdateFunc(RFunc_t func)
    {
        updateFuncArr.add(func);
    }
    // アップデート関数を先頭に登録する
    void    insertUpdateFunc(RFunc_t func)
    {
        updateFuncArr.insert(0, 1) = func;
    }

    // 1フレームに1回呼ばれる描画関数を登録する
    void    addDrawFunc(RFunc_t func)
    {
        drawFuncArr.add(func);
    }

    // 終了時処理関数を登録する
    void    addTermFunc(RFunc_t func)
    {
        termFuncArr.add(func);
    }



    static HMODULE LoadLibraryMB(const char* path)
    {
#ifdef _UNICODE
        int len = _mbslen((const unsigned char*)path);
        int siz = MultiByteToWideChar(0,0, path,len, NULL,0);
        TCHAR* apath = (TCHAR*)HMD_ALLOCA((siz+1) * sizeof(TCHAR));
        MultiByteToWideChar(0,0, path,len, apath,siz);
        return AfxLoadLibrary(apath);
#else
        return AfxLoadLibrary(path);
#endif
    }

    // バイトコードがメモリにロードされた直後のコールバック。
    // 対応するDLLがあればロードする
    static void cb4dll(const char* hybPath)
    {
        int len = HMD_STRLEN(hybPath);
        char* dllPath = (char*)HMD_ALLOCA(len + sizeof(dll_prefix)+sizeof(dll_ext));
        char* q = dllPath;
        char* q2 = q;

        const unsigned char* p = _mbsrchr((const unsigned char*)hybPath, PATH_DELIM);
        if (p == NULL) {
            p = (const unsigned char*)hybPath;
        } else {
            p += 1;
            size_t lp = p - (const unsigned char*)hybPath;
            memcpy(q, hybPath, lp);
            q += lp;
            q2 = q;
        }
        // pはhybPathのファイル名部分の先頭アドレス
        const unsigned char* p2 = _mbsstr((const unsigned char*)p, (const unsigned char*)".hyb");
        if ((p2 <= p) || (p2 >= (const unsigned char*)hybPath + len)) {
            p2 = (const unsigned char*)hybPath + len;
        }
        // p2は拡張子を抜いたファイル名部分の末尾アドレス

        memcpy(q, dll_prefix, sizeof(dll_prefix));
        q += sizeof(dll_prefix) - 1;
        size_t lp2 = p2 - p;
        memcpy(q, p, lp2);
        q += lp2;
        memcpy(q, dll_ext, sizeof(dll_ext));
        q += sizeof(dll_ext) - 1;
        *q = '\0';

#ifdef VERBOSE_LOAD_DLL
        HMD_PRINTF("try load dll: %s\n", dllPath);
#endif
		HMODULE hmod = LoadLibraryMB(dllPath);    // dllロード
#ifdef VERBOSE_LOAD_DLL
        if (hmod == NULL) printLastError();
#endif        
        if (hmod == NULL && q2 != dllPath) {
            // パス抜きのファイル名だけでロードしてみる
#ifdef VERBOSE_LOAD_DLL
            HMD_PRINTF("try load dll: %s\n", q2);
#endif        
            hmod = LoadLibraryMB(q2);
#ifdef VERBOSE_LOAD_DLL
            if (hmod == NULL) printLastError();
#endif        
        }

        if (hmod != NULL) {
            dllHandleArr.add(hmod);
#ifdef VERBOSE_LOAD_DLL
            HMD_PRINTF("DLL '%s' loaded.\n", q2);
#endif
        }
    }



    void init_memory(void)
    {
        // デバッグ出力は、OutputDebugString を使う
        //hmd_set_debug_out_mode(HMD_DEBUG_OUT_ODS);
        // デバッグ出力は、標準出力を使う
        hmd_set_debug_out_mode(HMD_DEBUG_OUT_STD);

        // メインメモリ初期化
        mainMem = HMD_ALLOC(mainMemSize);
        HMD_ASSERT(mainMem != NULL);
        initMemory(mainMem, mainMemSize);

        // デバッグメモリ初期化
        if (bReadDebugInfos) {
            if (debugMemSize == 0)
                debugMemSize = 4 * 1024 * 1024;
        }
        if (debugMemSize > 0) {
            debugMem = HMD_ALLOC(debugMemSize);
            if (debugMem == NULL)
                debugMemSize = 0;
            else
                initializeDebug(debugMem, debugMemSize);
        }
    }


    void init_main(void)
    {
        jmp_buf hayatJmpBuf;
        if (setjmp(hayatJmpBuf) == 0) {
            hmd_halt_jmpbuf = & hayatJmpBuf;

            if (bReadDebugInfos) {
                // デバッグ情報読み込み
                gSymbolTable.readFile("symbols.sym");
                Bytecode::setFlagReadDebugInfo(true);
                MMes::initialize();
                MMes::readTable("mm_sjis.mm");
            }

            // DLL読み込み用コールバック登録
            gCodeManager.setBytecodeLoadedCallback(cb4dll);

            // 標準ライブラリロード&初期化
            initStdlib();
        } else {
#ifdef HMD_ALLOW_CPP_EXCEPTION
            throw "initialization failed";
#else
            HMD_FATAL_ERROR(M_M("initialization failed"));
#endif
        }
    }

    
    Bytecode* load_bytecode(const char* hybFilename)
    {
        Bytecode* pBytecode = gCodeManager.readBytecode(hybFilename);
        if (pBytecode == NULL) {
            HMD_FATAL_ERROR(M_M("cannot load %s"), hybFilename);
        }
		return pBytecode;
	}

    bool reload_bytecode(const char* hybFilename, bool bInit)
    {
        Bytecode* pBytecode = gCodeManager.reloadBytecode(hybFilename, SYMBOL_ID_ERROR, bInit);
        if (pBytecode == NULL)
            return false;
        return true;
	}

    void start_bytecode(Bytecode* pBytecode)
    {
        jmp_buf hayatJmpBuf;
        if (setjmp(hayatJmpBuf) == 0) {
            hmd_halt_jmpbuf = & hayatJmpBuf;

            // 実行開始
            /*ThreadID_t tid =*/ startThread(pBytecode);

            // 最初の1tickを実行
            gThreadManager.exec1tick();

        } else {
#ifdef HMD_ALLOW_CPP_EXCEPTION
            throw "error on hayat script initialization";
#else
            HMD_FATAL_ERROR(M_M("error on hayat script initialization"));
#endif
        }
    }

    bool start_main(const char* hybFilename)
    {
        Bytecode* pBytecode = load_bytecode(hybFilename);
        if (pBytecode != NULL) {
            start_bytecode(pBytecode);
            return true;
        }
        return false;
    }

    bool tick_main(void)
    {
        if (! gThreadManager.isThreadRunning()) {
            return false;
        }

        jmp_buf hayatJmpBuf;
        if (setjmp(hayatJmpBuf) == 0) {

            hmd_halt_jmpbuf = & hayatJmpBuf;
        
            gThreadManager.exec1tick();

            hyu32 n = updateFuncArr.size();
            for (hyu32 i = 0; i < n; ++i)
                updateFuncArr[i]();

            n = drawFuncArr.size();
            for (hyu32 i = 0; i < n; ++i)
                drawFuncArr[i]();

            GC::incremental();

        } else {
#ifdef HMD_ALLOW_CPP_EXCEPTION
            throw "error on hayat script execution";
#else
            HMD_FATAL_ERROR(M_M("error on hayat script execution"));
#endif
        }

        return true;
    }

    void term_main(void)
    {
        hyu32 n = termFuncArr.size();
        for (hyu32 i = 0; i < n; ++i)
            termFuncArr[i]();

        finalizeAll();
        finalizeDebug();

        loadPathArr.finalize();
        updateFuncArr.finalize();
        drawFuncArr.finalize();
        termFuncArr.finalize();
        for (hyu32 i = dllHandleArr.size(); i > 0; ) {
            AfxFreeLibrary(dllHandleArr[--i]);
        }
        dllHandleArr.finalize();

        HMD_FREE(mainMem);
        HMD_FREE(debugMem);
    }

}
