/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_COMPILEFILE_H_
#define m_COMPILEFILE_H_


namespace Hayat {
    namespace Compiler {


        struct CompileOption {
            // パース途中状態メッセージを出すレベル 0～2
            int         printIntermediateLevel;

            // ジャンプ先ラベル存在チェックレベル
            // 0 = チェックしない
            // 1 = ファイル内にラベルが無ければWarning
            // 2 = ブロック内にラベルが無ければWarning
            int         Wlabel;
    
            // sayCommand通し番号総数がこれ以上になるとWarning
            int         Wnumsay;

            // WlabelをWarningではなくErrorとする
            bool        WlabelIsError;

            // WnumsayをWarningではなくErrorとする
            bool        WnumsayIsError;

            // 所在が不明なクラスやメソッドがあった場合の動作を指定する。
            // 0 = チェックしない
            // 1 = WARNINGを出す
            // 2 = ERRORを出して停止
            int         Wunknown;

            // classの外のスコープの定数名と同じ定数名をどう扱うか
            // 0 = エラーにならず使用可能
            // 1 = WARNINGを出すが使用可能
            // 2 = ERRORを出して停止
            int         Wconstscope;

            // パース完了時に構文木を出力
            bool        bPrintSyntaxTree;

            // パース時にメモ化機能を使うかどうか
            // メモ化を使うと高速だがメモリが大量に必要、使わなければ低速低メモリ
            bool        bUseMemoize;

            // 冗長メッセージフラグ
            bool        bVerbose;

            // 翻訳メッセージファイル名
            const char* mmesFileName;
        };


        extern CompileOption compileOption;

        int compileFile(const char* hyFileName);

    }
}

#endif /* m_COMPILEFILE_H_ */
