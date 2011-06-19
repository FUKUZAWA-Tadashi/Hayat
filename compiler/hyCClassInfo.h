/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCCLASSINFO_H_
#define m_HYCCLASSINFO_H_

#include "hyBMap.h"
#include "hySymbolID.h"
#include "hyCFfiType.h"
#include "hyCStrTable.h"
#include "hyCSignature.h"
#include "hyTuning.h"
#include <stdio.h>

class Test_hyCClassInfo;

namespace Hayat {
    namespace Compiler {

        class ClassInfo;

        typedef TArray<SymbolID_t> Scope_t;

        typedef struct {
            hyu32 argName;        // m_nameTable のオフセット
            hyu32 typeName;       // m_nameTable のオフセット
            bool haveDefaultVal; // デフォルト値を持つかどうか
        } ArgD_t;
        static const hyu32        NULL_STR_OFFS = (hyu32)-1;
        typedef struct {
            TArray<ArgD_t>*     argDecls;       // 可変長引数の時はNULL
            hyu16 signatureID;
            hyu16 defValOffs;
            hyu32 retType;        // m_nameTable のオフセット
        } FfiSigDecl_t;

        typedef enum {
            CONST_TYPE_NONE, CONST_TYPE_INT, CONST_TYPE_FLOAT,
        } ConstType_e;
        typedef struct {
            ConstType_e type;
            union {
                hys32     intValue;
                hyf32     floatValue;
            };
        } ConstVar_t;

        typedef enum {
            MT_NONE,    // 見つからない
            MT_OK,      // メソッドあり
            MT_CLASS,   // クラスメソッド呼び出しのみ可
        } MethodCallable_e;

        typedef enum {
            IT_NONE,        // 不明
            IT_LOCAL,       // ローカル変数名
            IT_CLASS,       // クラス名
            IT_CONST,       // 定数名
            IT_METHOD       // メソッド名
        } IdentType_e;
        extern const char* VAR_IDENT_STR[5];    // IdentType_e → const char*
        typedef struct {
            IdentType_e   type;
            SymbolID_t  symbol;
            union {
                ClassInfo*      classInfo;
                ConstVar_t*     constInfo;
                MethodCallable_e methodInfo;
            };
        } Var_t;


        class Package;
        class Context;
        class Bytecode;

        // クラスの情報を保持するクラス
        class ClassInfo {

            friend class Context; // バイトコード、デバッグ情報出力のため
            friend class ::Test_hyCClassInfo;

        public:
            static void* operator new(size_t size);
            static void operator delete(void* p);

            ClassInfo(SymbolID_t classSym, Package* package);
            ~ClassInfo();
            
            SymbolID_t classSymbol(void) { return m_classSym; }
            const char* className(void) const;
            void addSuperClass(ClassInfo* superClassInfo);
            bool isPrimitive(void);
            hyu32 fieldSize(void);

            void addMethod(SymbolID_t methodSym, Bytecode* pBytecode);
            void addClosure(Bytecode* pBytecode);
            bool isMembVar(SymbolID_t varSym);
            void addMembVar(SymbolID_t varSym);
            bool isClassVar(SymbolID_t varSym);
            void addClassVar(SymbolID_t varSym);
            ConstVar_t* getLocalConstVar(SymbolID_t varSym) { //自クラスのみ
                return m_constVars.find(varSym); }
            ConstVar_t* getMyConstVar(SymbolID_t varSym); //super含む
            ConstVar_t* getConstVar(SymbolID_t varSym); //using,package含む
            ConstVar_t* createConstVar(SymbolID_t varSym);
            Var_t getScopedVal(TArray<SymbolID_t>& scope, SymbolID_t varSym);
            void getMyIdent(SymbolID_t sym, Var_t* pVar);

            hyu32 nameTableEntry(Substr str);
            void addCppClass(Substr cppClassName);
            bool addCppClass(const char* cppClassName);
            void addCppInclude(Substr fileName);
            void addFfi(SymbolID_t ffiSym, FfiSigDecl_t sigDecl);

            // classSym がクラスとしてここから見えるか
            ClassInfo* searchClassInfo(SymbolID_t classSym);
            // varSym が定数としてここから見えるか
            ConstVar_t* searchConstVar(SymbolID_t varSym);
            // methodSym がメソッド名としてここから見えるか
            MethodCallable_e searchMethod(SymbolID_t methodSym);
            
            // methodSym がメソッド名としてここかsuperにあるか
            MethodCallable_e searchMyMethod(SymbolID_t methodSym);


            // 継承を含めた m_cppClass のチェックと構築
            bool checkCppClass(void);

            ClassInfo* addInnerClassCreate(SymbolID_t classSym);
            ClassInfo* getInnerClassInfo(SymbolID_t classSym);

            ClassInfo*  searchRelativeClassInfo(SymbolID_t classSym);
            ClassInfo*  getMyClassInfo(SymbolID_t classSym); // inner,super->inner
            Package*    getPackage(void) { return m_package; }
            ClassInfo*  searchTopClassInfo(SymbolID_t classSym);
            void        addUsing(Scope_t* scope);
            ClassInfo*  getScopeClassInfo(Scope_t* scope, bool ignoreUsing = false);
            ClassInfo*  searchUsingClassInfo(SymbolID_t classSym);
            bool        checkScope_inSuper(Scope_t* scope);
            bool        isSuper(ClassInfo* cinfo);

