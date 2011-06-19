/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCContext.h"
#include "hyCCompiler.h"
#include "hyC_opcode.h"
#include "hyEndian.h"
#include "hyCFileOut.h"
#include "hyTuning.h"


namespace Hayat {
    namespace Compiler {
        extern SymbolTable gSymTable;
        extern int  jumpLabelCheckLevel;
        extern SymbolID_t gFinallyValVar_SymTop;
    }
}
using namespace Hayat::Common;
using namespace Hayat::Compiler;


extern void compileError(const char* fmt, ...);
extern const char* gFfiOutDir;


Stack<Context*> Context::m_contextStack(0);
Context* Context::m_current = NULL;
TArray<Context*> Context::m_pool(0);
Package* Context::m_package = NULL;
hys32 Context::m_sayCommandIndex = 0;


/*static*/ void Context::initializeAll(void)
{
    m_contextStack.initialize();
    m_current = NULL; 
    m_pool.initialize();
    m_sayCommandIndex = 0;
}
/*static*/ void Context::finalizeAll(void)
{
    m_current = NULL;
    m_contextStack.finalize();
    for (int i = (int)m_pool.size() - 1; i >= 0; --i)
        delete m_pool[i];
    m_pool.finalize();
    m_package = NULL;
}

/*static*/ void Context::newPackageClass(ClassInfo* packageClass)
{
    HMD_DEBUG_ASSERT(m_contextStack.size() == 0);
    HMD_DEBUG_ASSERT(m_package == NULL);
    m_package = packageClass->getPackage();
    Context* cur = new Context(NULL);
    cur->m_classInfo = packageClass;
    push(cur);
}


void* Context::operator new(size_t size)
{
    HMD_DEBUG_ASSERT_EQUAL(sizeof(Context), size);
    MemCell* p = gMemPool->alloc(sizeof(Context));
    m_pool.add((Context*)p);
    return (void*)p;
}

void Context::operator delete(void* p)
{
    gMemPool->free(p);
}

// コンテキストをスタックに積み、カレントとする
/*static*/ void Context::push(Context* child)
{
    m_contextStack.push(child);
    m_current = child;
}

// スタックからカレントを降ろして1つ前のをカレントとする
/*static*/ void Context::pop(void)
{
#ifdef HMD_DEBUG
    Context* p = m_contextStack.pop();
    HMD_DEBUG_ASSERT(p == m_current);
#else
    m_contextStack.pop();
#endif
    m_current = m_contextStack.getTop();
}

// カレントの子コンテキストを作ってスタックに積み、新たなカレントとする
/*static*/ Context* Context::createChild(void)
{
    Context* p = new Context(m_current);
    push(p);
    return p;
}

// カレントのインナークラスコンテキストを作ってスタックに積み、カレントとする
/*static*/ Context* Context::createInnerClass(SymbolID_t classSym)
{
    Context* p = new Context(m_current);
    p->newClass(classSym);
    push(p);
    return p;
}

/*static*/ void Context::jumpLabelCheck(void)
{
    const char* w =
        (Compiler::jumpLabelCheckLevel == 1) ? "WARNING" :
        (Compiler::jumpLabelCheckLevel == 3) ? "ERROR" :
        NULL;
    if (w == NULL) return;

    hyu32 npool = m_pool.size();
    int numErr = 0;
    for (hyu32 i = 0; i < npool; ++i) {
        const TArray<SymbolID_t>& unresLabels = m_pool[i]->getUnresolvedJumpControlLabels();
        hyu32 nl = unresLabels.size();
        for (hyu32 j = 0; j < nl; ++j) {
            SymbolID_t label = unresLabels[j];
            bool notfound = true;
            for (hyu32 k = 0; k < npool; ++k) {
                if (m_pool[k]->haveLabel(label)) {
                    notfound = false;
                    break;
                }
            }
            if (notfound) {
                HMD_PRINTF("%s: goto label ':%s' not found\n", w, gSymTable.id2str(label));
                ++numErr;
            }
        }
    }
    if ((numErr > 0) && (Compiler::jumpLabelCheckLevel == 3))
        compileError(M_M("%d label(s) not found"), numErr);
}




