/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCCONTEXT_H_
#define m_HYCCONTEXT_H_

#include "hyStack.h"
#include "hyArray.h"
#include "hyCSymbolTable.h"
#include "hyCBytecode.h"
#include "hyCPackage.h"
#include "hyCJumpControlTree.h"
#include "hyCSignature.h"
#include "hyC_opcode.h"
#include <stdio.h>

class Test_hyCContext;

namespace Hayat {
    namespace Compiler {

        class JumpControlTree;

        class Context {
            friend class ::Test_hyCContext;

        public:
            static void* operator new(size_t size);
            static void operator delete(void* p);

            // 全体の初期化
            static void initializeAll(void);
            // 全体の後始末
            static void finalizeAll(void);

            // カレントコンテキストを返す
            static Context* current(void) { return m_current; }

            // パッケージクラスコンテキストを作って最初のコンテキストとする
            static void newPackageClass(ClassInfo*);

            // コンテキストをスタックに積み、カレントとする
            static void push(Context* child);
            // スタックからカレントを降ろして1つ前のをカレントとする
            static void pop(void);
            // カレントの子コンテキストを作ってスタックに積み、新たなカレントとする
            static Context* createChild(void);
            // カレントのインナークラスコンテキストを作ってスタックに積み、カレントとする
            static Context* createInnerClass(SymbolID_t classSym);
            // 現パッケージを取得
            static Package* getPackage(void) { return m_package; }

            // sayCommandStart()に渡すシリアルナンバー
            static hys32 sayCommandIndexCount(void) { return m_sayCommandIndex++; }
            // sayCommandStart()に渡すシリアルナンバー：カウントアップしない
            static hys32 sayCommandIndex(void) { return m_sayCommandIndex; }

            // goto の飛び先ラベルが全Context中にあるかどうかチェック
            static void jumpLabelCheck(void);

        protected:
            static Stack<Context*> m_contextStack;
            static Context* m_current; // カレントコンテキスト
            static TArray<Context*> m_pool; // 全Contextを記憶
            static Package* m_package;
            static hys32 m_sayCommandIndex;

        public:
            Context(Context* outerContext = NULL);
            ~Context();

            ClassInfo*  classInfo(void) { return m_classInfo; }
            Context*    outerContext(void) { return m_outerContext; }

            // class開始
            void newClass(SymbolID_t classSym);
            // スーパークラス追加
            void addSuper(SymbolID_t superClassSym, Context* searchContext);

            // 最終調整
            void postCompileProcess(hyu16 defaultValOffs, hyu16 signatureID);

            // スコープに対応するClassInfoを検索
            ClassInfo* getScopeClassInfo(Scope_t scope, bool ignoreUsing = false);

            typedef struct {
                hys16     idx;            // インデックス
                hys16     outerIdx;       // 外側contextと共有の場合
                bool    substFlag;      // 代入フラグ
            } LocalVar_t;
            static const hys16 NO_LOCAL_VAR = (hys16)0x8000; // ローカル変数が存在しないという意味のidx

            // ====== 識別子の検索 ======
            // 今のスコープと外のスコープにローカル変数があるか
            LocalVar_t* getMyLocalVar(const char* p, hyu32 len, LocalVarSymID_t* pSym = NULL);
            // 今のスコープと親クラスに識別子があるか
            Var_t       myIdent(const char* p, hyu32 len);
            Var_t       myIdent(InputBuffer* inp, Substr_st& ss) {
                return myIdent(inp->addr(ss.startPos),ss.len()); }

            Var_t       searchVar(const char* p, hyu32 len);
            Var_t       searchVar(InputBuffer* inp, Substr_st& ss) {
                return searchVar(inp->addr(ss.startPos),ss.len()); }

            // ローカル変数が存在すればそれを返す(外側コンテキストは無視)
            LocalVar_t* getLocalVar(LocalVarSymID_t varSym) { return m_localVars.find(varSym); }
            // 外側コンテキストの可視ローカル変数を返す
            LocalVar_t* getOuterLocalVar(SymbolID_t varSym);
            // 可視ローカル変数が無ければ作成
            LocalVar_t& getLocalVarCreate(LocalVarSymID_t varSym);

            // メソッドが最初に確保すべきローカル変数の数
            int numLocalVarAlloc(void) { return m_localVars.size() - m_numParamVar; }
            // メソッド仮引数作成
            void createParamVar(LocalVarSymID_t varSym);

            // シグネチャバイト列をIDに変換
            hyu16 getSignatureID(const hyu8* sigBytes, hyu32 len) {
                return m_package->getSignatureID(sigBytes, len); }

            // 代入の無いローカル変数をチェック
            void checkNoSubstLocalVar(void);

            // 保持しているバイトコード
            Bytecode& bytecode(void) { return m_bytecode; }

            // 他パッケージリンク
            void require(SymbolID_t sym) { m_package->require(sym); }

            template <typename OPCODE> void addCode(void) {
                OPCODE::addCodeTo(m_bytecode);
            }
            template <typename OPCODE, typename OPERAND1> void addCode(OPERAND1 opr1) {
                OPCODE::addCodeTo(m_bytecode, opr1);
            }
            template <typename OPCODE, typename OPERAND1, typename OPERAND2> void addCode(OPERAND1 opr1, OPERAND2 opr2) {
                OPCODE::addCodeTo(m_bytecode, opr1, opr2);
            }
            template <typename OPCODE, typename OPERAND1, typename OPERAND2, typename OPERAND3> void addCode(OPERAND1 opr1, OPERAND2 opr2, OPERAND3 opr3) {
                OPCODE::addCodeTo(m_bytecode, opr1, opr2, opr3);
            }
            void addCodePushInt(hys32 v);
            void addCodePushFloat(hyf32 v);
            bool removeLastPop(void) { return m_bytecode.removeLastCode(OPL_pop); }

