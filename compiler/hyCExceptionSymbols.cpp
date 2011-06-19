/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


// エンジン内での例外送出時に使われるシンボル定義
//
// hyCCompiler.cpp の initializeCompiler() の中で、シンボルに変換される。

#include "hyStaticAssert.h"
using namespace Hayat::Common;


namespace Hayat {
    namespace Compiler {
        extern const char* exceptionSymbols[] = {
            "fatal_error",      // 致命的エラー
            "type_mismatch",    // 型が期待していたものと違う
            "fiber_return",     // Fiberでyieldを期待していたのにreturnが実行された
            "fiber_error",      // Fiberの実行に失敗
            "method_not_found", // メソッドが見つからなかった
            "array_index_out_of_bounds", // 配列添字が範囲外を指している
            "invalid_argument",          // 引数異常
            "use_member_var_out_of_instance", // インスタンスでないのにメンバ変数にアクセスしようとした
            "division_by_zero",               // 0で割り算
            "empty_list",                     // 空リストだった
            "closure_return",                 // closureのreturn先が無い
            "signature_mismatch",             // 引数シグネチャがマッチしない
            "type_error",                     // 引数の型が合わない
            "cannot_access_member",           // メンバにアクセスできない
            "cannot_access_constant",         // 定数にアクセスできない
            "cannot_access_scoped_val",       // スコープ付きの定数またはクラスにアクセスできない
            "file_reload_failed",             // リロード失敗
            "bad_constructor",                // コンストラクタが正しいインスタンスを返さなかった
            "unknown_identifier",             // 不明な識別子
            "bad_ffi_call",                   // FFI呼出しが正しくない
            "ffi_failed",                     // FFIの実行中にエラーがあった

            (const char*)0      // end mark
        };

        STATIC_ASSERT(sizeof(exceptionSymbols)/sizeof(exceptionSymbols[0]) < 100, exceptionSymbols_size_limited_in_initializeCompiler);
    }
}