Context::Context(Context* outerContext)
    : m_outerContext(outerContext), m_localVars(0), m_numParamVar(0),
      m_finallyNestLevel(0),
      m_canSeeOuterLocal(false), m_bClosureContext(false),
      m_jumpControlTbl(0), m_unresolvedJumpControlLabels(0),
      m_innerClasses(0), bPushCode(false), bCompileSetVal(false)
{
    if (outerContext != NULL) {
        m_classInfo = outerContext->classInfo();
    } else {
        m_classInfo = NULL;
    }
    m_jumpControlInfo = new JumpControlTree(NULL);
    m_jumpControlTbl.add(m_jumpControlInfo);
}

Context::~Context()
{
    m_localVars.finalize();
    hyu32 n = m_jumpControlTbl.size();
    for (hyu32 i = 0; i < n; i++)
        delete m_jumpControlTbl[i];
    m_jumpControlTbl.finalize();
}


// class開始
void Context::newClass(SymbolID_t classSym)
{
    // sys_printf("newClass(%s)\n",gSymTable.id2str(classSym));
    HMD_ASSERT(m_classInfo != NULL);
    m_classInfo = m_classInfo->addInnerClassCreate(classSym);
    m_outerContext = NULL;
}

// スーパークラス追加
void Context::addSuper(SymbolID_t superClassSym, Context* searchContext)
{
    ClassInfo* s = searchContext->classInfo()->searchClassInfo(superClassSym);
    if (s == NULL)
        compileError(M_M("class '%s' not found"), gSymTable.id2str(superClassSym));
    classInfo()->addSuperClass(s);
}

void Context::postCompileProcess(hyu16 defaultValOffs, hyu16 signatureID)
{
    resolveJumpControl();
    // todo: checkNoSubstLocalVar();
    m_bytecode.setSignature(defaultValOffs, signatureID);
    m_bytecode.setNumLocalAlloc(numLocalVarAlloc());
    hyu32 n = m_jumpControlTbl.size();
    for (hyu32 i = 0; i < n; ++i) {
        m_bytecode.addJumpTable(m_jumpControlTbl[i]->genCode());
    }

    const char* w =
        (Compiler::jumpLabelCheckLevel == 2) ? "WARNING" :
        (Compiler::jumpLabelCheckLevel == 4) ? "ERROR" :
        NULL;
    if (w != NULL) {
        n = m_unresolvedJumpControlLabels.size();
        bool err = false;
        for (hyu32 i = 0; i < n; ++i) {
            SymbolID_t label = m_unresolvedJumpControlLabels[i];
            if (! haveLabel(label)) {
                HMD_PRINTF("%s: goto label ':%s' not found\n", w, gSymTable.id2str(label));
                err = true;
            }
        }
        if (err && (Compiler::jumpLabelCheckLevel == 4))
            compileError(M_M("%d label(s) not found"), n);
    }
}


Context::LocalVar_t* Context::getMyLocalVar(const char* p, hyu32 len, LocalVarSymID_t* pSym)
{
    LocalVarSymID_t lsym = gLocalVarSymbols.check(p, len);
    if (lsym == SYMBOL_ID_ERROR)
        return NULL;
    Context::LocalVar_t* lvp = getLocalVar(lsym);
    if (lvp == NULL)
        lvp = getOuterLocalVar(lsym);
    if (lvp == NULL)
        return NULL;
    // ローカル変数があった
    if (pSym != NULL)
        *pSym = lsym;
    return lvp;
}


Var_t Context::myIdent(const char* p, hyu32 len)
{
    Var_t var;
    LocalVarSymID_t lsym;
    LocalVar_t* lvp = Context::getMyLocalVar(p, len, &lsym);
    if (lvp != NULL) {
        var.type = IT_LOCAL;
        var.symbol = (SymbolID_t)lsym;
        return var;
    }

    SymbolID_t sym = gSymTable.symbolID(p, p+len);

    m_classInfo->getMyIdent(sym, &var);

    return var;
}

