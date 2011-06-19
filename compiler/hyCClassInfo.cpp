/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCClassInfo.h"
#include "hyCSymbolTable.h"
#include "hyCFileOut.h"
#include "hyCPackage.h"
#include "hyPacking.h"
#include "hyCBytecode.h"
#include "hyCSignature.h"
#include "hpInputBuffer.h"
#include "hyCCompileError.h"
#include "hyCIdent.h"


namespace Hayat {
    namespace Compiler {
        extern SymbolTable gSymTable;
        extern SymbolID_t HyCSymS_cppSize;
        extern SymbolID_t HyCSym_Object;
        extern SymbolID_t HyCSym_nil;
        extern FfiTypeMgr gFfiTypeMgr;

        const char* VAR_IDENT_STR[5] = {    // IdentType_e → const char*
            "unknown",
            "local variable",
            "class",
            "constant",
            "method"
        };
    }
}
using namespace Hayat::Common;
using namespace Hayat::Compiler;
using namespace Hayat::Parser;



extern const char* gFfiDir;
extern const char* gFfiOutDir;





// for TArray<const char*>::search()
static bool compStr(const char* const& p, const char* const& q) { return HMD_STRCMP(p,q) == 0; }


void* ClassInfo::operator new(size_t size)
{
    HMD_DEBUG_ASSERT_EQUAL(sizeof(ClassInfo), size);
    return (void*) gMemPool->alloc(sizeof(ClassInfo));
}

void ClassInfo::operator delete(void* p)
{
    gMemPool->free(p);
}

ClassInfo::ClassInfo(SymbolID_t classSym, Package* package)
    : m_classSym(classSym), m_package(package), m_outerInfo(NULL),
      m_superClasses(0), m_innerClasses(0),
      m_membVars(0), m_classVars(0), m_constVars(0), m_numDefaultVal(0),
      m_classNameOnCpp(NULL), m_cppClass(NULL),
      m_usingClassInfos(0), m_ffi(0), m_cppIncludeFiles(0),
      m_userDefinedFfi(0), m_userDefinedDeepFfi(0),
      m_bUserDefinedGCMarkFunc(false)
{
    HMD_DEBUG_ASSERT(package != NULL);
    m_nameTable.initialize();
}

ClassInfo::~ClassInfo()
{
    hyu32 i, n;
    n = m_userDefinedDeepFfi.size();
    for (i = 0; i < n; i++)
        gMemPool->free((void*) m_userDefinedDeepFfi.nth(i));
    n = m_userDefinedFfi.size();
    for (i = 0; i < n; i++)
        gMemPool->free((void*) m_userDefinedFfi.nth(i));
    n = m_usingList.size();
    for (i = 0; i < n; i++)
        delete m_usingList[i];
    TArray<ClassInfo*>& v = m_innerClasses.values();
    n = v.size();
    for (i = 0; i < n; i++)
        delete v[i];
    if (m_cppClass != NULL)
        gMemPool->free((void*)m_cppClass);
    TArray<FfiSigDecl_t>& ssdArr = m_ffi.values();
    n = ssdArr.size();
    for (i = 0 ; i < n; i++)
        delete ssdArr[i].argDecls;
    if (m_classNameOnCpp != NULL)
        gMemPool->free(m_classNameOnCpp);
}

const char* ClassInfo::className(void) const
{
    return gSymTable.id2str(m_classSym);
}

const char* ClassInfo::classNameOnCpp(void)
{
    if (m_classNameOnCpp == NULL)
        m_classNameOnCpp = Ident::encode(className(), "HSc");
    return m_classNameOnCpp;
}

void ClassInfo::fwriteFfiClassNameOnCpp(FILE* fp)
{
    if (m_superClassInfo_cppClass != NULL) {
        m_superClassInfo_cppClass->fwriteFfiScope(fp);
    } else {
        fprintf(fp, classNameOnCpp());
    }
}

void ClassInfo::fwriteFfiScope(FILE* fp)
{
    if (m_outerInfo != NULL) {
        m_outerInfo->fwriteFfiScope(fp);
    } else {
        fprintf(fp, "Hayat::Ffi");
    }
    fprintf(fp, "::%s", classNameOnCpp());
}

bool ClassInfo::sprintGenFName(char* buf, size_t bufSize, const char* pPath, const char* pref, const char* postf)
{
    if (pPath == NULL) {
        buf[0] = '\0';
    } else {
        HMD_STRNCPY(buf, pPath, bufSize);
    }
    if (pref != NULL)
        HMD_STRSCAT(buf, pref, bufSize);
    if (m_outerInfo == NULL) {
        // パッケージクラス
        HMD_STRSCAT(buf, classNameOnCpp(), bufSize);
    } else {
        m_strscatGenFName(buf, bufSize);
    }
    hyu32 len = HMD_STRLEN(buf);
    hyu32 plen = HMD_STRLEN(postf);
    if (len + plen >= bufSize)
        return false;
    HMD_STRNCPY(buf+len, postf, plen+1);
    //HMD_PRINTF("%s\n",buf);
    return true;
}
void ClassInfo::m_strscatGenFName(char* buf, size_t bufSize)
{
    if (m_outerInfo == NULL)
        return;                 // パッケージ名(stdlibなど)は書かない

    if (m_outerInfo->m_outerInfo != NULL) {
        m_outerInfo->m_strscatGenFName(buf, bufSize);
        HMD_STRSCAT(buf, HY_GEN_FNAME_DELIM, bufSize);
        HMD_STRSCAT(buf, className(), bufSize);
    } else {
        HMD_STRSCAT(buf, classNameOnCpp(), bufSize);
    }
}

hyu32 ClassInfo::fieldSize(void)
{
    return numMembVar() * 8;
}

void ClassInfo::addMethod(SymbolID_t methodSym, Bytecode* pBytecode)
{
    TArray<SymbolID_t>& msyms = m_methods.keys();
    int idxMin, idxMax;
    hyu16 mySig = pBytecode->getSignatureID();
    m_SigCode_t* ps;
    if (binarySearchRange<SymbolID_t>(msyms.top(), msyms.size(), methodSym, &idxMin, &idxMax)) {
        TArray<m_SigCode_t>& sigCodes = m_methods.values();
        for (int i = idxMin; i <= idxMax; ++i) {
            if (mySig == sigCodes[i].signatureID) {
                // same method symbol, same signature
                compileError(M_M("multiple definition of method '%s'"), gSymTable.id2str(methodSym));
            }
        }
        // method overload
        msyms.insert(idxMax + 1) = methodSym;
        ps = &(sigCodes.insert(idxMax + 1));
    } else {
        ps = &(m_methods[methodSym]);
    }
    ps->pBytecode = pBytecode;
    ps->signatureID = mySig;
}

void ClassInfo::addClosure(Bytecode* pBytecode)
{
    m_closures.add(pBytecode);
}


bool ClassInfo::isMembVar(SymbolID_t varSym)
{
    return m_membVars.isInclude(varSym);
}

void ClassInfo::addMembVar(SymbolID_t varSym)
{
    if (! isMembVar(varSym))
        m_membVars.add(varSym);
}

bool ClassInfo::isClassVar(SymbolID_t varSym)
{
    if (m_classVars.isInclude(varSym))
        return true;
    for (hys32 i = (hys32)m_superClasses.size() - 1; i >= 0; --i) {
        if (m_superClasses[i]->isClassVar(varSym))
            return true;
    }
    return false;
}

void ClassInfo::addClassVar(SymbolID_t varSym)
{
    if (! isClassVar(varSym))
        m_classVars.add(varSym);
}

