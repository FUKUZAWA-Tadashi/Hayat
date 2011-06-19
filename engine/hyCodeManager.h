/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCODEMANAGER_H_
#define m_HYCODEMANAGER_H_

#include "hySymbolID.h"
#include "hyBMap.h"
#include "hyValueArray.h"
#include "hyStringBox.h"

using namespace Hayat::Common;

namespace Hayat {
    namespace Engine {
        class Bytecode;
        class Thread;
        class Context;
        class HClass;

        class CodeManager {
            friend class GC;
        public:

            // バイトコードをメモリにロードした直後に呼ぶコールバックの型
            typedef void    (*LoadedCallback_t)(const char*);


            CodeManager(void);
            ~CodeManager();

            void    initialize(void);
            void    finalize(void);
            // バイトコード読み込み
            Bytecode* readBytecode(const char* filename);
            // バイトコードをメモリにロードした直後に呼ぶコールバックを登録
            void    setBytecodeLoadedCallback(LoadedCallback_t cb) { m_loadedCallback = cb; }
            // バイトコードを管理に追加
            // bAutoloadがtrueだと、バイトコードの後始末は内部でやる
            void    addBytecode(Bytecode* pBytecode, bool bAutoload = false);
            Bytecode* getBytecode(SymbolID_t bytecodeSym);
            void    deleteBytecode(Bytecode* pBytecode);

            void    clearAllFlag(void);
            void    setFlag(SymbolID_t bytecodeSym);
            bool    getFlag(SymbolID_t bytecodeSym);

            // バイトコードが無ければ読み込む
            // 読み込まれたバイトコードは、finalize()を呼ぶまで開放しない。
            // バイトコードの後始末は内部でやる
            // todo: バイトコードもGC対象にする
            Bytecode* require(SymbolID_t bytecodeSym, const hyu8* bytecodeName);

            // GCに対応できる Context を作成。
            // ここで作成したContextは、他のオブジェクトから参照されていなくても
            // GCで回収されない。releaseContext()で、GCに回収されるようになる。
            // 逆に、releaseContext()を呼ばずに放置すると、GCで回収されずに
            // ずっと残る事になるので注意。
            Context* createContext(void);
            // createContext()で作ったContextを解放する
            void     releaseContext(Context*);
            // 他所で作成したContextを、GCに対応できるようにここで登録する
            void     addContext(Context*);
            // addContext()で登録したContextを管理から外す
            void     removeContext(Context*);

            void     setWorkingBytecode(const Bytecode* pBytecode) { m_workingBytecode = pBytecode; }
            const Bytecode* getWorkingBytecode(void) { return m_workingBytecode; }


            // リロード：以前のバイトコードを上書きする形でロードする
            Bytecode*   reloadBytecode(const char* filename, SymbolID_t asName = SYMBOL_ID_ERROR, bool bInit = true);
            // 以前のバイトコードを上書きする
            void    overrideBytecode(Bytecode* pBytecode, SymbolID_t asName = SYMBOL_ID_ERROR, bool bInit = true);
            // リロード時に旧クラスにあって新クラスに無い変数を残すかどうかを設定
            void    setKeepObsoleteVar_whenReload(bool b) { m_ov_keepObsoleteVar = b; }
            // リロード時にクラス変数値を旧クラスから新クラスにコピーするかを設定
            void    setCopyClassVar_whenReload(bool b) { m_ov_copyClassVar = b; }
            // リロード時に定数値を旧クラスから新クラスにコピーするかを設定
            void    setCopyConstVar_whenReload(bool b) { m_ov_copyConstVar = b; }
            // リロードされ置き替えられたバイトコードで、まだメモリ上に
            // 残っているもののシンボルを引数のArrayに入れる
            void    takeReplacedBytecodeSyms(ValueArray* arr);


            // 置き換えられたバイトコードの使用中フラグをクリア
            void    clearCodeUsingFlag(void);
            // codeAddrの部分のバイトコードが使用中である事を登録
            void    usingCodeAt(const hyu8* codeAddr);

            void    unmarkStringBox() { m_stringBox.unmark(); }
            // 文字列が置き換えられたバイトコードの中のテーブルを指していたら、
            // m_stringBoxにコピーしてそちらを指すように変更
            // m_stringBoxの中を指していたらそれをマーク
            void    markString(const char** pStr);
            void    sweepStringBox() { m_stringBox.sweep(); }

            // どこからも参照されていないバイトコードを削除
            void    deleteUnnecessaryBytecode(void);

        public:
            void     m_GC_mark(void);

            // GCのmarkフェーズのインクリメンタル実行
            void            startMarkIncremental(void);
            void            markIncremental(void);
            void            startMarkIncremental_2(void);
            void            markIncremental_2(void);
            void            markAllStack(void);


        protected:
            TArray<Context*>    m_contexts;

            LoadedCallback_t    m_loadedCallback;

            struct tbl_st {
                Bytecode*   bytecode;
                bool        flag;
                bool        autoloadFlag;
            };

            // 有効なバイトコード
            BMap<SymbolID_t, tbl_st>    m_tbl;

            tbl_st*         m_searchTbl(SymbolID_t bytecodeSym) { return m_tbl.find(bytecodeSym); }


            struct repl_st {
                Bytecode*   bytecode;
                bool        inUse; // バイトコード実行中フラグ
            };

            // 置き換えられたバイトコード
            TArray<repl_st>             m_replaced;

            // 置き換えられたバイトコード中にあって、まだ使われている
            // 文字列を、ここに移動する
            StringBox       m_stringBox;

            // リロード時に旧クラスにあって新クラスに無い変数を残すかどうか
            bool            m_ov_keepObsoleteVar;
            // リロード時にクラス変数値を旧クラスから新クラスにコピーするか
            bool            m_ov_copyClassVar;
            // リロード時に定数値を旧クラスから新クラスにコピーするか
            bool            m_ov_copyConstVar;


            const Bytecode*     m_workingBytecode;
        };


        extern CodeManager      gCodeManager;

    }
}
#endif /* m_HYCODEMANAGER_H_ */