Var_t Context::searchVar(const char* p, hyu32 len)
{
    Var_t var;
    LocalVarSymID_t lsym;
    LocalVar_t* lvp = Context::getMyLocalVar(p, len, &lsym);
    if (lvp != NULL) {
        var.type = IT_LOCAL;
        var.symbol = (SymbolID_t)lsym;
        return var;
    }

    SymbolID_t sym = gSymTable.symbolID(p, p+len);
    var.symbol = sym;

    ClassInfo* cinf = m_classInfo;
    if (cinf->searchClassInfo(sym) != NULL) {
        var.type = IT_CLASS;
        var.classInfo = cinf;
        return var;
    }

    ConstVar_t* pv = cinf->searchConstVar(sym);
    if (pv != NULL) {
        var.type = IT_CONST;
        var.constInfo = pv;
        return var;
    }

    MethodCallable_e mt = cinf->searchMethod(sym);
    if (mt != MT_NONE) {
        var.type = IT_METHOD;
        var.methodInfo = mt;
        return var;
    }

    var.type = IT_NONE;
    return var;
}

// メソッド仮引数作成
void Context::createParamVar(LocalVarSymID_t varSym)
{
    if (m_localVars.find(varSym) != NULL) {
        compileError(M_M("duplicated parameter variable name %s"), gLocalVarSymbols.localVarSymID2str(varSym));
    }
    hys16 idx = (hys16) m_localVars.size();
    LocalVar_t& v = m_localVars[varSym];
    v.idx = idx;
    v.outerIdx = -1;
    v.substFlag = true;
    ++ m_numParamVar;
}

Context::LocalVar_t* Context::getOuterLocalVar(LocalVarSymID_t varSym)
{
    LocalVar_t* p = NULL;
    if (m_canSeeOuterLocal && m_outerContext != NULL) {
        p = m_outerContext->getLocalVar(varSym);
        if (p == NULL)
            p = m_outerContext->getOuterLocalVar(varSym);
        if (p != NULL) {
            hys16 idx = (hys16) m_localVars.size();
            hys16 outerIdx = p->idx;
            p  = &m_localVars[varSym];
            p->idx = idx;
            p->outerIdx = outerIdx;
            p->substFlag = true;
        }
    }
    return p;
}

// 可視ローカル変数が無ければ作成
Context::LocalVar_t& Context::getLocalVarCreate(LocalVarSymID_t varSym)
{
    LocalVar_t* p = getLocalVar(varSym);
    if (p != NULL)
        return *p;

    p = getOuterLocalVar(varSym);
    if (p != NULL)
        return *p;

    hys16 idx = (hys16) m_localVars.size();
    HMD_ASSERT(idx >= 0);
    LocalVar_t& v = m_localVars[varSym];
    v.idx = idx;
    v.outerIdx = -1;
    v.substFlag = false;
    return v;
}

void Context::addCodePushInt(hys32 v)
{
    if (v == 0) {
        addCode<OP_push_i0>();
    } else if (v == 1) {
        addCode<OP_push_i1>();
    } else if (v == -1) {
        addCode<OP_push_im1>();
    } else if (v >= -128 && v <= 127) {
        addCode<OP_push_i8>((hys8)v);
    } else {
        addCode<OP_push_i>(v);
    }
}

void Context::addCodePushFloat(hyf32 v)
{
    if (v == 0.0f) {
        addCode<OP_push_f0>();
    } else if (v == 1.0f) {
        addCode<OP_push_f1>();
    } else if (v == -1.0f) {
        addCode<OP_push_fm1>();
    } else {
        addCode<OP_push_f>(v);
    }
}

// メソッド追加
void Context::addMethod(SymbolID_t methodSym, Context* methodContext)
{
    m_classInfo->addMethod(methodSym, &(methodContext->bytecode()));
}

// インナークラス登録
void Context::addInnerClass(Context* classContext)
{
    HMD_DEBUG_ASSERT(classContext != this);
    m_innerClasses[classContext->m_classInfo->classSymbol()] = classContext;
}

