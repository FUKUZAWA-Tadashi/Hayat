/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCONTEXT_H_
#define m_HYCONTEXT_H_

#include "hyCodePtr.h"
#include "hyStack.h"
#include "hyCellList.h"
#include "hyThread.h"
#include "hyObject.h"

namespace Hayat {
    namespace Engine {

        extern bool isJumpControlLabel(SymbolID_t label);

        class Fiber;
        class Exception;

        class Context {
            friend class VM;

        public:
            static void* operator new(size_t size);
            static void operator delete(void* ptr);
            static Context* create(void);
            void    destroy(void);

            Context(void);
            ~Context();

            Context* initialize(hyu32 stackSize = 0, hyu32 frameStackSize = 0);
            void    finalize(void);
            void    cleanup(void);  // スタックリセット
            Object* getObj(void) { return Object::fromCppObj(this); }

            hyu8      getCodeHYU8(void);
            hys8      getCodeHYS8(void);
            hyu16     getCodeHYU16(void);
            hys16     getCodeHYS16(void);
            hyu32     getCodeHYU32(void);
            hys32     getCodeHYS32(void);
            hyf32     getCodeHYF32(void);
            SymbolID_t      getCodeSymbolID(void);

            inline void push(Value d) { stack.push(d); }
            inline Value pop(void) { return stack.pop(); }
            inline void pushBool(bool b) { stack.push(Value::fromBool(b)); }
            inline void pushNil(void) { stack.push(NIL_VALUE); }
            void pushInt(hys32 i) { stack.push(Value::fromInt(i)); }
            void pushFloat(hyf32 f) { stack.push(Value::fromFloat(f)); }
            void pushSymbol(SymbolID_t sym) { stack.push(Value::fromSymbol(sym)); }
            void pushString(const char* s) { stack.push(Value::fromString(s)); }
            void pushObj(Object* o) { stack.push(Value::fromObj(o)); }
            void pushClass(const HClass* pClass) {stack.push(Value::fromClass(pClass));}
            void pushList(ValueList* list) {stack.push(Value::fromList(list));}
            bool popBool(void) { return stack.pop().toBool(); }
            hys32  popInt(void) { return stack.pop().toInt(); }
            hyf32  popFloat(void) { return stack.pop().toFloat(); }
            SymbolID_t popSymbol(void) { return stack.pop().toSymbol(); }
            const char* popString(void) { return stack.pop().toString(); }
            Object* popObj(void) { return stack.pop().toObj(); }
#ifdef HMD_DEBUG
            Object* popObj(SymbolID_t sym) {
                Object* obj = popObj();
                HMD_ASSERT(obj->type()->symCheck(sym));
                return obj;
            }
#else
            Object* popObj(SymbolID_t) { return popObj(); }
#endif
            template<typename T> T* popCppObj() {
                Object* obj = popObj(); return obj->cppObj<T>(); }
            template<typename T> T* popCppObj(SymbolID_t sym) {
                Object* obj = popObj(sym); return obj->cppObj<T>(); }

            const HClass* popClass(void) { return stack.pop().toClass(); }
            ValueList* popList(void) { return stack.pop().toList(); }



            const HClass* lexicalScope(void) { return curFrame->lexicalScope; }
            const HClass* ownerClass(void) { return curFrame->ownerClass; }


            // メソッド呼び出し
            // スタックtopがインスタンス
            // インスタンスが Class クラスならば、そのクラスメソッドを呼び出す
            // 失敗すると例外を投げる
            // [instance argn .. arg2 arg1] -> [retval]
            void    methodCall(SymbolID_t methodSymbol, Signature* pSig);
            void    methodCall(SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sig.setNumOnStack(numArgs + 1);
                methodCall(methodSymbol, &sig);
            }
            bool    methodCall_ifExist(SymbolID_t methodSymbol, int numArgs);
            // メソッド呼び出し
            // スタックtopは引数の最後の値
            // インスタンスが Class クラスならば、そのクラスメソッドを呼び出す
            // 失敗すると例外を投げる
            // [argn .. arg2 arg1 instance] -> [retval]
            void    methodCallR(SymbolID_t methodSymbol, Signature* pSig);
            void    methodCallR(SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sig.setNumOnStack(numArgs);
                methodCallR(methodSymbol, &sig);
            }
            bool    methodCallR_ifExist(SymbolID_t methodSymbol, int numArgs);
            // メソッド呼び出し
            // スタックtopは引数の最後の値
            // インスタンスが Class クラスならば、そのクラスメソッドを呼び出す
            // 失敗すると例外を投げる
            // [argn .. arg2 arg1] -> [retval]
            void    methodCall(const Value& instance, SymbolID_t methodSymbol, Signature* pSig);
            void    methodCall(const Value& instance, SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sig.setNumOnStack(numArgs);
                methodCall(instance, methodSymbol, &sig);
            }
            bool    methodCall_ifExist(const Value& instance, SymbolID_t methodSymbol, int numArgs);


