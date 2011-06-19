/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCLASS_H_
#define m_HYCLASS_H_

#include "hySymbolID.h"
#include "hyGC.h"
#include "hyBMap.h"
#include "hyTuning.h"
#include "hySignature.h"


// FFIテーブル参照宣言
#define EXTERN_BYTECODE_FFI(codename)             \
  namespace Hayat { namespace Ffi { namespace HScb_##codename {    \
    extern const Hayat::Engine::HClass::ClassFfiEntry_t CLASS_FFI_TABLE; \
  } } }

// FFIテーブルリンク
#define LINK_BYTECODE_FFI(codename)               \
  Hayat::Engine::HClass::linkRootFfiTable(&Hayat::Ffi::HScb_##codename::CLASS_FFI_TABLE)



/*
  クラスのバイトコードデータ構造については、doc/dataformat/bytecode.txt を参照。
*/

class Test_hyGC;

namespace Hayat {
    namespace Common {
        class MemCell;
    }

    namespace Engine {
        class Object;
        class Context;
        class Value;
        class Bytecode;
        class MethodPtr;


        /// FFI関数型
        typedef void (*Ffi_t)(Context* context, int numArgs);


        class HClass {
            friend class Bytecode;
            friend class Object;
            friend class ::Test_hyGC;
        
        public:
            // 初期化
            // @param classSymbol   このクラスを示すシンボル
            // @param pBytecode     所属するバイトコード
            // @param code          クラスのバイトコードの先頭アドレス
            HClass(SymbolID_t classSymbol, const Bytecode* pBytecode, const hyu8* code);
            // 疑似クラスの初期化
            // 変数やメソッドなどが1つも無いクラスを作成
            HClass(SymbolID_t symbol);
            ~HClass(void);
            static void*    operator new(size_t size);
            static void     operator delete(void* p);

            const hyu8*       classInitializer(void) const {return m_codes;}

            const Bytecode* bytecode(void) const { return m_pBytecode; }
            // selfとinnerからクラスを検索
            const HClass*   getInnerClass(SymbolID_t classSymbol) const;
            // self,inner,super,super->innetからクラスを検索
            const HClass*   getNearClass(SymbolID_t classSymbol) const;
            // using->nearからクラスを検索
            const HClass*   getClassInUsing(SymbolID_t classSymbol) const;
            // 現クラスから見えるクラスを検索
            const HClass*   getScopeClass(SymbolID_t classSymbol, bool bNotSearchUsingPath = false) const;
            size_t          fieldSize(void) const; //c++インスタンスを除くフィールドサイズ
            // インスタンスオブジェクト生成
            // [argn, .., arg1] -> []
            // contextが指定してあれば、クラスメソッド *cppSize とインスタンス
            // メソッド initialize を呼ぶ。
            // initialize中で例外が発生すると NULL を返す。
            // *cppSize が無い場合は context = NULL で呼び出す事ができる。
            // context == NULL または bInit == false の場合initializeは呼ばれない。
            Object*         createInstance(Context* context = NULL, Signature* pSig = NULL, bool bInit = true) const;
            // インスタンスオブジェクト生成
            // *cppSize も initialize も呼ばない。
            // C++インスタンス部分のサイズを指定する。
            Object*         createInstance_noInit(size_t cppFieldSize) const;

            bool            isPrimitive(void) const { return m_bPrimitive; }

            bool            searchMethod(MethodPtr* pMethodPtr, SymbolID_t methodSymbol, Signature* pSig) const;
            void            setClosureMethodPtr(int num, MethodPtr* pmptr) const;
            bool            haveMethod(SymbolID_t methodSymbol) const;

            Value*          getClassVarAddr(SymbolID_t classVarSymbol) const;
            Value*          getConstVarAddr(SymbolID_t constVarSymbol) const;
            Value*          getMembVarAddr(Object* obj, SymbolID_t varSymbol) const;
            Value*          getDefaultValAddr(hyu16 defValIdx) const;
            void            setDefaultVal(hyu16 defValIdx, const Value& val);
            SymbolID_t      getSymbol(void) const { return m_classSymbol; }
            bool            symCheck(SymbolID_t sym) const { return m_classSymbol == sym; }
            void            changeSymbol(SymbolID_t sym);

            bool            isInitialized(void) const { return m_bInitialized; }
            void            initialized(void) { m_bInitialized = true; }

            const hyu8*       name(void) const;

            void            getMethodSignatureBytes(int methodNum, const Signature::Sig_t** pSigBytes, hyu16* pDefValOffs) const;

            bool            doesInherit(const HClass* other) const;


            // 全クラスを走査して1度だけ何かを実行するためのフラグ操作
            void            recSetNodeFlag(hyu8 val = 0);
            hyu8            getNodeFlag(void) { return m_nodeFlag; }
            void            setNodeFlag(hyu8 val) { m_nodeFlag = val; }


        protected:

            static void     m_rewriteJumpTableEndian(const hyu8* table);
            void            m_resolveTable(const HClass* outer);
            void            m_initHashes(void);
            const HClass*   m_super(int n) const { return (const HClass*)m_superTable[n]; }

            bool            m_lookupRoutine(MethodPtr* pMethodPtr, SymbolID_t methodSymbol, Signature* pSig) const;
            bool            m_lookupFfi(MethodPtr* pMethodPtr, SymbolID_t methodSymbol, Signature* pSig) const;
            bool            m_lookupMethod(MethodPtr* pMethodPtr, SymbolID_t methodSymbol, Signature* pSig) const;


            const Bytecode* m_pBytecode;
            const HClass*   m_outerClass;
            SymbolID_t      m_classSymbol;
            bool            m_bPrimitive;
            hyu16             m_numMethods;
            hyu16             m_numInnerClasses;
            hyu16             m_numSuper;
            hyu16             m_numDefaultVal;
            hyu16             m_numClosures;
            SymbolID_t*     m_methodSymTable;
            SymbolID_t*     m_classSymTable;
            TArray<SymbolID_t>  m_classVarSymArr;
            TArray<SymbolID_t>  m_membVarSymArr;
            TArray<SymbolID_t>  m_constVarSymArr;
            hyu32*            m_methodTable; // バイトコード読み込み時にはバイトコードオフセット、読み込んだ時にバイトコードアドレスに変換
            hyu32*            m_innerClassTable; // バイトコード読み込み時にはバイトコードオフセット、読み込んだ時にHClass*に変換
            hyu32*            m_superTable; // バイトコード読み込み時にはクラスリンクID、読み込んだ時にHClass*に変換
            hyu16             m_numUsingPaths;
            HClass**        m_usingPaths;
            Value*          m_varPool;       // クラス変数、定数の実体置き場所
            const hyu8*       m_codes;
            BMap<SymbolID_t, Value*>    m_classVarHash;
            BMap<SymbolID_t, Value*>    m_constVarHash;
            bool            m_bInitialized;
            hyu8            m_nodeFlag;

#ifdef HMD_DEBUG
            hys32             m_defaultValSetLimit;
#endif

            //============================================================
            // FFI (Foreign Function Interface)
            //============================================================
        public:
            /// FFIメソッドテーブル
            struct FfiMethodEntry_t {
                SymbolID_t      methodSymbol;
                hyu16             signatureID;
                hyu16             defaultValueOffset;
                Ffi_t           func;
            };

            /// クラス毎FFIテーブル
            struct ClassFfiEntry_t {
                SymbolID_t              classSymbol;
                hyu16                     numFfis;
                hyu16                     numInnerClasses;
                // methodSymbolでソート済みのffiテーブル
                const FfiMethodEntry_t*	ffiMethodTable;
                // classSymbolでソート済みのinnerclass用FFIテーブル
                const ClassFfiEntry_t* const*	innerClassFfiTable;
                GC::MarkFunc_t		GCMarkFunc;
            };




        protected:
            const ClassFfiEntry_t* m_ffiTable;
            void bindFfiTable(const ClassFfiEntry_t* tbl);
            Ffi_t m_searchFfi(SymbolID_t methodSymbol, Signature* pSig) const;


        protected:
            static TArray<const ClassFfiEntry_t*> m_rootFfiTable;
        public:
            static void initializeRootFfiTable(void);
            static void finalizeRootFfiTable(void);
            static void linkRootFfiTable(const ClassFfiEntry_t* tbl);
            static const ClassFfiEntry_t* findFfiTable(SymbolID_t bytecodeSymbol);

            GC::MarkFunc_t getGCMarkFunc(void) const;



            //============================================================
            // バイトコードリロード
            //============================================================
#ifdef HY_ENABLE_BYTECODE_RELOAD
        public:
            void        keepObsoleteVar(const HClass* oldClass);
            void        copyDataTo(HClass* dest, BMap<const HClass*,const HClass*>* pGenMap, bool bCopyClassVar, bool bCopyConstVar);

            // スーパークラスがリロードされていたら調整する
            void        regulateReloadedSuper(BMap<const HClass*,const HClass*>& genMap);
#endif


            //============================================================
            // GC
            //============================================================
        public:
            // objのメンバ変数をマーク
            void m_GC_mark_membVar(Object* obj) const;
            // クラス変数、定数をマーク
            void m_GC_mark_staticVar(void);

#ifdef HMD_DEBUG
        public:
            void debugPrintAbsName(void) const;
#endif

        };

    }
}

#endif /* m_HYCLASS_H_ */
