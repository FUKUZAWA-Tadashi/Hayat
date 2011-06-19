/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#pragma once

#include "machdep.h"
#include "tchar.h"

namespace Hayat {
    namespace Engine {
        class Bytecode;
    }
}

namespace hydll {

    // 登録関数型
    typedef void (*RFunc_t)(void);

    // foo.hy用のDLLのファイル名は hyr_foo.dll であるとして検索する。
    // DLLファイル名の前に置く文字列
    static const char dll_prefix[] = "hyr_";
    // DLLファイル名の拡張子
    static const char dll_ext[] = ".dll";

    // 実行フレームカウント
    extern hyu32    g_nFrame;

    // Hayatで使用するメモリサイズを指定する
    // 指定しなければ、4MiBのメモリを確保する
    // init_memory()の前に呼ばなければならない
    extern void    setMemSize(size_t memSize);

    // デバッグ用情報をロードするかどうかを設定
    // falseだと、setDebugMemSize()を呼んでもデバッグ用メモリは確保しない。
    // trueだと、シンボル情報、デバッグ情報、SJISエラーメッセージ を読む。
    //     setDebugMemSize()を呼んでいない場合はメインのメモリに読む。
    // init_memory()の前に呼ばなければならない
    extern void    setFlagReadDebugInfos(bool flag);

    // Hayatで使用するデバッグ用メモリサイズを指定する
    // 指定しなければ、デバッグ用メモリは確保しない
    // init_memory()の前に呼ばなければならない
    extern void    setDebugMemSize(size_t MemSize);

    // メモリ初期化
    extern void    init_memory(void);

    // ロードパスを追加する
    extern void    addLoadPath(const char* path);

    // 基本初期化
    extern void    init_main(void);

    // バイトコードを読み込む
    Hayat::Engine::Bytecode*   load_bytecode(const char* hybFilename);

    // バイトコードをリロード
    extern bool    reload_bytecode(const char* hybFilename, bool bInit);

    // 指定バイトコードの頭から新規スレッドで実行
    void    start_bytecode(Hayat::Engine::Bytecode* pBytecode);

    // 指定バイトコードファイルを読み込んで頭から新規スレッドで実行
    extern bool    start_main(const char* hybFilename);


    // スクリプトを 1tick 実行
    extern bool    tick_main(void);

    // 終了処理
    extern void    term_main(void);




    // 1フレームに1回呼ばれるアップデート関数を登録する
    extern void    addUpdateFunc(RFunc_t func);
    // アップデート関数を先頭に登録する
    extern void    insertUpdateFunc(RFunc_t func);

    // 1フレームに1回呼ばれる描画関数を登録する
    extern void    addDrawFunc(RFunc_t func);

    // 終了時処理関数を登録する
    extern void    addTermFunc(RFunc_t func);

}