// クロージャ登録
void Context::addClosure(Context* callContext, Context* closureContext)
{
    if (m_outerContext != NULL) {
        m_outerContext->addClosure(callContext, closureContext);
        return;
    }

    int numShared = 0;
    const TArray<LocalVar_t>& vars = closureContext->m_localVars.values();
    hyu32 n = vars.size();
    for (hyu32 i = 0; i < n; i++) {
        if (vars[i].outerIdx >= 0)
            ++ numShared;
    }
    HMD_ASSERT(numShared < 255);
    callContext->addCode<OP_closure>((hyu16)m_classInfo->numClosure(), (hyu8)numShared);
    for (hyu32 i = 0; i < n; i++) {
        LocalVar_t& v = vars[i];
        if (v.outerIdx >= 0)
            callContext->addCode<OP_shareLocal>(v.idx, v.outerIdx);
    }
    m_classInfo->addClosure(&(closureContext->bytecode()));
}



void Context::jumpControlStart(LocalVarSymID_t catchVar, LocalVarSymID_t finallyValVar)
{
    hyu32 n = m_jumpControlTbl.size();
    if (n >= 255)
        compileError(M_M("too many jump controls in a context"));
    m_jumpControlInfo = m_jumpControlInfo->newChild();
    m_jumpControlTbl.add(m_jumpControlInfo);
    hys16 catchVarIdx, finallyValVarIdx;
    if (catchVar != SYMBOL_ID_ERROR)
        catchVarIdx = getLocalVarCreate(catchVar).idx;
    else
        catchVarIdx = NO_LOCAL_VAR;
    if (finallyValVar != SYMBOL_ID_ERROR)
        finallyValVarIdx = getLocalVarCreate(finallyValVar).idx;
    else
        finallyValVarIdx = NO_LOCAL_VAR;

    addCode<OP_jumpControlStart>(catchVarIdx, finallyValVarIdx, (hyu8)n);
    m_jumpControlInfo->startAddr = codeAddr();
}

void Context::jumpControlEnd(void)
{
    addCode<OP_jumpControlEnd>();
    m_jumpControlInfo = m_jumpControlInfo->parent();
}

void Context::resolveJumpControl(void)
{
    if (m_jumpControlTbl.size() > 0)
        m_jumpControlTbl[0]->resolve(this);
}

// ジャンプアドレス解決
void Context::resolveJumpAddr(hyu32 resolveAddr, hyu32 jumpAddr)
{
    if (jumpAddr == JumpControlTree::INVALID_ADDR)
        jumpAddr = codeAddr();
    hys32 relative = jumpAddr - resolveAddr - OPR_RELATIVE::SIZE;
    hyu8 r[OPR_RELATIVE::SIZE];
    Endian::pack<OPR_RELATIVE::BaseType_t>(r, relative);
    m_bytecode.replaceCodes(resolveAddr, r, OPR_RELATIVE::SIZE);
}

// jump命令をjumpControl命令に置き換える
void Context::replaceJumpControl(hyu32 resolveAddr, SymbolID_t label)
{
    HMD_DEBUG_ASSERT(m_bytecode.getCode(resolveAddr - 1) == OPL_jump);
    hyu8 b[8];
    b[0] = OPL_jumpControl;
    Endian::pack<SymbolID_t>(&b[1], label);
    m_bytecode.replaceCodes(resolveAddr - 1, b, sizeof(SymbolID_t) + 1);
    if ((! Compiler::isJumpControlLabel(label)) &&
        (! m_unresolvedJumpControlLabels.isInclude(label)))  {
        m_unresolvedJumpControlLabels.add(label);
    }
}

LocalVarSymID_t Context::useFinallyValVar(void)
{
    if (m_finallyNestLevel >= MAX_FINALLY_NEST)
        return SYMBOL_ID_ERROR;
    LocalVarSymID_t finallyValVar = gFinallyValVar_SymTop + m_finallyNestLevel;
    ++ m_finallyNestLevel;
    return finallyValVar;
}

void Context::endUseFinallyValVar(void)
{
    HMD_DEBUG_ASSERT(m_finallyNestLevel > 0);
    -- m_finallyNestLevel;
}