            // 暗黙self、もしくはクラスメソッド、もしくは組み込み関数呼び出し
            // [argn .. arg2 arg1] -> [retval]
            void    sMethodCall(SymbolID_t methodSymbol, Signature* pSig);
            void    sMethodCall(SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sMethodCall(methodSymbol, &sig);
            }
            // 指定クラスのメソッド呼び出し
            // 暗黙self、もしくはクラスメソッド、もしくは組み込み関数を呼び出す
            // [argn .. arg2 arg1] -> [retval]
            void    scopeMethodCall(SymbolID_t methodSymbol, const HClass* pClass, Signature* pSig);
            void    scopeMethodCall(SymbolID_t methodSymbol, const HClass* pClass, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                scopeMethodCall(methodSymbol, pClass, &sig);
            }

            void    callBytecodeTop(const Bytecode* pBytecode);
            bool    execBytecodeTop(const Bytecode* pBytecode);

            // メソッド実行
            // [instance argn .. arg2 arg1] -> [retval]
            // 失敗すると例外を投げる
            bool    execMethod(SymbolID_t methodSymbol, Signature* pSig);
            bool    execMethod(SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sig.setNumOnStack(numArgs + 1);
                return execMethod(methodSymbol, &sig);
            }
            // メソッド実行
            // [argn .. arg2 arg1 instance] -> [retval]
            // 失敗すると例外を投げる
            bool    execMethodR(SymbolID_t methodSymbol, Signature* pSig);
            bool    execMethodR(SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sig.setNumOnStack(numArgs + 1);
                return execMethodR(methodSymbol, &sig);
            }
            // メソッド実行
            // [argn .. arg2 arg1] -> [retval]
            // 失敗すると例外を投げる
            bool    execMethod(const Value & instance, SymbolID_t methodSymbol, Signature* pSig);
            bool    execMethod(const Value & instance, SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                sig.setNumOnStack(numArgs);
                return execMethod(instance, methodSymbol, &sig);
            }

            // メソッド実行
            // [argn .. arg2 arg1] -> [retval]
            // メソッドが無ければ 0, あって実行成功は 1, 実行失敗は -1 を返す
            int     execMethod_ifExist(const Value& instance, SymbolID_t methodSymbol, Signature* pSig);
            int     execMethod_ifExist(const Value& instance, SymbolID_t methodSymbol, int numArgs) {
                Signature sig((hyu8)numArgs, this);
                return execMethod_ifExist(instance, methodSymbol, &sig);
            }

            Value   getLocalVar(int varIdx);
            void    setLocalVar(int varIdx, const Value& val);
            Object* shareLocalVar(int outVarIdx);
            Value   getMembVar(SymbolID_t varSym);
            void    setMembVar(SymbolID_t varSym, const Value&  val);
            Value   getClassVar(SymbolID_t varSym);
            void    setClassVar(SymbolID_t varSym, const Value&  val);
            Value   getGlobalVar(SymbolID_t varSym);
            void    setGlobalVar(SymbolID_t varSym, const Value&  val);
            Value   getConstVar(SymbolID_t varSym);
            void    setConstVar(SymbolID_t varSym, const Value&  val);
            void    setDefaultVal(hyu16 defValIdx, const Value& val) { const_cast<HClass*>(m_getSelfClass())->setDefaultVal(defValIdx, val); }

            // 例外throw
            void    throwException(SymbolID_t type, Value exVal);
            void    throwException(Exception* e);
            void    throwJumpCtlException(SymbolID_t label, Value val); // next,break用
            void    throwGotoException(SymbolID_t label, Value val); // goto用
            // printf形式でメッセージを作り、throwする
            void    throwException(SymbolID_t type, const char* msg, ...);
            // tryでcatchできない例外をthrow
            void    throwError(Value exVal);
            // printf形式でメッセージを作り、エラーとしてthrowする
            void    throwError(const char* msg, ...);