ConstVar_t* ClassInfo::getMyConstVar(SymbolID_t varSym)
{
    ConstVar_t* pv = getLocalConstVar(varSym);
    if (pv != NULL)
        return pv;
    for (hys32 i = (hys32)m_superClasses.size() - 1; i >= 0; --i) {
        pv = m_superClasses[i]->getMyConstVar(varSym);
        if (pv != NULL)
            return pv;
    }
    return NULL;
}

ConstVar_t* ClassInfo::getConstVar(SymbolID_t varSym)
{
    ConstVar_t* pv = getMyConstVar(varSym);
    if (pv != NULL)
        return pv;
    for (hys32 i = (hys32)m_usingClassInfos.size() - 1; i >= 0; --i) {
        pv = m_usingClassInfos[i]->getConstVar(varSym);
        if (pv != NULL)
            return pv;
    }
    ClassInfo* q = m_package->packageClass();
    if (q != this) {
        pv = q->getMyConstVar(varSym);
        if (pv != NULL)
            return pv;
    }
    return NULL;
}

Var_t ClassInfo::getScopedVal(TArray<SymbolID_t>& scope, SymbolID_t varSym)
{
    ClassInfo* cinf = NULL;
    hyu32 n = scope.size();
    if (scope[0] == HyCSym_nil) {
        // ::SCOPE:: ... ::VAR
        cinf = m_package->packageClass();
    } else {
        // SCOPE:: ... ::VAR
        cinf = searchClassInfo(scope[0]);
    }
    for (hyu32 i = 1; i < n; ++i) {
        if (cinf == NULL)
            break;
        cinf = cinf->getMyClassInfo(scope[i]);
    }
    Var_t var;
    if (cinf != NULL) {
        cinf->getMyIdent(varSym, &var);
    } else {
        var.type = IT_NONE;
    }
    return var;
}



ConstVar_t* ClassInfo::createConstVar(SymbolID_t varSym)
{
    if (m_constVars.find(varSym))
        return NULL;    // 既にあるので作成できない 
    return &m_constVars[varSym];
}

void ClassInfo::getMyIdent(SymbolID_t sym, Var_t* pVar)
{
    pVar->symbol = sym;

    ClassInfo* cinf = getMyClassInfo(sym);
    if (cinf != NULL) {
        pVar->type = IT_CLASS;
        pVar->classInfo = cinf;
        return;
    }

    ConstVar_t* pv = getMyConstVar(sym);
    if (pv != NULL) {
        pVar->type = IT_CONST;
        pVar->constInfo = pv;
        return;
    }

    MethodCallable_e mt = searchMyMethod(sym);
    if (mt != MT_NONE) {
        pVar->type = IT_METHOD;
        pVar->methodInfo = mt;
        return;
    }

    pVar->type = IT_NONE;
}


void ClassInfo::addSuperClass(ClassInfo* superClassInfo)
{
    m_superClasses.add(superClassInfo);
    // メンバ変数追加
    TArray<SymbolID_t>& superMembVars = superClassInfo->m_membVars;
    hys32 size = (hys32) superMembVars.size();
    for (hys32 i = 0; i < size; ++i)
        addMembVar(superMembVars[i]);
}

bool ClassInfo::isPrimitive(void)
{
	if (m_cppClass != NULL) return false;
    if (numMembVar() != 0) return false;
    return ! (m_ffi.keys().isInclude(HyCSymS_cppSize));
}

ClassInfo* ClassInfo::addInnerClassCreate(SymbolID_t classSym)
{
    ClassInfo* inf = new ClassInfo(classSym, m_package);
    m_package->addClassInfo(inf);
    m_innerClasses[classSym] = inf;
    inf->m_outerInfo = this;
    return inf;
}

ClassInfo* ClassInfo::getInnerClassInfo(SymbolID_t classSym)
{
    ClassInfo** p = m_innerClasses.find(classSym);
    if (p == NULL)
        return NULL;
    return *p;
}

ClassInfo* ClassInfo::searchRelativeClassInfo(SymbolID_t classSym)
{
    ClassInfo** p = m_innerClasses.find(classSym);
    if (p != NULL) return *p;
    for (hys32 i = (hys32)m_superClasses.size() - 1; i >= 0; --i) {
        ClassInfo* q = m_superClasses[i];
        if (q->classSymbol() == classSym)
            return q;
    }
    if (m_outerInfo != NULL)
        return m_outerInfo->getInnerClassInfo(classSym);
    return NULL;
}

ClassInfo* ClassInfo::getMyClassInfo(SymbolID_t classSym)
{
    // inner
    ClassInfo** p = m_innerClasses.find(classSym);
    if (p != NULL) return *p;
    // super->inner
    for (hys32 i = (hys32)m_superClasses.size() - 1; i >= 0; --i) {
        ClassInfo* q = m_superClasses[i]->getMyClassInfo(classSym);
        if (q != NULL) return q;
    }
    return NULL;
}

ClassInfo* ClassInfo::searchTopClassInfo(SymbolID_t classSym)
{
    return m_package->packageClass()->getMyClassInfo(classSym);
}

// innerクラスにあるか
// superクラスにあるか
// superクラスから見えるか
// usingパスから見えるか
// Outerクラスから見えるか
ClassInfo* ClassInfo::searchClassInfo(SymbolID_t classSym)
{
    ClassInfo** p = m_innerClasses.find(classSym);
    if (p != NULL) return *p;
    for (hys32 i = (hys32)m_superClasses.size() - 1; i >= 0; --i) {
        ClassInfo* q = m_superClasses[i];
        if (q->classSymbol() == classSym)
            return q;
        ClassInfo* r = q->searchClassInfo(classSym);
        if (r != NULL) return r;
    }
    for (hys32 i = (hys32)m_usingClassInfos.size() - 1; i >= 0; --i) {
        ClassInfo* q = m_usingClassInfos[i]->searchClassInfo(classSym);
        if (q != NULL) return q;
    }
    if (m_outerInfo != NULL)
        return m_outerInfo->searchClassInfo(classSym);
    return NULL;
}

// 自クラスにあるか
// superクラスから見えるか
// usingパスから見えるか
// Outerクラスから見えるか
ConstVar_t* ClassInfo::searchConstVar(SymbolID_t varSym)
{
    ConstVar_t* v = m_constVars.find(varSym);
    if (v != NULL) return v;
    for (hys32 i = (hys32)m_superClasses.size() - 1; i >= 0; --i) {
        v = m_superClasses[i]->searchConstVar(varSym);
        if (v != NULL) return v;
    }
    for (hys32 i = (hys32)m_usingClassInfos.size() - 1; i >= 0; --i) {
        v = m_usingClassInfos[i]->searchConstVar(varSym);
        if (v != NULL) return v;
    }
    if (m_outerInfo != NULL)
        return m_outerInfo->searchConstVar(varSym);
    return NULL;

}