            void setClosureContext(void) { m_bClosureContext = true; m_canSeeOuterLocal = true; }
            bool isClosureContext(void) { return m_bClosureContext; }

            // メソッド追加
            void addMethod(SymbolID_t methodSym, Context* methodContext);
            // インナークラス登録
            void addInnerClass(Context* classContext);
            // クロージャ登録
            void addClosure(Context* callContext, Context* closureContext);

            // 現在のバイトコード登録位置
            hyu32 codeAddr(void) { return m_bytecode.getSize(); }


            // ラベル登録
            void addLabel(SymbolID_t labelSym) {
                m_jumpControlInfo->addLabel(labelSym, codeAddr());
            }
            // jump controlにおけるジャンプコード追加
            template <typename OP_JMP> hyu32 addGotoCode(SymbolID_t labelSym = SYMBOL_ID_ERROR) {
                hyu32 pos = codeAddr() + 1;
                if (labelSym == SYMBOL_ID_ERROR) {
                    addCode<OP_JMP>(-2);
                } else {
                    hyu32 adr = m_jumpControlInfo->getLocalLabelAddr(labelSym);
                    if (adr == JumpControlTree::INVALID_ADDR) {
                        // 未解決ラベル
                        addCode<OP_JMP>(-3);
                        m_jumpControlInfo->addResolveAddr(labelSym, pos);
                    } else {
                        addCode<OP_JMP>(adr - pos - OPR_RELATIVE::SIZE);
                    }
                }
                return pos;
            }
            // jump control 開始
            void jumpControlStart(SymbolID_t catchVar = SYMBOL_ID_ERROR, SymbolID_t finallyValVar = SYMBOL_ID_ERROR);
            // jump control 終了
            void jumpControlEnd(void);
            // jump controlのアドレス解決
            void resolveJumpControl(void);
            // ジャンプアドレス解決 jumpAddrを省略すると現在位置
            void resolveJumpAddr(hyu32 resolveAddr, hyu32 jumpAddr = JumpControlTree::INVALID_ADDR);
            // jump命令をjumpControl命令に置き換える
            void replaceJumpControl(hyu32 resolveAddr, SymbolID_t label);
            // finallyで使用する一時的な変数を用意
            LocalVarSymID_t     useFinallyValVar(void);
            // finallyでの一時変数使用終了
            void        endUseFinallyValVar(void);

            // ジャンプラベルが登録されているか
            bool haveLabel(SymbolID_t label);
            // resolve後に解決されなかったジャンプラベル
            const TArray<SymbolID_t>& getUnresolvedJumpControlLabels(void) { return m_unresolvedJumpControlLabels; }



            // ソースコード位置情報をセット
            void setSourceInfo(hyu32 parsePos);


            // バイトコード情報をArrayに出力
            void writeByteCodes(TArray<hyu8>* out);
            // バイトコード情報をファイル出力
            void fwriteByteCodes(FILE* fp);
            // パッケージリンク情報をファイル出力
            void fwriteLinks(FILE* fp);
            // デバッグ情報をファイル出力
            void fwriteDebugInfo(FILE* fp, hyu32 offs);
            // FFI宣言からC++ソースを作成
            void writeFfi(void);
            // FFI呼出し用関数テーブルのC++ソースを出力
            void writeFfiTbl(const char* ffiTblFileName);


        protected:
            // 自分のレベル以下のデバッグ情報をファイル出力
            void m_fwriteDebugInfo(FILE* fp, TArray<const char*>& paths, hyu32 offset);

        protected:
            Context*    m_outerContext;
            ClassInfo*  m_classInfo;
            BMap<LocalVarSymID_t, LocalVar_t>        m_localVars;
            hyu8        m_numParamVar;
            hyu8        m_finallyNestLevel;
            bool        m_canSeeOuterLocal;
            bool        m_bClosureContext;
            Bytecode    m_bytecode;
            TArray<JumpControlTree*>    m_jumpControlTbl;
            JumpControlTree*            m_jumpControlInfo;
            // resolve後にjumpControl命令となったラベル
            TArray<SymbolID_t>          m_unresolvedJumpControlLabels;
            BMap<SymbolID_t, Context*>  m_innerClasses;


        public:
            // SyntaxTree::compile()の結果、スタックに値をpushするコードを追加したか
            bool        bPushCode;
            // bPushCodeがfalseなら、push_nilする
            void        needLastVal(void) { if (!bPushCode) { addCode<OP_push_nil>(); bPushCode = true; } }
            // bPushCodeがtrueなら、popする
            void        popLastVal(void) { if (bPushCode) { addCode<OP_pop>(); bPushCode = false; } }

            // leftValueをコンパイルする時、falseなら値取り出し、trueなら代入
            bool        bCompileSetVal;

            // SyntaxTree::compile()が何かの値を一時的に記憶させるための変数
            volatile union {
                hys32             tmp_int;
                hyu32             tmp_hyu32;
                hyf32             tmp_float;
                SymbolID_t      tmp_symbol;
                void*           tmp_ptr;
                ArgD_t          tmp_argD;
                FfiSigDecl_t tmp_ffiSigDecl;
                Signature*      tmp_signature;
            };
        };

    }
}

#endif /* m_HYCCONTEXT_H_ */