            hyu16 numMethod(void) { return m_methods.size(); }
            hyu16 numClosure(void) { return m_closures.size(); }
            hyu16 numMembVar(void) { return m_membVars.size(); }
            hyu16 numClassVar(void) { return m_classVars.size(); }
            hyu16 numSuper(void) { return m_superClasses.size(); }
            hyu16 numConstVar(void) { return m_constVars.size(); }
            hyu16 numDefaultVal(void) { return m_numDefaultVal; }
            hyu16 defaultValAdded(void) { HMD_ASSERT(m_numDefaultVal <= Signature::MAXNUM_DEFAULT_VAL); return m_numDefaultVal++; }

            Bytecode* getMethodBytecode(hyu16 idx) { return m_methods.values()[idx].pBytecode; }
            Bytecode* getClosureBytecode(hyu16 idx) { return m_closures[idx]; }

            void generateCppProg(void);
            void writeCHeader(void);
            bool haveFfi(void); // innerも含めてffiを持っているか
            void fwriteIncludeCHeader(FILE* fp);
            void fwriteClassFfiTable(FILE* fp);
            void writeFfiCpp(void);
            
            void writeClassVarSyms(TArray<hyu8>* out);
            void writeMembVarSyms(TArray<hyu8>* out);
            void writeConstVarSyms(TArray<hyu8>* out);
            void writeSuperLinks(TArray<hyu8>* out);
            void writeUsingPaths(TArray<hyu8>* out);

            void serialize(TArray<hyu8>* out);
            const hyu8* deserialize(const hyu8* inp);

            // innerクラスも再帰的に、クラスが存在するかチェック
            bool checkClass(SymbolID_t classSym);
            // innerクラスも再帰的に、メソッドが存在するかチェック(シグネチャも合わせてチェック)
            bool checkMethod(SymbolID_t methodSym, const Signature::Sig_t* rightSig);
            // innerクラスも再帰的に、メソッドが存在するかチェック(メソッド名のみチェク)
            bool checkMethodName(SymbolID_t methodSym);


#ifdef EXPERIMENT_SYMBOL_DUMP
            void printSyms(void);
#endif
        protected:
            SymbolID_t  m_classSym;
            Package*    m_package;
            ClassInfo*  m_outerInfo;
            TArray<ClassInfo*>  m_superClasses;
            BMap<SymbolID_t, ClassInfo*>  m_innerClasses;
            typedef struct {
                Bytecode*       pBytecode;
                hyu16             signatureID;
            } m_SigCode_t;
            BMap<SymbolID_t, m_SigCode_t> m_methods;
            TArray<Bytecode*> m_closures;
            TArray<SymbolID_t> m_membVars;
            TArray<SymbolID_t>  m_classVars;
            BMap<SymbolID_t, ConstVar_t> m_constVars;
            TArray<Scope_t*>    m_usingList;
            hyu16                 m_numDefaultVal;
            char*       m_classNameOnCpp;  // C++上で使われる名前

            StrTable    m_nameTable;


            // c++ 命令で指定したC++クラス名
            const char* m_cppClass;      // serializeする
            // 継承した親クラスにc++命令があった時のクラス情報
            ClassInfo*  m_superClassInfo_cppClass; // serializeする

            // addUsing()でm_usingListと同時に作られる : serializeしない
            TArray<ClassInfo*>  m_usingClassInfos;

            // 以下ffi情報
            BMap<SymbolID_t, FfiSigDecl_t> m_ffi; //一部serialize
            // 以下serializeしないffi情報
            TArray<Substr>      m_cppIncludeFiles;
            TArray<const char*> m_userDefinedFfi;
            TArray<const char*> m_userDefinedDeepFfi;
            bool m_bUserDefinedGCMarkFunc;


        protected:
            static void m_funcName(SymbolID_t sym, char* buf, hyu32 bufSize);
            void m_funcName_addArity(char* funcName, hyu16 sigID);
            bool m_hsc_FFI_check(const char* buf);

            void m_fwriteCHeaderSub(FILE* fp);
            void m_fwriteNamespace(FILE* fp);
            void m_fwriteNamespaceEnd(FILE* fp);
            void m_decl_pop(FILE* fp, const char* varName, const char* type);
            void m_checkUserDefineFfi(void);
            void m_fwriteFfiFunc(FILE* fp);
            bool m_needGenerate_cppSize(void);
            bool m_addUsingClassInfo(ClassInfo* cinfo);
            const char* classNameOnCpp(void);
            void fwriteFfiClassNameOnCpp(FILE* fp);
            void fwriteFfiScope(FILE* fp);
            bool sprintGenFName(char* buf, size_t bufSize, const char* pPath, const char* pref, const char* postf);
            void m_strscatGenFName(char* buf, size_t bufSize);
            static void m_serializeClassID(TArray<hyu8>* out, ClassInfo* ci);
            static ClassInfo* m_deserializeClassID(const hyu8** pinp);

        };

    }
}

#endif /* m_HYCCLASSINFO_H_ */