bool Context::haveLabel(SymbolID_t label)
{
    if (m_jumpControlTbl.size() > 0)
        return m_jumpControlTbl[0]->haveLabel(label);
    return false;
}

void Context::setSourceInfo(hyu32 parsePos)
{
    SourceInfo* psi = m_bytecode.getSourceInfoToAdd();
    gpInp->buildSourceInfo(psi, parsePos);
    //printf("%x %x %x %d:%d \n",this,psi,parsePos,psi->line,psi->col);
    Hayat::Compiler::setCompileErrorPos(parsePos);
}







void Context::writeByteCodes(TArray<hyu8>* out)
{
    TArray<hyu8> classInitializer;
    m_bytecode.write(&classInitializer);
    TArray< TArray<hyu8>* > binaries;
    TArray< TArray<hyu8>* > needDeleteBin;
    //innerClassSyms = @innerClasses.keys.sort
    TArray<SymbolID_t>& innerClassSyms = m_innerClasses.keys();

    hyu16 flags = 0;
    if (classInfo()->isPrimitive())
        flags |= 0x0001;

    hyu32 numInnerClass = innerClassSyms.size();
    HMD_ASSERT(numInnerClass < 65536);

    TArray<SymbolID_t>& methodSyms = m_classInfo->m_methods.keys();
    TArray<ClassInfo::m_SigCode_t>& methodSigCodes = m_classInfo->m_methods.values();
    hyu32 numMethod = methodSyms.size();
    HMD_ASSERT(numMethod < 65536);
    hyu32 numClosure = m_classInfo->numClosure();
    HMD_ASSERT(numClosure < 65536);

    packOut<hyu16>(out,flags);                // フラグ hyu16
    packOut<hyu16>(out,(hyu16)numMethod);      // メソッド数 hyu16
    packOut<hyu16>(out,(hyu16)numInnerClass);   // インナークラス数 hyu16
    packOut<hyu16>(out,m_classInfo->numSuper());    // スーパークラス数 hyu16
    packOut<hyu16>(out,m_classInfo->numClassVar()); // クラス変数数 hyu16
    packOut<hyu16>(out,m_classInfo->numMembVar()); // メンバ変数数 hyu16
    packOut<hyu16>(out,m_classInfo->numConstVar());    // 定数数 hyu16
    packOut<hyu16>(out,m_classInfo->numDefaultVal()); // デフォルト値数
    packOut<hyu16>(out,(hyu16)numClosure);            // クロージャ数 hyu16
    packOut<hyu16>(out,0xf9f9); // 未使用

    for (hyu32 i = 0; i < numMethod; i++)
        packOut<SymbolID_t>(out,methodSyms[i]); // メソッドシンボル
    for (hyu32 i = 0; i < numInnerClass; i++)
        packOut<SymbolID_t>(out,innerClassSyms[i]);    // クラス名シンボル


    m_classInfo->writeClassVarSyms(out); // クラス変数シンボル
    m_classInfo->writeMembVarSyms(out); // メンバ変数シンボル
    m_classInfo->writeConstVarSyms(out); // 定数シンボル


    out->align(4, 0xfb);


    hyu32 pos = classInitializer.size();

    for (hyu32 i = 0; i < numMethod; i++) {
        packOut<hyu32>(out, pos);     // メソッドオフセット
        Bytecode* bc = methodSigCodes[i].pBytecode;
        TArray<hyu8>* bin = new TArray<hyu8>(128);
        needDeleteBin.add(bin);
        bc->setOffset(pos);
        bc->write(bin);
        binaries.add(bin);
        pos += bin->size();
        HMD_DEBUG_ASSERT((pos & 3) == 0);
    }
    for (hyu32 i = 0; i < numClosure; i++) {
        packOut<hyu32>(out, pos);     // クロージャオフセット
        Bytecode* bc = m_classInfo->m_closures[i];
        TArray<hyu8>* bin = new TArray<hyu8>(128);
        needDeleteBin.add(bin);
        bc->setOffset(pos);
        bc->write(bin);
        binaries.add(bin);
        pos += bin->size();
        HMD_DEBUG_ASSERT((pos & 3) == 0);
    }

    for (hyu32 i = 0; i < numInnerClass; i++) {
        packOut<hyu32>(out, pos);     // クラスオフセット
        Context* inner = m_innerClasses[innerClassSyms[i]];
        TArray<hyu8>* bin = new TArray<hyu8>(128);
        needDeleteBin.add(bin);
        inner->bytecode().setOffset(pos);
        inner->writeByteCodes(bin);
        bin->align(4, 0xfa);
        binaries.add(bin);
        pos += bin->size();
    }

    packOut<hyu32>(out, pos); // インナークラスバイトコードの終了位置オフセット

    m_classInfo->writeSuperLinks(out); // スーパークラス
    m_classInfo->writeUsingPaths(out); // usingパス情報

    //printf("classInitializer addr=%x\n",out->size());
    m_bytecode.setOffset(m_bytecode.getOffset() + out->size());
    out->add(classInitializer);
    hyu32 n = binaries.size();
    for (hyu32 i = 0; i < n; i++) {
        out->add(*binaries[i]);
    }
    n = needDeleteBin.size();
    for (hyu32 i = 0; i < n; i++) {
        delete needDeleteBin[i];
    }
#ifdef EXPERIMENT_SYMBOL_DUMP
    m_classInfo->printSyms();
#endif
}