// 自クラスにあるか
// superクラスにあるか
// Objectクラスにあるか
// usingパスから見えるか		(クラスメソッド呼出しとなる)
// Outerクラスから見えるか		(クラスメソッド呼出しとなる)
MethodCallable_e ClassInfo::searchMethod(SymbolID_t methodSym)
{
    if (m_methods.find(methodSym) != NULL)
        return MT_OK;
    if (m_ffi.find(methodSym) != NULL)
        return MT_OK;
    if (m_classSym == HyCSym_Object)
        return MT_NONE;         // 自分がObjectクラスなので終わり
    for (hys32 i = (hys32)m_superClasses.size() - 1; i >= 0; --i) {
        MethodCallable_e mt = m_superClasses[i]->searchMethod(methodSym);
        if (mt != MT_NONE) return mt;
    }
    ClassInfo* objClass = searchTopClassInfo(HyCSym_Object);
    if (objClass != NULL) {
        MethodCallable_e mt = objClass->searchMethod(methodSym);
        if (mt != MT_NONE) return mt;
    }
    for (hys32 i = (hys32)m_usingClassInfos.size() - 1; i >= 0; --i) {
        MethodCallable_e mt = m_usingClassInfos[i]->searchMethod(methodSym);
        if (mt != MT_NONE) return MT_CLASS;
    }
    if (m_outerInfo != NULL)
        if (m_outerInfo->searchMethod(methodSym) != MT_NONE)
            return MT_CLASS;
    return MT_NONE;
}

MethodCallable_e ClassInfo::searchMyMethod(SymbolID_t methodSym)
{
    if (m_methods.find(methodSym) != NULL)
        return MT_OK;
    if (m_ffi.find(methodSym) != NULL)
        return MT_OK;
    if (m_classSym == HyCSym_Object)
        return MT_NONE;         // 自分がObjectクラスなので終わり
    for (hys32 i = (hys32)m_superClasses.size() - 1; i >= 0; --i) {
        MethodCallable_e mt = m_superClasses[i]->searchMyMethod(methodSym);
        if (mt != MT_NONE) return mt;
    }
    ClassInfo* objClass = searchTopClassInfo(HyCSym_Object);
    if (objClass != NULL) {
        MethodCallable_e mt = objClass->searchMyMethod(methodSym);
        if (mt != MT_NONE) return mt;
    }
    return MT_NONE;
}


void ClassInfo::addUsing(Scope_t* scope)
{
    ClassInfo* ci = getScopeClassInfo(scope, true);
    if (m_addUsingClassInfo(ci))
        m_usingList.add(scope);
}

ClassInfo* ClassInfo::getScopeClassInfo(Scope_t* scope, bool ignoreUsing)
{
    hyu32 n = scope->size();
    hyu32 i = 0;
    ClassInfo* ci;
    SymbolID_t tsym = scope->nth(0);
    if (tsym == (SymbolID_t)0) {
        // 絶対
        HMD_DEBUG_ASSERT(n > 1);
        ci = searchTopClassInfo(scope->nth(1));
        i = 2;
    } else {
        // 相対
        ci = searchRelativeClassInfo(tsym);
        if (ci == NULL && !ignoreUsing)
            ci = searchUsingClassInfo(tsym);
        if (ci == NULL)
            ci = searchTopClassInfo(tsym);
        i = 1;
    }
    if (ci == NULL)
        return NULL;
    for ( ; i < n; i++) {
        ci = ci->getInnerClassInfo(scope->nth(i));
        if (ci == NULL)
            return NULL;
    }
    return ci;
}

bool ClassInfo::checkScope_inSuper(Scope_t* scope)
{
    if (scope->size() == 1 && scope->nth(0) == (SymbolID_t)0) // '::'
        return false;
    return isSuper(getScopeClassInfo(scope));
}

bool ClassInfo::isSuper(ClassInfo* cinfo)
{
    if (cinfo == NULL)
        return false;
    for (hys32 i = (hys32)m_superClasses.size() - 1; i >= 0; --i) {
        ClassInfo* sup = m_superClasses[i];
        if (sup == cinfo)
            return true;
        if (sup->isSuper(cinfo))
            return true;
    }
    return false;
}

ClassInfo* ClassInfo::searchUsingClassInfo(SymbolID_t classSym)
{
    for (hys32 i = (hys32)m_usingClassInfos.size() - 1; i >= 0; --i) {
        ClassInfo* p = m_usingClassInfos[i]->getInnerClassInfo(classSym);
        if (p != NULL)
            return p;
    }
    return NULL;
}

bool ClassInfo::m_addUsingClassInfo(ClassInfo* cinfo)
{
    if (! m_usingClassInfos.isInclude(cinfo)) {
        m_usingClassInfos.add(cinfo);
        return true;
    }
    return false;
}

hyu32 ClassInfo::nameTableEntry(Substr str)
{
    hyu32 len = str.len() + 1;
    char* buf = (char*)HMD_ALLOCA(len);
    gpInp->copyStr(buf, len, str);
    return m_nameTable.getOffs(buf);
}

void ClassInfo::addCppClass(Substr cppClassName)
{
    if (m_cppClass == NULL) {
        hyu32 len = cppClassName.len();
        m_cppClass = gMemPool->allocT<char>(len + 1);
        gpInp->copyStr((char*)m_cppClass, len, cppClassName);
    } else
        compileError(M_M("only 1 C++ class allowed in a Hayat class : including inherited class"));
    gFfiTypeMgr.createInterface(className(), m_cppClass);
}

bool ClassInfo::addCppClass(const char* cppClassName)
{
    if (cppClassName == NULL)
        return true;
    if (m_cppClass == NULL) {
        hyu32 len = HMD_STRLEN(cppClassName);
        m_cppClass = gMemPool->allocT<char>(len + 1);
        HMD_STRNCPY((char*)m_cppClass, cppClassName, len + 1);
    } else if (HMD_STRCMP(m_cppClass, cppClassName) != 0) {
        // 多重継承で元が同じC++クラスではない
        return false;
    }
    gFfiTypeMgr.createInterface(className(), m_cppClass);
    return true;
}

bool ClassInfo::checkCppClass(void)
{
    if (m_cppClass == NULL) {
        // C++クラス指定無し → スーパークラスをチェックする
        hyu32 size = m_superClasses.size();
        for (hyu32 i = 0; i < size; ++i) {
            ClassInfo* superInfo = m_superClasses[i];
            if (superInfo->m_cppClass != NULL) {
                if (! addCppClass(superInfo->m_cppClass))
                    return false;
                while (superInfo->m_superClassInfo_cppClass != NULL)
                    superInfo = superInfo->m_superClassInfo_cppClass;
                m_superClassInfo_cppClass = superInfo;
            }
        }
    } else {
        // C++クラスが明示的に指定されている
        // todo: C++クラスの継承関係チェック
    }
    return true;
}

void ClassInfo::addCppInclude(Substr fileName)
{
    m_cppIncludeFiles.add(fileName);
}


static int cmpssdsid(const FfiSigDecl_t& sd, const hyu16& id) {
    return sd.signatureID - id;
}
void ClassInfo::addFfi(SymbolID_t ffiSym, FfiSigDecl_t ffiSigDecl)
{
    TArray<SymbolID_t>& ssyms = m_ffi.keys();
    int idxMin, idxMax, idx;
    if (binarySearchRange(ssyms.top(), ssyms.size(), ffiSym, &idxMin, &idxMax)) {
        hyu16 mySig = ffiSigDecl.signatureID;
        TArray<FfiSigDecl_t>& ssds = m_ffi.values();
        if (binarySearchFn<const FfiSigDecl_t,const hyu16>(ssds.nthAddr(idxMin), idxMax-idxMin+1, cmpssdsid, mySig, &idx)) {
            compileError(M_M("multiple definition of ffi '%s'"), gSymTable.id2str(ffiSym));
        }
        idx += idxMin;
        ssyms.insert(idx) = ffiSym;
        ssds.insert(idx) = ffiSigDecl;
    } else {
        m_ffi[ffiSym] = ffiSigDecl;
    }
}