            // numArgs個の引数を、modeに従ってconcatし、それぞれの間に , を挟み
            // 表示して、引数をスタックから削除
            void    debugPrintValues(int numArgs, int mode, const char* m);


            void    jumpControlStart(void);
            void    jumpControlEnd(void);
        

            // 実行中フレームを、別Contextに移動する
            Context* fork(int stackAdjust);

        protected:
            // [self argn .. arg1]
            // selfがClassクラスならば、クラスメソッド呼び出し、
            // そうでなければインスタンスメソッド呼び出し
            // 呼び出しに成功すれば true を返す
            bool    m_methodCall(SymbolID_t methodSymbol, Signature* pSig);
            // [argn .. arg1 self]
            // selfがClassクラスならば、クラスメソッド呼び出し、
            // そうでなければインスタンスメソッド呼び出し
            // 呼び出しに成功すれば true を返す
            bool    m_methodCallR(SymbolID_t methodSymbol, Signature* pSig);
            // [argn .. arg1]
            // selfがClassクラスならば、クラスメソッド呼び出し、
            // そうでなければインスタンスメソッド呼び出し
            // 呼び出しに成功すれば true を返す
            bool    m_methodCall(const Value& self, SymbolID_t methodSymbol, Signature* pSig);
            // メソッドを検索して結果をmethodPtrに格納する
            bool    m_buildMethodPtr(MethodPtr& methodPtr, SymbolID_t methodSymbol, Signature* pSig);

            // メソッドを検索した結果がmethodPtrに入っている状態でメソッドを呼び出す
            void    m_doMethodCall(MethodPtr& methodPtr, SymbolID_t methodSymbol, Signature* pSig);

            // トップレベルメソッド(バイトコードメソッド、Objectクラスメソッド)を呼び出す
            bool    m_topMethodCall(SymbolID_t methodSymbol, Signature* pSig);
            // 指定クラスのメソッド呼び出し
            bool    m_scopeMethodCall(SymbolID_t methodSymbol, const HClass* pClass, Signature* pSig);
            // フレーム消費せずに実行、現在のフレームまで戻った所で終了
            // 例外が発生して現在のフレームを突き抜けて呼び出し元まで戻って
            // しまった場合は false を返す
            bool    m_doExec(void);

            // selfがインスタンスならそのインスタンスのクラス、
            // クラスならそのクラスを返す
            const HClass* m_getSelfClass(void) { return curFrame->self.getScope(); }
            // 共有も考慮したローカル変数アドレスを取得
            Value*  m_getLocalVarAddr(hyu32 pos);
        
            // method_not_found例外を投げる
            void    m_throwMethodNotFoundException(const Value& self, SymbolID_t methodSym);


        public:
            // FFIコール操作
            void    FfiCall(const MethodPtr& methodPtr, hyu8 numArgs, SymbolID_t methodSymbol);
            // FFI終了後に呼び出す関数
            typedef void (*Aff_t)(Context* context);
            // FFI中の例外を処理する関数
            // 処理したならtrue,処理せずスルーならfalseを返す
            typedef bool (*FfiExceptionCatcher_t)(Context*, Exception*);
            // FFI終了後に呼び出す関数の登録
            void        setAfterFfiFunc(Aff_t func);
            void        setAfterFfiParam(void* param);
            void*       getAfterFfiParam(void);
            // FFI呼出し状態か？
            bool        isInFfi(void);
            // FFI終了後の関数呼出し
            void        afterFfi(void);
            // FFI終了後の標準処理
            void        ffiFinish(void);
            // FFI終了の強制処理
            void        ffiKill(void);
            // FFI中の例外を処理する関数の登録
            void        setFfiExceptionCatcher(FfiExceptionCatcher_t);


            // サブルーチンコール操作
            void    routineCall(const MethodPtr& methodPtr, hyu8 numArgs);
            // サブルーチンリターン操作
            void    routineReturn(hyu16 callAwayMethodID = (hyu16)-1);
            // サブルーチンを抜ける例外をfiberに投げるように設定する
            // routineCallの直後に呼ぶ
            void    exceptionToFiber(Fiber* exfiber);
            // 相対ジャンプ操作
            void    jumpRelative(hys32 relative);

            // finally後のtry節の値をセット
            void    valueAfterFinally(Value& val);
            // finally終了、元の処理に復帰
            void    endFinally(void);