void Context::fwriteByteCodes(FILE* fp)
{
    TArray<hyu8> out(256);
    writeByteCodes(&out);
    size_t nwrite = fwrite(out.top(), 1, out.size(), fp);
    HMD_ASSERT(nwrite == out.size());
}

void Context::writeFfi(void)
{
    m_classInfo->generateCppProg();
}

void Context::writeFfiTbl(const char* ffiTblFileName)
{
    if (! m_classInfo->haveFfi())
        return;
    
    hmd_mkdir(gFfiOutDir, 0775);

    DiffWrite dw;
    FILE* fp = dw.open(ffiTblFileName);

    writeGenHead(fp);
    fprintf(fp, "#include \"hyClass.h\"\n");
    fprintf(fp, "using namespace Hayat::Common;\n");
    fprintf(fp, "using namespace Hayat::Engine;\n");

    m_classInfo->fwriteIncludeCHeader(fp);

    fprintf(fp, "namespace Hayat {\n");
    fprintf(fp, "namespace Ffi {\n");

    m_classInfo->fwriteClassFfiTable(fp);

    fprintf(fp, "} // namespace Ffi\n");
    fprintf(fp, "} // namespace Hayat\n");

    dw.close();
}


void Context::m_fwriteDebugInfo(FILE* fp, TArray<const char*>& paths, hyu32 offs)
{
    m_bytecode.fwriteDebugInfo(fp, paths, offs);
    offs += m_bytecode.getOffset();

    hyu16 n = m_classInfo->numMethod();
    for (hyu16 i = 0; i < n; ++i)
        m_classInfo->getMethodBytecode(i)->fwriteDebugInfo(fp, paths, offs);
    n = m_classInfo->numClosure();
    for (hyu16 i = 0; i < n; ++i)
        m_classInfo->getClosureBytecode(i)->fwriteDebugInfo(fp, paths, offs);

    hyu32 num = m_innerClasses.size();
    Context** ppc = m_innerClasses.values().top();
    for (hyu32 i = 0; i < num; i++,++ppc) {
        (*ppc)->m_fwriteDebugInfo(fp, paths, offs);
    }
}

void Context::fwriteDebugInfo(FILE* fp, hyu32 offs)
{
    TArray<const char*> paths;

    m_fwriteDebugInfo(fp, paths, offs);

    // SourceInfo終了マーク
    hyu8 buf[4];
    memset(buf, 0, 4);
    fwrite(buf, sizeof(hyu32), 1, fp);

    // パス情報書き出し
    hyu32 n = paths.size();
    Endian::pack<hyu16>(buf, (hyu16)n);
    fwrite(buf, sizeof(hyu16), 1, fp);
    for (hyu32 i = 0; i < n; i++) {
        const char* p = paths[i];
        fwrite(p, 1, HMD_STRLEN(p)+1, fp);
    }
}