void ClassInfo::generateCppProg(void)
{
    if (m_ffi.size() < 1 && ! m_needGenerate_cppSize())
        return;
    hmd_mkdir(gFfiOutDir, 0775);
    m_checkUserDefineFfi();
    writeCHeader();
    writeFfiCpp();
}

void ClassInfo::writeCHeader(void)
{
    char path[1024];
    if (! sprintGenFName(path, 1024, gFfiOutDir, "/", ".h"))
        compileError(M_M("path name too long to write FFI header of class %s"), classNameOnCpp());

    DiffWrite dw;
    FILE* fp = dw.open(path);
    if (fp == NULL)
        compileError(M_M("can't write to file %s"), path);
    writeGenHead(fp);
    char upname[128];
    HMD_ASSERT(HMD_STRLEN(className()) < 128);
    const char* p = className(); char* q = upname;
    while (*p) {
        hyu8 c = *p++;
        if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
        else if (c == '.') c = '_';
        *q++ = c;
    }
    *q = '\0';
    fprintf(fp, "#ifndef m_HSCA_%s_H_\n", upname);
    fprintf(fp, "#define m_HSCA_%s_H_\n", upname);
    fprintf(fp, "#include \"hyFfiDecl.h\"\n");
    hyu32 n = m_cppIncludeFiles.size();
    for (hyu32 i = 0; i < n; i++) {
        fprintf(fp, "#include \"");
        gpInp->fprintStr(fp, m_cppIncludeFiles.nth(i));
        fprintf(fp, "\"\n");
    }
    fprintf(fp, "#undef FFI_DEFINITION_START\n");
    fprintf(fp, "#define FFI_DEFINITION_START namespace Hayat { namespace Ffi");
    m_fwriteNamespace(fp);
    fprintf(fp,
            "\n"
            "#undef FFI_DEFINITION_END\n");
    fprintf(fp, "#define FFI_DEFINITION_END ");
    m_fwriteNamespaceEnd(fp);
    fprintf(fp,
            "}\n"
            "\n"
            "FFI_DEFINITION_START {\n");

    if (m_cppClass != NULL) {
        fprintf(fp, "        typedef %s HyCppClass_t;\n", m_cppClass);
    }
	fprintf(fp, "        static const size_t HY__FIELD__SIZE = %d;\n", fieldSize());


    m_fwriteCHeaderSub(fp);

    if (m_needGenerate_cppSize())
        fprintf(fp, "        extern void m_HSfx_2a63707053697a65(Context* context, int numArgs);\n");

    if (m_bUserDefinedGCMarkFunc) {
        fprintf(fp, "        extern void HSfg_GCMarkFunc(Object*);\n");
    }
    fprintf(fp,
            "} FFI_DEFINITION_END\n"
            "#endif\n");
    dw.close();
}
void ClassInfo::m_fwriteNamespace(FILE* fp)
{
    if (m_outerInfo != NULL)
        m_outerInfo->m_fwriteNamespace(fp);
    fprintf(fp, " { namespace %s", classNameOnCpp());
}
void ClassInfo::m_fwriteNamespaceEnd(FILE* fp)
{
    for (ClassInfo* p = m_outerInfo; p != NULL; p = p->m_outerInfo)
        fprintf(fp, "}");
    fprintf(fp, "}");
}



bool ClassInfo::m_needGenerate_cppSize(void)
{
    if (m_ffi.keys().isInclude(HyCSymS_cppSize))
        return false;   // ユーザが定義したのでgenerateしなくて良い
    if (m_cppClass == NULL)
        return false;   // C++のクラス定義が無い
    if (m_superClassInfo_cppClass != NULL)
        return false;           // C++クラス定義は親クラスのものだった
    return true;
}

void ClassInfo::m_funcName(SymbolID_t sym, char* buf, hyu32 bufSize)
{
    HMD_ASSERT(bufSize > 6);
    const char* m = gSymTable.id2str(sym);
    if (! Ident::encode(m, buf+3, bufSize-3)) {
        compileError(M_M("function name '%s' too long"), m);
    }
    *buf = 'H';
    *++buf = 'S';
    *++buf = 'f';
}

// funcNameのバッファ容量に余裕がなければならない
void ClassInfo::m_funcName_addArity(char* funcName, hyu16 sigID)
{
    int arity = sigID;
    if (sigID > Signature::ID_MULTI) {
        const Signature::Sig_t* sigBytes = m_package->getSignatureBytes(sigID);
        arity = Signature::arity(sigBytes);
    }
    char* p = funcName;
    while (*p != '\0')
        ++p;
    HMD_SNPRINTF(p, 5, "_%d", arity);
}    