            hyu16     getMethodID(void) { HMD_DEBUG_ASSERT(! curFrame->bFfi); return curFrame->r.methodID; }
            hys32     callStackLevel(void) { return frameStack.size() + 1; }

        public:
            void    m_GC_mark(void);
            void    m_GC_mark_stack(void);


        protected:
            enum {
                FINALLY_EXIT_TYPE_NONE,
                FINALLY_EXIT_TYPE_NORMAL,
                FINALLY_EXIT_TYPE_EXCEPTION,
                FINALLY_EXIT_TYPE_RETURN
            };
            struct JumpControlInfo_st {
                hyu16       tableNo;
                hys16       catchVar;      //catchのローカル変数
                hys16       finallyValVar; //finally終了後に使う値をストアしておくローカル変数
                hyu16       finallyCallAwayMethodID; //return->finally時に使用
                hyu8        finallyExitType;
                hyu32       stackLv;
                union {
                    const hyu8*   startAddr;
                    Fiber*  fiber; // tableNo == EXCEPTION_TO_FIBERの時有効
                };
            };
            static const hys16 NO_CATCH_VAR = -0x8000;
            static const hyu16 EXCEPTION_TO_FIBER = 0xffff;

            // try-catch-finally, break,next, goto 情報
            typedef CellList<JumpControlInfo_st>   JumpControlList;


        public:
            // finally実行中か？
            static inline bool isInFinally(const JumpControlInfo_st& inf) {
                return (inf.finallyExitType != FINALLY_EXIT_TYPE_NONE); }
            // ジャンプアドレスを取得。無ければ NULL を返す
            const hyu8* getJumpAddr(SymbolID_t label);
            const hyu8* getJumpAddr(SymbolID_t label, const JumpControlInfo_st& inf);

        public:
            /// バイトコードメソッド呼び出し情報型
            struct RoutineCallParam_st {
                const hyu8* jumpControlTableTop;  ///< JumpControl情報テーブル
                hyu8          numLocals;          ///< ローカル変数の数
                hyu16         methodID;           ///< メソッドID
                hyu16         creatorMethodID; ///< これを生成したメソッドのID
                /// メソッドIDは、クラス初期化コードなら 0
                /// メソッドなら メソッド番号+1
                /// クロージャなら クロージャ番号+メソッド総数+1
                JumpControlList*    jumpControls;     ///< JumpControl情報リスト
            };
            /// FFI呼び出し情報型
            struct FfiParam_st {
                SymbolID_t  methodSymbol;
                Aff_t       afterFfiFunc;
                void*       afterFfiParam;
                const Context* ffiContext;
                FfiExceptionCatcher_t   exceptionCatcher;
            };


            // コールスタック
            struct Frame_st {
                hyu32     framePos; ///< stackのframePosからnumArgs個の引数がある
                Value   self;     ///< 暗黙のself

                /// 実行中のコードの属するクラス
                /// リロードすると新しいクラスを指す
                const HClass* lexicalScope;

                /// 実行中のコードを持つクラス
                /// リロードしても、これは変更されない
                const HClass* ownerClass;

                CodePtr returnCodePtr;  ///< リターンアドレス

                hyu8      numArgs;  ///< 引数の数

                bool    bFfi;   ///< trueならfが有効、falseならrが有効
                union {
                    RoutineCallParam_st r;
                    FfiParam_st         f;
                };
            };

            ValueStack          stack;
            CodePtr             codePtr;
            Stack<Frame_st>     frameStack; // サブルーチン呼び出しスタック
            Frame_st*           curFrame; // 実行中フレーム

            Thread*         thread;
            Fiber*      fiber;

            // 定数に代入可能な状態かどうか
            bool            canSubstConst;
            // exec前にfalseにして、throwExceptionでtrueになる
            bool        throwFlag;



#ifdef HMD_DEBUG
        public:
            void    debugStackTrace(void);
            void    debugPrintStack(void);
            void    debugPrintCurPos(void) { m_debugStackTrace1(-1, codePtr, *curFrame); }

        protected:
            void    m_debugStackTrace(void);
            void    m_debugStackTrace1(int level, const CodePtr codePtr, Frame_st& fr);
#else
        public:
            void    debugStackTrace(void) {}
            void    debugPrintStack(void) {}
            void    debugPrintCurPos(void) {}
#endif

        };

    }
}

#endif /* m_HYCONTEXT_H_ */
