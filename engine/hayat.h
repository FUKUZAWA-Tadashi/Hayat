/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HAYAT_H_
#define m_HAYAT_H_

#include "hyThreadManager.h"
#include "hyVM.h"
#include "hyBytecode.h"
#include "hyCodeManager.h"
#include "hyVarTable.h"
#include "hyStringBuffer.h"
#include "hyValueArray.h"
#include "hyException.h"
#include "hyHash.h"
#include "hyStringBuffer.h"
#include "hySymbolTable.h"
#include "hyDebug.h"

namespace Hayat {
    namespace Engine {

        // メモリ、GC等初期化
        void    initMemory(void* pMemory, size_t memorySize);

        // stdlib初期化
        // バイトコードは内部で管理されるので、最後のfinalizeも内部で行なわれる
        void    initStdlib(const char* stdlibPath = "stdlib.hyb");

        // ライブラリ類バイトコード読み込み、初期化
        // 戻り値はバイトコードシンボル
        // バイトコードは内部で管理されるので、最後のfinalizeも内部で行なわれる
        SymbolID_t      readLibrary(const char* filename);

        // バイトコードの先頭から、スレッドで開始
        ThreadID_t      startThread(Bytecode* bytecode);

        // スレッド強制終了
        void    terminateThread(ThreadID_t tid);

        // 後始末
        void    finalizeAll(void);

        // デバッグメモリ初期化
        void    initializeDebug(void* debugMemory = NULL, size_t debugMemorySize = 0);
        // デバッグ後始末
        void    finalizeDebug(void);
    }
}


#endif /* m_HAYAT_H_ */


/********** 使用例 *******
   
    // メモリ確保
    void* pMem = OS_ALLOCATE_MEMORY(memSize); 
   
    // 初期化
    Hayat::Engine::initMemory(pMem, memSize);
   
    // ===={ デバッグ用設定  (この部分は呼ばなくても良い)
    // デバッグ情報用メモリ確保 (しなければメインメモリgMemPoolに読む)
    void* pDebMem = OS_ALLOCATE_MEMORY(debMemSize);
    initializeDebug(pDebMem, debMemSize);
    // ↓バイトコードのデバッグ情報(*.hdb)読み込みをオフにする場合は以下を使う
    // Hayat::Engine::Bytecode::setFlagReadDebugInfo(false);

    // デバッグ用シンボル(symbols.sym)読み込み (無くても良い)
    Hayat::Engine::gSymbolTable.readFile("symbols.sym");
    // エラーメッセージのShift-JISバージョンを読み込み (無くても良い)
    MMes::readTable("mm_sjis.mm");
    // ====} デバッグ用設定終わり

   
    // stdlib初期化
    Hayat::Engine::initStdlib();
   
    // それぞれのプロジェクトで必要なバイトコード読み込み
    // 大元を読めばrequire先は芋蔓式に自動で読み込む
    Hayat::Engine::readLibrary("project_library_bytecode.hyb");
   
    // 実行したいバイトコードを読み込む
    Hayat::Engine::Bytecode* pBytecode = Hayat::Engine::gCodeManager.readBytecode("target.hyb");
    // それをスレッドで実行開始
    Hayat::Engine::ThreadID_t tid;
    tid = Hayat::Engine::startThread(pBytecode);
   
    // 1フレーム毎にexec1tick()を呼ぶ
    // while (Hayat::Engine::gThreadManager.isThreadRunning())
    //     Hayat::Engine::gThreadManager.exec1tick();
   
    // 後始末
    Hayat::Engine::finalizeAll();
 
    // デバッグ後始末  (デバッグ用設定をしたならば呼ぶ)
    Hayat::Engine::finalizeDebug();

    // デバッグ用メモリ解放 (確保したならば)
    OS_FREE_MEMORY(pDebMem);


    // メモリ解放
    OS_FREE_MEMORY(pMem);

********** **********/
