/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


#ifndef m_HYTUNING_H_
#define m_HYTUNING_H_


// バイトコードリロード機能を使用するか
#define HY_ENABLE_BYTECODE_RELOAD

// Objectの移動を許可するかどうか
// 移動を許可すると、GCのオーバーヘッドが高くなり、スピード低下につながる
#define HY_ENABLE_RELOCATE_OBJECT

// バイトコードリロード機能が有効なら、Objectの移動を許可しなければならない
// ↓以下4行は変更してはいけない
#ifdef HY_ENABLE_BYTECODE_RELOAD
#undef HY_ENABLE_RELOCATE_OBJECT
#define HY_ENABLE_RELOCATE_OBJECT
#endif


// Hash initial capacity 
#define HASH_DEFAULT_INIT_CAPACITY	31
// Hash load factor 75%
#define HASH_DEFAULT_LOAD_FACTOR	75


// 配列容量がこれ未満の場合は、増やす時に元容量の2倍とする
#define ARRAY_CAPACITY_DOUBLE_LIMIT	    0x20
// 配列容量がこれ未満の場合は、増やす時に元容量の1.5倍とする
#define ARRAY_CAPACITY_ONE_HALF_LIMIT	0x80
// 配列容量がそれ以上の場合の増分値
#define ARRAY_CAPACITY_INCREMENTS	    0x40


// finally がネスティングできる最大数
#define MAX_FINALLY_NEST                16


// コンパイル時にシンボルを標準出力へ出力
//#define EXPERIMENT_SYMBOL_DUMP
#undef EXPERIMENT_SYMBOL_DUMP


// 使用可能残メモリ計算を厳密にするかどうか
#ifdef TEST__CPPUNIT
#define EXACT_FREEMEMSIZE
#else
#undef EXACT_FREEMEMSIZE
//#define EXACT_FREEMEMSIZE
#endif

// バイトコード実行履歴を記録するかどうか
// VMがhaltした時に実行履歴が記録されていれば表示される
#ifdef HMD_DEBUG
#define HY_ENABLE_BYTECODE_HISTORY
#else
//#undef HY_ENABLE_BYTECODE_HISTORY
#define HY_ENABLE_BYTECODE_HISTORY
#endif


// FFIの自動生成ファイル *.h *.cpp のファイル名を生成する時、
// クラス名とインナークラス名の間に挟む文字列
#define HY_GEN_FNAME_DELIM  "__"


#endif /* m_HYTUNING_H_ */