static const char* strstr2(const char* haystack, const char* needle)
{
    const char* p = strstr(haystack, needle);
    if (p == NULL) return NULL;
    return p + HMD_STRLEN(needle);
}
static void m_hsc_skipWord(const char*& p)
{
    char c = *p;
    while ((c == '_') ||
           ((c >= 'A')&&(c <= 'Z')) ||
           ((c >= 'a')&&(c <= 'z')) ||
           ((c >= '0')&&(c <= '9')) )
        c = *++p;
}
static void m_hsc_skipSpace(const char*& p)
{
    char c = *p;
    while ((c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'))
        c = *++p;
}
static bool m_hsc_skipChar(const char*& p, char c)
{
    if (*p == c) {
        ++p;
        return true;
    }
    return false;
}
static void m_hsc_skipToChar(const char*& p, char c)
{
    while (*p != '\0' && *p != c)
        ++p;
}
static void m_hsc_skipToChar2(const char*& p, char c1, char c2)
{
    while (*p != '\0' && *p != c1 && *p != c2)
        ++p;
}
bool ClassInfo::m_hsc_FFI_check(const char* buf)
{
    static const char* CHECK_STRS[] = {
        "DEEP_FFI_FUNC", 
        "FFI_FUNC",
        "FFI_MEMBER_GETTER",
        "FFI_MEMBER_SETTER",
        "FFI_S_MEMBER_GETTER",
        "FFI_S_MEMBER_SETTER",
        "FFI_GCMARK_FUNC"
    };
    typedef enum {
        DEEP_FFI,
        FFI_FUNC,
        FFI_MEMBER_GETTER,
        FFI_MEMBER_SETTER,
        FFI_S_MEMBER_GETTER,
        FFI_S_MEMBER_SETTER,
        GCMARK_FUNC,
        NUM_STYPE
    } Stype_e;

    // 定義マクロ取得
    int i;
    const char* p;
    for (i = DEEP_FFI; i < NUM_STYPE; ++i) {
        p = strstr2(buf, CHECK_STRS[i]);
        if (p != NULL) break;
    }
    if (i >= NUM_STYPE)
        return false;
    Stype_e stype = (Stype_e)i;
    
    // メソッド名取得
    const char* op = p;
    m_hsc_skipWord(p);
    const char* op_e = p;
    if ((op < op_e) && (*op == '_')) ++op;
    m_hsc_skipSpace(p);
    if (!m_hsc_skipChar(p, '('))
        return false;
    m_hsc_skipSpace(p);
    const char* nam = p;
    m_hsc_skipWord(p);
    const char* nam_e = p;
    m_hsc_skipSpace(p);
    if (nam == nam_e)
        return false;
    if (*p != ')' && *p != ',')
        return false;


    // FFI_FUNC の場合は引数の数を数える
    int numArgs = 0;
    if (stype == FFI_FUNC) {
        m_hsc_skipToChar(p, ')');
        if (*p++ != ')')
            return false;
        m_hsc_skipToChar(p, '(');
        if (*p++ != '(')
            return false;

        for(numArgs = 0; ; ++numArgs) {
            m_hsc_skipToChar2(p, ',', ')');
            if (*p == '\0')
                return false;
            if (*p++ == ')')
                break;
        }
        HMD_DEBUG_ASSERT(numArgs <= 999);
    }


    hyu32 namLen = nam_e - nam;
    char* namBuf = (char*)HMD_ALLOCA(namLen + 2);
    HMD_STRNCPY(namBuf, nam, namLen + 1);
    namBuf[namLen] = '\0';
    if (stype == FFI_FUNC || stype == DEEP_FFI) {
        if (op_e - op == 1) {
            if (*op == 'Q' || *op == 'E' || *op == 'S') {
                if (*op == 'Q')
                    namBuf[namLen] = '?';
                else if (*op == 'E')
                    namBuf[namLen] = '!';
                else {
                    namBuf[namLen] = '=';
                    numArgs = 1;
                }
                namBuf[++namLen] = '\0';
            }
        }
    } else if (stype == FFI_MEMBER_SETTER || stype == FFI_S_MEMBER_SETTER) {
        namBuf[namLen] = '=';
        namBuf[++namLen] = '\0';
        numArgs = 1;
    }

    char* fn;
    if ((stype == FFI_FUNC || stype == DEEP_FFI) && (op_e - op == 1) && (*op == 'X')) {
        fn = gMemPool->allocT<char>(namLen + 16);
        HMD_STRNCPY(fn, "HSfx_", 6);
        HMD_STRNCPY(fn+5, namBuf, namLen + 1);
    } else {
        fn = Ident::encode(namBuf, "HSf", 8);
    }

    if (stype == DEEP_FFI) {
        m_userDefinedDeepFfi.add(fn);
    } else {
        if (stype != GCMARK_FUNC) {
            char* q = fn + HMD_STRLEN(fn);
            HMD_SNPRINTF(q, 5, "_%d", numArgs);
        }
        m_userDefinedFfi.add(fn);
    }
    return true;
}
void ClassInfo::m_checkUserDefineFfi(void)
{
    char path[1024];
    if (! sprintGenFName(path, 1024, gFfiDir, "/", ".cpp"))
        compileError(M_M("path name too long to write FFI cpp file of class %s"), classNameOnCpp());

    FILE* fp = hmd_fopen(path, "r");
    if (fp == NULL) {
        return;
    }
    
    char buf[256];
    while (fgets(buf, 256, fp) != NULL) {
        if (m_hsc_FFI_check(buf))
            continue;
        if (strstr(buf, "FFI_GCMARK_FUNC") != NULL)
            m_bUserDefinedGCMarkFunc = true;
    }
    fclose(fp);
}

void ClassInfo::m_fwriteCHeaderSub(FILE* fp)
{
    TArray<SymbolID_t>& ffiSyms = m_ffi.keys();
    TArray<FfiSigDecl_t>& ffiSDs = m_ffi.values();
    hyu32 n = ffiSyms.size();
    char fname[248];
    char fname_a[256];
    for (hyu32 i = 0; i < n; i++) {
        SymbolID_t sym = ffiSyms[i];
        FfiSigDecl_t& decl = ffiSDs[i];
        m_funcName(sym, fname, 248);
        hyu32 idx;
        if (m_userDefinedDeepFfi.search(fname, compStr, idx)) {
            fprintf(fp, "        extern void m_%s(Context* context, int numArgs);\n", fname);
            continue;
        }
        HMD_STRNCPY(fname_a, fname, 248);
        m_funcName_addArity(fname_a, decl.signatureID);
        fprintf(fp, "        extern void m_%s(Context* context, int numArgs);\n", fname_a);
        if (m_userDefinedFfi.search(fname_a, compStr, idx)) {

            const FfiType& retInterface = gFfiTypeMgr.get(m_nameTable.getStr(decl.retType));
            const char* typeName = retInterface.type() ? retInterface.type() : "void";
            fprintf(fp, "        extern %s %s(Value selfVal", typeName, fname);
            hyu32 m = decl.argDecls->size();
            for (hyu32 j = 0; j < m; j++) {
                fprintf(fp, ", ");
                ArgD_t& t = decl.argDecls->nth(j);
                const FfiType& interface = gFfiTypeMgr.get(m_nameTable.getStr(t.typeName));
                if (interface.type() == NULL)
                    fprintf(fp, "Value ");
                else
                    fprintf(fp, "%s %s", interface.type(), m_nameTable.getStr(t.argName));
            }
            fprintf(fp, ");\n");
        }
    }
}



bool ClassInfo::haveFfi(void)
{
    if (m_ffi.size() > 0 || m_needGenerate_cppSize() || (m_superClassInfo_cppClass != NULL))
        return true;
    hyu32 numInnerClass = m_innerClasses.size();
    for (hyu32 i = 0; i < numInnerClass; ++i) {
        if (m_innerClasses.values()[i]->haveFfi())
            return true;
    }
    return false;
}

void ClassInfo::fwriteIncludeCHeader(FILE* fp)
{
    hyu32 numInnerClass = m_innerClasses.size();
    for (hyu32 i = 0; i < numInnerClass; ++i) {
        m_innerClasses.values()[i]->fwriteIncludeCHeader(fp);
    }
    if (m_ffi.size() < 1 && ! m_needGenerate_cppSize())
        return;

    char path[1024];
    if (! sprintGenFName(path, 1024, NULL, NULL, ".h"))
        compileError(M_M("path name too long to write FFI header of class %s"), classNameOnCpp());
    fprintf(fp, "#include \"%s\"\n", path);
}

void ClassInfo::fwriteClassFfiTable(FILE* fp)
{
    HMD_DEBUG_ASSERT(fp != NULL);
    bool bCppSize = m_needGenerate_cppSize();
    hyu32 m = m_ffi.size();
    hyu32 n = m + (bCppSize ? 1 : 0);
    hyu32 numInnerClass = m_innerClasses.size();

    fprintf(fp, "namespace %s {\n", classNameOnCpp());

    hyu32 numFfiTbl = 0;
    for (hyu32 i = 0; i < numInnerClass; ++i) {
        ClassInfo* inner = m_innerClasses.values()[i];
        if (inner->haveFfi()) {
            ++ numFfiTbl;
            inner->fwriteClassFfiTable(fp);
        }
    }

    if (n > 0) {
        TArray<SymbolID_t> scsyms(n);
        scsyms.add(m_ffi.keys());

        TArray<hyu16> sigIDs(n);
        TArray<hyu16> defValOffs(n);
        TArray<FfiSigDecl_t>& sigDecls = m_ffi.values();
        for (hyu32 i = 0; i < m; ++i) {
            sigIDs.add(sigDecls[i].signatureID);
            defValOffs.add(sigDecls[i].defValOffs);
        }
        if (bCppSize) {
            hyu32 i;
            for (i = 0; i < m; i++) {
                if (HyCSymS_cppSize < scsyms[i])
                    break;
            }
            scsyms.insert(i) = HyCSymS_cppSize;
            sigIDs.insert(i) = 0;
            defValOffs.insert(i) = 0;
        }


        fprintf(fp, "HClass::FfiMethodEntry_t METHODS[%d] = {\n", n);
        hyu32 idx;
        for (hyu32 i = 0; i < n; i++) {
            char funname[256];
            m_funcName(scsyms[i], funname, 248);
            if ((scsyms[i] != HyCSymS_cppSize) &&
                ! m_userDefinedDeepFfi.search(funname, compStr, idx))
            {
                m_funcName_addArity(funname, sigIDs[i]);
            }
            fprintf(fp, "  {%d, %d, %d, m_%s},\n", scsyms[i], sigIDs[i], defValOffs[i], funname);
        }
        fprintf(fp, "};\n");
    }


    if (numFfiTbl > 0) {
        fprintf(fp, "const HClass::ClassFfiEntry_t* const INNER_CLASSES[%d] = {\n", numFfiTbl);
        for (hyu32 i = 0; i < numInnerClass; ++i) {
            ClassInfo* inner = m_innerClasses.values()[i];
            if (inner->haveFfi()) {
                fprintf(fp, "  &%s::CLASS_FFI_TABLE,\n", inner->classNameOnCpp());
            }
        }
        fprintf(fp, "};\n");
    }


    fprintf(fp, "extern const HClass::ClassFfiEntry_t CLASS_FFI_TABLE = {\n");
    fprintf(fp, "  %d, %d, %d, ", m_classSym, n, numFfiTbl);

    if (n > 0) {
        fprintf(fp, "METHODS, ");
    } else if (m_superClassInfo_cppClass != NULL) {
        fwriteFfiClassNameOnCpp(fp);
        fprintf(fp, "::METHODS, ");
    } else {
        fprintf(fp, "NULL, ");
    }
    fprintf(fp, (numFfiTbl>0) ? "INNER_CLASSES, " : "NULL, ");
    fprintf(fp, m_bUserDefinedGCMarkFunc ? "HSfg_GCMarkFunc\n" : "NULL\n");
    fprintf(fp, "};\n");

    fprintf(fp, "} // namespace %s\n", classNameOnCpp());
}

void ClassInfo::writeFfiCpp(void)
{
    hyu32 n = m_ffi.size();
    if (n < 1 && !m_needGenerate_cppSize())
        return;

    char path[1024];
    if (! sprintGenFName(path, 1024, gFfiOutDir, "/_", ".cpp"))
        compileError(M_M("path name too long to write FFI cpp file of class %s"), classNameOnCpp());

    DiffWrite dw;
    FILE* fp = dw.open(path);
    if (fp == NULL)
        compileError(M_M("can't write to file %s"), path);

    char hpath[1024];
    if (! sprintGenFName(hpath, 1024, NULL, NULL, ".h"))
        compileError(M_M("path name too long to write FFI header of class %s"), classNameOnCpp());

    fprintf(fp, "/* this file was automatically generated by Hayat compiler */\n"
            "/* !!! DO NOT EDIT !!! */\n"
            "\n"
            "// system call interface of class %s\n"
            "\n"
            "#include \"%s\"\n"
            "\n"
            "FFI_DEFINITION_START {\n"
            , className(), hpath);

    m_fwriteFfiFunc(fp);
    if (m_needGenerate_cppSize()) {
        // 自動生成 *cppSize メソッド
        fprintf(fp, "// class method \"*cppSize\"\n"
                "void m_HSfx_2a63707053697a65(Context* context, int numArgs)\n"
                "{\n"
                "    context->stack.drop(numArgs + 1);\n"
                "    context->pushInt(sizeof(HyCppClass_t));\n"
                "}\n"
                "\n");
    }

    fprintf(fp,
            "} FFI_DEFINITION_END\n");

    dw.close();
}



void ClassInfo::m_decl_pop(FILE* fp, const char* varName, const char* type)
{
    const FfiType& interface = gFfiTypeMgr.get(type);
    if (interface.type() != NULL) {
        fprintf(fp, "%s %s = context->pop()%s;\n", interface.type(), varName, interface.to());
    } else {
        // no C++ interface declaration for 'type'
        fprintf(fp, "Value %s = context->pop();\n", varName);
    }
}

    
void ClassInfo::m_fwriteFfiFunc(FILE* fp)
{
    TArray<SymbolID_t>& syms = m_ffi.keys();
    TArray<FfiSigDecl_t>& sigs = m_ffi.values();

    hyu32 m = syms.size();
    for (hyu32 k = 0; k < m; k++) {
        SymbolID_t ffiSym = syms[k];
        FfiSigDecl_t& decl = sigs[k];
        if (decl.argDecls == NULL)
            continue;          // 可変長引数だと簡易ffiは使えない 
        char funname[248];
        char funname_a[256];

        m_funcName(ffiSym, funname, 248);
        hyu32 idx;
        if (m_userDefinedDeepFfi.search(funname, compStr, idx))
            continue;
        HMD_STRNCPY(funname_a, funname, 256);
        m_funcName_addArity(funname_a, decl.signatureID);
        
        const char* ffiName = gSymTable.id2str(ffiSym);
        hyu32 n = decl.argDecls->size();
        fprintf(fp, "// method \"%s\"\n", ffiName);
        
        fprintf(fp, "void m_%s(Context* context, int numArgs)\n", funname_a);
        fprintf(fp, "{\n");
        fprintf(fp, "#ifndef HMD_DEBUG\n");
        fprintf(fp, "    (void)numArgs;\n");
        fprintf(fp, "#endif\n");
        fprintf(fp, "    HMD_ASSERT(numArgs == %d);\n", n);
        fprintf(fp, "    Value selfVal = context->pop();\n");

        for (int i = (int)n - 1; i >= 0; i--) {
            fprintf(fp, "    ");
            ArgD_t& arg = decl.argDecls->nth(i);
            m_decl_pop(fp, m_nameTable.getStr(arg.argName), m_nameTable.getStr(arg.typeName));
        }        
        const FfiType& retInterface = gFfiTypeMgr.get(m_nameTable.getStr(decl.retType));
        if (m_userDefinedFfi.search(funname_a, compStr, idx)) {
            
            fprintf(fp, "    ");
            if (retInterface.type() != NULL)
                fprintf(fp, "%s retval = ", retInterface.type());
            fprintf(fp, "%s(selfVal", funname);
            for (hyu32 i = 0; i < n; i++) {
                fprintf(fp, ",");
                fprintf(fp, m_nameTable.getStr(decl.argDecls->nth(i).argName));
            }
            fprintf(fp, ");\n");
        } else {
            if (m_cppClass == NULL) {
                compileError(M_M("bad ffi definition %s::%s\n  ffi definition not found in file %s.cpp"), className(), ffiName, classNameOnCpp());
            }
            
            const char* sname = ffiName;
            if (strncmp("HSfx_", funname, 5) == 0)
                sname = funname;
            
            const FfiType& interface = gFfiTypeMgr.get(className());

            fprintf(fp, "    HyCppClass_t* self;\n");
            fprintf(fp, "    if (selfVal.type->symCheck(HSym_Class))\n");
            fprintf(fp, "        self = (HyCppClass_t*)NULL;\n");
            fprintf(fp, "    else\n");
            fprintf(fp, "        self = selfVal%s;\n", interface.to());
            if (retInterface.type() != NULL) {
                fprintf(fp, "    %s retval = self->", retInterface.type());
            } else {
                fprintf(fp, "    self->");
            }
            fprintf(fp, "%s(", sname);

            for (hyu32 i = 0; i < n; i++) {
                fprintf(fp, m_nameTable.getStr(decl.argDecls->nth(i).argName));
                if (i < n - 1)
                    fprintf(fp, ",");
            }
            fprintf(fp, ");\n");

        }

        fprintf(fp, "    context->push(");
        fprintf(fp, retInterface.from(), "retval");
        fprintf(fp, ");\n}\n\n");
    }
}


template<typename T> static void combSort(T arr[], int n)
{
    int h = (n * 10) / 13;
    for (;;) {
        bool noSwap = true;
        for (int i = 0; i + h < n; ++i) {
            T* a = &arr[i];
            T* b = &arr[i+h];
            if (*a > *b) { T tmp = *a; *a = *b; *b = tmp; noSwap = false; }
        }
        if (h <= 1) {
            if (noSwap) break;
        } else {
            h = (h * 10) / 13;
        }
    }
}
static void sortSymbols(TArray<SymbolID_t>& symArr)
{
    combSort<SymbolID_t>(symArr.top(), symArr.size());
}
static void writeSymbols(TArray<hyu8>* out, TArray<SymbolID_t>& symArr)
{
    hyu32 n = symArr.size();
    for (hyu32 i = 0; i < n; i++) {
        packOut<SymbolID_t>(out, symArr[i]);
    }
}
static void readSymbols(int n, const hyu8** pInp, TArray<SymbolID_t>& symArr)
{
    while (n-- > 0) {
        symArr.add(Endian::unpackP<SymbolID_t>(pInp));
    }
}

void ClassInfo::writeClassVarSyms(TArray<hyu8>* out)
{
    sortSymbols(m_classVars);
    writeSymbols(out, m_classVars);
}

void ClassInfo::writeMembVarSyms(TArray<hyu8>* out)
{
    sortSymbols(m_membVars);
    writeSymbols(out, m_membVars);
}

void ClassInfo::writeConstVarSyms(TArray<hyu8>* out)
{
    writeSymbols(out, m_constVars.keys());
}

void ClassInfo::writeSuperLinks(TArray<hyu8>* out)
{
    hyu32 n = m_superClasses.size();
    for (hyu32 i = 0; i < n; i++) {
        ClassInfo* sc = m_superClasses[i];
        // 後で32bitポインタと入れ替えるため、SymbolID_tではなくhyu32でパック
        packOut<hyu32>(out, (hyu32)sc->classSymbol());
    }
}

void ClassInfo::writeUsingPaths(TArray<hyu8>* out)
{
    //# usingパス情報部分の総バイト数(2byte)
    //# パス情報並び
    //   using Ac::Bc::Cc だったら、 {Ac},{Bc},{Cc},0xffff 
    //   これがいくつか並んだ後、0xffff がパス情報終了マーク
    //# パディング

    hyu32 offs = out->size();
    out->addSpaces(sizeof(hyu16)*2); // 後でサイズ情報を入れる場所を確保
    hyu32 n = m_usingList.size();
    for (hys32 i = (hys32)n-1; i >= 0; --i) {
        Scope_t* scope = m_usingList[i];
        hyu32 m = scope->size();
        for (hyu32 j = 0; j < m; j++) {
            packOut<SymbolID_t>(out, scope->nth(j));
        }
        packOut<SymbolID_t>(out, SYMBOL_ID_ERROR);
    }
    // パス情報終了マーク
    packOut<SymbolID_t>(out, SYMBOL_ID_ERROR);
    out->align(4, 0xfc);
    hyu32 need = n * sizeof(hyu32);
    if (need > out->size() - offs) {
        need -= out->size() - offs;
        memset(out->addSpaces(need), 0, need);
    }
    HMD_ASSERT(out->size() - offs <= 0xffff);
    Endian::pack<hyu16>(out->nthAddr(offs), (hyu16)(out->size() - offs)); // サイズ情報を入れる
}





void ClassInfo::serialize(TArray<hyu8>* out)
{
    // m_outerInfo
    m_serializeClassID(out, m_outerInfo);
    // m_superClasses
    hyu16 n16 = numSuper();
    packOut<hyu16>(out, n16);
    for (hyu16 i = 0; i < n16; ++i)
        m_serializeClassID(out, m_superClasses[i]);
    // m_innerClasses
    TArray<ClassInfo*>& inner_ci = m_innerClasses.values();
    hyu32 n32 = inner_ci.size();
    HMD_ASSERT(n32 <= 0xffff);
    packOut<hyu16>(out, (hyu16)n32);
    for (hyu32 i = 0; i < n32; ++i)
        m_serializeClassID(out, inner_ci[i]);
    // m_membVars
    n16 = numMembVar();
    packOut<hyu16>(out, n16);
    writeSymbols(out, m_membVars);
    // m_classVars
    n16 = numClassVar();
    packOut<hyu16>(out, n16);
    writeSymbols(out, m_classVars);
    // constVars
    n16 = numConstVar();
    packOut<hyu16>(out, n16);
    writeSymbols(out, m_constVars.keys());
    TArray<ConstVar_t>& cv = m_constVars.values();
    for (hyu16 i = 0; i < n16; i++) {
        ConstVar_t& c = cv[i];
        packOut<hyu8>(out, (hyu8)c.type);
        packOut<hys32>(out, c.intValue);
    }
    // m_usingList
    n32 = m_usingList.size();
    HMD_ASSERT(n32 <= 0xffff);
    packOut<hyu16>(out, (hyu16)n32);
    for (hyu32 i = 0; i < n32; ++i) {
        Scope_t* s =m_usingList[i];
        hyu32 m = s->size();
        HMD_ASSERT(m <= 0xffff);
        packOut<hyu16>(out, (hyu16)m);
        writeSymbols(out, *s);
    }
    // m_cppClass
    if (m_cppClass == NULL)
        packOut<hyu16>(out, 0);
    else {
        n32 = HMD_STRLEN(m_cppClass);
        HMD_ASSERT(n32 <= 0xffff);
        packOut<hyu16>(out, (hyu16)n32);
        memcpy(out->addSpaces(n32), m_cppClass, n32);
    }

    // m_superClassInfo_cppClass
    m_serializeClassID(out, m_superClassInfo_cppClass);
    // m_methods
    n32 = m_methods.size();
    HMD_ASSERT(n32 <= 0xffff);
    packOut<hyu16>(out, (hyu16)n32);
    writeSymbols(out, m_methods.keys());
    for (hyu32 i = 0; i < n32; ++i) {
        packOut<hyu16>(out, m_methods.values()[i].signatureID);
    }
    // m_ffi
    n32 = m_ffi.size();
    HMD_ASSERT(n32 <= 0xffff);
    packOut<hyu16>(out, (hyu16)n32);
    writeSymbols(out, m_ffi.keys());
    TArray<FfiSigDecl_t>& sigDecl = m_ffi.values();
    for (hyu32 i = 0; i < n32; ++i)
        packOut<hyu16>(out, sigDecl[i].signatureID);
}

// inpからのバイト列を読んでClassInfoを構築する。
// コンストラクタで m_classSym と m_package が初期化されている事が前提。
// バイト列の読み終わった位置を返す。異常があった場合は compileError
const hyu8* ClassInfo::deserialize(const hyu8* inp)
{
    HMD_DEBUG_ASSERT(m_package != NULL);
    HMD_DEBUG_ASSERT(className() != NULL);
    
    // m_outerInfo
    m_outerInfo = m_deserializeClassID(&inp);
    // m_superClasses
    hyu16 n16 = Endian::unpackP<hyu16>(&inp);
    for (hyu16 i = 0; i < n16; ++i)
        m_superClasses.add(m_deserializeClassID(&inp));
    // m_innerClasses
    n16 = Endian::unpackP<hyu16>(&inp);
    for (hyu16 i = 0; i < n16; ++i) {
        ClassInfo* ci = m_deserializeClassID(&inp);
        m_innerClasses[ci->m_classSym] = ci;
    }
    // m_membVars
    n16 = Endian::unpackP<hyu16>(&inp);
    readSymbols(n16, &inp, m_membVars);
    // m_classVars
    n16 = Endian::unpackP<hyu16>(&inp);
    readSymbols(n16, &inp, m_classVars);
    // m_constVars
    n16 = Endian::unpackP<hyu16>(&inp);
    for (hyu16 i = 0; i < n16; ++i) {
        SymbolID_t sym = Endian::unpackP<SymbolID_t>(&inp);
        m_constVars[sym]; // ConstVar_t領域が確保される
    }
    TArray<ConstVar_t>& cv = m_constVars.values();
    HMD_DEBUG_ASSERT(n16 == cv.size());
    for (hyu16 i = 0; i < n16; ++i) {
        ConstVar_t& c = cv[i];
        c.type = (ConstType_e)Endian::unpackP<hyu8>(&inp);
        c.intValue = Endian::unpackP<hys32>(&inp);
    }
    // m_usingList, m_usingClassInfos
    n16 = Endian::unpackP<hyu16>(&inp);
    for (hyu16 i = 0; i < n16; ++i) {
        hyu16 m = Endian::unpackP<hyu16>(&inp);
        Scope_t* s = new Scope_t(m);
        readSymbols(m, &inp, *s);
        addUsing(s);
    }
    // m_cppClass
    n16 = Endian::unpackP<hyu16>(&inp);
    if (n16 == 0)
        m_cppClass = NULL;
    else {
        m_cppClass = gMemPool->allocT<char>(n16+1);
        memcpy((void*)m_cppClass, inp, n16);
        ((char*)m_cppClass)[n16] = '\0';
        inp += n16;
        gFfiTypeMgr.createInterface(className(), m_cppClass);
    }
    // m_superClassInfo_cppClass
    m_superClassInfo_cppClass = m_deserializeClassID(&inp);

    // m_methods
    n16 = Endian::unpackP<hyu16>(&inp);
    static m_SigCode_t nullSigCode = { (Bytecode*)NULL, (hyu16)0 };
    for (hyu16 i = 0; i < n16; ++i) {
        m_methods.forceAdd(Endian::unpackP<SymbolID_t>(&inp), nullSigCode);
    }
    for (hyu16 i = 0; i < n16; ++i) {
        m_methods.values()[i].signatureID = Endian::unpackP<hyu16>(&inp);
    }
    // m_ffi
    n16 = Endian::unpackP<hyu16>(&inp);
    {
        hys32 n32 = n16;
        TArray<SymbolID_t>& symarr = m_ffi.keys();
        symarr.reserve(n32-1);
        for (hys32 i = 0; i < n32; ++i) {
            symarr.subst(i, Endian::unpackP<SymbolID_t>(&inp), 0);
        }
        static FfiSigDecl_t m_dummyDecl = { NULL, 0, 0, NULL_STR_OFFS };
        TArray<FfiSigDecl_t>& sigDecl = m_ffi.values();
        sigDecl.reserve(n32-1);
        for (hys32 i = 0; i < n32; ++i) {
            m_dummyDecl.signatureID = Endian::unpackP<hyu16>(&inp);
            sigDecl.subst(i, m_dummyDecl, m_dummyDecl);
        }
    }


    return inp;
}

/*static*/ void ClassInfo::m_serializeClassID(TArray<hyu8>* out, ClassInfo* ci)
{
    HMD_DEBUG_ASSERT(sizeof(ClassInfo*) >= sizeof(hys32));
    if (ci != NULL) {
        packOut<SymbolID_t>(out, ci->m_package->getSymbol());
        packOut<hys32>(out, ci->m_package->classInfo2ID(ci));
    } else {
        packOut<SymbolID_t>(out, SYMBOL_ID_ERROR);
    }
}

/*static*/ ClassInfo* ClassInfo::m_deserializeClassID(const hyu8** pinp)
{
    // HMD_DEBUG_ASSERT(sizeof(ClassInfo*) >= sizeof(hys32));
    SymbolID_t pkgSym = Endian::unpackP<SymbolID_t>(pinp);
    if (pkgSym != SYMBOL_ID_ERROR) {
        hys32 classID = Endian::unpackP<hys32>(pinp);
        ClassInfo* ci = Package::ID2classInfo(pkgSym, classID);
        HMD_ASSERT(ci != NULL);
        return ci;
    } else {
        return NULL;
    }
}



bool ClassInfo::checkClass(SymbolID_t classSym)
{
    if (m_innerClasses.find(classSym) != NULL)
        return true;
    TArray<ClassInfo*>& a = m_innerClasses.values();
    hyu32 n = a.size();
    for (hyu32 i = 0; i < n; ++i)
        if (a[i]->checkClass(classSym))
            return true;
    return false;
}

bool ClassInfo::checkMethod(SymbolID_t methodSym, const Signature::Sig_t* rightSig)
{
    int min, max;
    const Signature::Sig_t* rs;
    if (binarySearchRange(m_methods.keys().top(), m_methods.size(), methodSym, &min, &max)) {
        TArray<m_SigCode_t>& sigCodes = m_methods.values();
        for (int i = min; i <= max; ++i) {
            const Signature::Sig_t* leftSig = m_package->getSignatureBytes(sigCodes[i].signatureID);
            rs = rightSig;
            if (Signature::canSubst(leftSig, rs))
                return true;
        }
    }

    TArray<SymbolID_t>& ssyms = m_ffi.keys();
    if (binarySearchRange(ssyms.top(), ssyms.size(), methodSym, &min, &max)) {
        TArray<FfiSigDecl_t>& ssds = m_ffi.values();
        for (int i = min; i <= max; ++i) {
            const Signature::Sig_t* leftSig = m_package->getSignatureBytes(ssds[i].signatureID);
            rs = rightSig;
            if (Signature::canSubst(leftSig, rs))
                return true;
        }
    }

    TArray<ClassInfo*>& a = m_innerClasses.values();
    hyu32 n = a.size();
    for (hyu32 i = 0; i < n; ++i) {
        rs = rightSig;
        if (a[i]->checkMethod(methodSym, rs))
            return true;
    }
    return false;
}

bool ClassInfo::checkMethodName(SymbolID_t methodSym)
{
    if (m_methods.keys().isInclude(methodSym))
        return true;
    if (m_ffi.find(methodSym) != NULL)
        return true;

    TArray<ClassInfo*>& a = m_innerClasses.values();
    hyu32 n = a.size();
    for (hyu32 i = 0; i < n; ++i) {
        if (a[i]->checkMethodName(methodSym))
            return true;
    }

    return false;
}



#ifdef EXPERIMENT_SYMBOL_DUMP
static void printArrSyms(const char* title, TArray<SymbolID_t>& symarr)
{
    HMD_PRINTF(title);
    HMD_PRINTF("\n");
    for (hys32 i = 0; i < (hys32)symarr.size(); ++i) {
        SymbolID_t sym = symarr[i];
        HMD_PRINTF("%d (%s)\n", sym, gSymTable.id2str(sym));
    }
}
void ClassInfo::printSyms(void)
{
    HMD_PRINTF("=BEGIN %d (%s)\n", m_classSym, gSymTable.id2str(m_classSym));
    printArrSyms("--- inner classes", m_innerClasses.keys());
    printArrSyms("--- methods", m_methods.keys());
    printArrSyms("--- member vars", m_membVars);
    printArrSyms("--- class vars", m_classVars);
    printArrSyms("--- const vars", m_constVars.keys());
    printArrSyms("--- ffi", m_ffi.keys());
    HMD_PRINTF("=END\n");
}
#endif
