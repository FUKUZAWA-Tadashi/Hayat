/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "grammar.tab.h"
#include "hyCCompiler.h"
#include "hyCContext.h"
#include "hyC_opcode.h"
#include "hyCStrTable.h"
#include "hyCSignature.h"
#include "hyMemPool.h"
#include "hyTuning.h"
#include "hyCCompileError.h"
#include "hyCCompileFile.h"

namespace Hayat {
    namespace Compiler {
        extern StrTable gStrTable;
        extern SymbolTable gSymTable;
        extern SymbolTable gLocalVarSymbols;

        extern SymbolID_t gFinallyValVar_SymTop;

        int  jumpLabelCheckLevel;


        static SymbolID_t HyCSym_sayCommandStart;
        static SymbolID_t HyCSym_sayCommandEnd;
        static SymbolID_t HyCSym_sayCommand;
        static SymbolID_t HyCSym_Fiber;
        static SymbolID_t HyCSymS_jumpControlStart;
        static SymbolID_t HyCSymS_jumpControlEnd;
        static SymbolID_t HyCSymS_next;
        static SymbolID_t HyCSymS_break;
        static SymbolID_t HyCSymS_catch;
        static SymbolID_t HyCSymS_finally;
        static SymbolID_t HyCSym_exception;
        static SymbolID_t HyCSym_goto;
        static SymbolID_t HyCSym_multisubst_failed;
        static SymbolID_t HyCSym_Array;
        static SymbolID_t HyCSym_Hash;
        static SymbolID_t HyCSym_StringBuffer;
        static SymbolID_t HyCSym_Method;
        static SymbolID_t HyCSymx_5b5d; // []
        static SymbolID_t HyCSymx_7c7c; // ||
        static SymbolID_t HyCSymx_2626; // &&
        static SymbolID_t HyCSymx_2e2e3c; // ..<
        static SymbolID_t HyCSymx_2e2e;   // ..
        static SymbolID_t HyCSymx_3c3d3e; // <=>
        static SymbolID_t HyCSymx_3d3d3d; // ===
        static SymbolID_t HyCSymx_3d3d; // ==
        static SymbolID_t HyCSymx_213d; // !=
        static SymbolID_t HyCSymx_3c3e; // <>
        static SymbolID_t HyCSymx_3c3d; // <=
        static SymbolID_t HyCSymx_3e3d; // >=
        static SymbolID_t HyCSymx_3c; // <
        static SymbolID_t HyCSymx_3e; // >
        static SymbolID_t HyCSymx_7c; // |
        static SymbolID_t HyCSymx_26; // &
        static SymbolID_t HyCSymx_7e; // ~
        static SymbolID_t HyCSymx_2b; // +
        static SymbolID_t HyCSymx_2d; // -
        static SymbolID_t HyCSymx_2b40; // +@
        static SymbolID_t HyCSymx_2d40; // -@
        static SymbolID_t HyCSymx_2a; // *
        static SymbolID_t HyCSymx_2f; // /
        static SymbolID_t HyCSymx_25; // %
        static SymbolID_t HyCSymx_2a2a; // **
        static SymbolID_t HyCSymx_21; // !
        static SymbolID_t HyCSymx_2d3e; // ->
        static SymbolID_t HyCSymx_5b5d3d; // []=
        static SymbolID_t HyCSymx_5e; // ^
        static SymbolID_t HyCSymx_2829; // ()

        SymbolID_t HyCSymS_cppSize;
        SymbolID_t HyCSym_Object;
        SymbolID_t HyCSym_nil;

        static SymbolID_t LocalVarSym_args_; // gLocalVarSymbols.localVarSymID("args_")

        extern const char* exceptionSymbols[];
    }
}



using namespace Hayat::Common;
using namespace Hayat::Compiler;


#define NUMELEMS(x) (sizeof(x)/sizeof((x)[0]))


void Hayat::Compiler::initializeCompiler(void)
{
    HyCSym_sayCommandStart = gSymTable.symbolID("sayCommandStart");
    HyCSym_sayCommandEnd = gSymTable.symbolID("sayCommandEnd");
    HyCSym_sayCommand = gSymTable.symbolID("sayCommand");
    HyCSym_Fiber = gSymTable.symbolID("Fiber");
    // ↓以下6個順番変更禁止  Hayat::Compiler::isJumpControlLabel で使用
    HyCSymS_jumpControlStart = gSymTable.symbolID("*jumpControlStart");
    HyCSymS_jumpControlEnd = gSymTable.symbolID("*jumpControlEnd");
    HyCSymS_next = gSymTable.symbolID("*next");
    HyCSymS_break = gSymTable.symbolID("*break");
    HyCSymS_catch = gSymTable.symbolID("*catch");
    HyCSymS_finally = gSymTable.symbolID("*finally");
    // ↑以上6個順番変更禁止  Hayat::Compiler::isJumpControlLabel で使用
    HyCSym_exception = gSymTable.symbolID("exception");
    HyCSym_goto = gSymTable.symbolID("goto");
    HyCSym_multisubst_failed = gSymTable.symbolID("multisubst_failed");
    HyCSym_Array = gSymTable.symbolID("Array");
    HyCSym_Hash = gSymTable.symbolID("Hash");
    HyCSym_StringBuffer = gSymTable.symbolID("StringBuffer");
    HyCSym_Method = gSymTable.symbolID("Method");
    HyCSymx_5b5d = gSymTable.symbolID("[]");
    HyCSymx_7c7c = gSymTable.symbolID("||");
    HyCSymx_2626 = gSymTable.symbolID("&&");
    HyCSymx_2e2e3c = gSymTable.symbolID("..<");
    HyCSymx_2e2e = gSymTable.symbolID("..");
    HyCSymx_3c3d3e = gSymTable.symbolID("<=>");
    HyCSymx_3d3d3d = gSymTable.symbolID("===");
    HyCSymx_3d3d = gSymTable.symbolID("==");
    HyCSymx_213d = gSymTable.symbolID("!=");
    HyCSymx_3c3e = gSymTable.symbolID("<>");
    HyCSymx_3c3d = gSymTable.symbolID("<=");
    HyCSymx_3e3d = gSymTable.symbolID(">=");
    HyCSymx_3c = gSymTable.symbolID("<");
    HyCSymx_3e = gSymTable.symbolID(">");
    HyCSymx_7c = gSymTable.symbolID("|");
    HyCSymx_26 = gSymTable.symbolID("&");
    HyCSymx_7e = gSymTable.symbolID("~");
    HyCSymx_2b = gSymTable.symbolID("+");
    HyCSymx_2d = gSymTable.symbolID("-");
    HyCSymx_2b40 = gSymTable.symbolID("+@");
    HyCSymx_2d40 = gSymTable.symbolID("-@");
    HyCSymx_2a = gSymTable.symbolID("*");
    HyCSymx_2f = gSymTable.symbolID("/");
    HyCSymx_25 = gSymTable.symbolID("%");
    HyCSymx_2a2a = gSymTable.symbolID("**");
    HyCSymx_21 = gSymTable.symbolID("!");
    HyCSymx_2d3e = gSymTable.symbolID("->");
    HyCSymx_5b5d3d = gSymTable.symbolID("[]=");
    HyCSymx_5e = gSymTable.symbolID("^");
    HyCSymx_2829 = gSymTable.symbolID("()");

    HyCSymS_cppSize = gSymTable.symbolID("*cppSize");
    HyCSym_Object = gSymTable.symbolID("Object");
    HyCSym_nil = gSymTable.symbolID("nil");

    gSymTable.symbolID("initialize");
    gSymTable.symbolID("finalize");
    gSymTable.symbolID("run");
    gSymTable.symbolID("*jump_control");

    const char** p = exceptionSymbols;
    //100個を越えるようだとテーブルの末尾にNULLが無いバグだろう
    for (int i = 100; i > 0; --i) {
        if (*p == NULL)
            break;
        gSymTable.symbolID(*p++);
    }

    gLocalVarSymbols.initialize(NULL);
    LocalVarSym_args_ = gLocalVarSymbols.localVarSymID("args_");
    char fvs[8];
    HMD_STRNCPY(fvs, "*fa*", 5);
    gFinallyValVar_SymTop = gLocalVarSymbols.localVarSymID(fvs);
    for (int i = 1; i < MAX_FINALLY_NEST; ++i) {
        fvs[2] = 'a' + i;
#ifdef HMD_DEBUG
        SymbolID_t id = gLocalVarSymbols.localVarSymID(fvs);
        HMD_DEBUG_ASSERT(id == gFinallyValVar_SymTop + i);
#else
        gLocalVarSymbols.localVarSymID(fvs);
#endif
    }

    // jumpLabelCheckLevel = 4;  // main()で初期化
}

void Hayat::Compiler::finalizeCompiler(void)
{
    gLocalVarSymbols.finalize();
}


bool Hayat::Compiler::isJumpControlLabel(SymbolID_t label)
{
    return (label >= HyCSymS_jumpControlStart && label <= HyCSymS_finally);
}


#define COMPILE_AT(name) {      \
        char buf[40];                 \
        gpInp->copyStr(buf, 40, str);                 \
        HMD_PRINTF("compile %s  at %s\n",#name,buf); }
#if 0
#define PRE_COMPILE_HOOK(name) COMPILE_AT(name)
#else
#define PRE_COMPILE_HOOK(name) ((void)0)
#endif

//============================================================
//@ main            <- classBody %z EOF
void P_main::actionAtParse(SyntaxTree*){}
void ST_main::compile(Context* context)
{
    PRE_COMPILE_HOOK(main);
    get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    context->needLastVal();
    context->addCode<OP_return>();
    context->writeFfi();
}

//============================================================
//@ classBody       <- %z (statements $1 %separator)?
//@ classBody$1 = "semicolon or linefeed required after statement"
void P_classBody::actionAtParse(SyntaxTree*){}
void ST_classBody::compile(Context* context)
{
    PRE_COMPILE_HOOK(classBody);
    context->bPushCode = false;
    if (numChild() > 0)
        get(0)->compile(context);
}

//============================================================
//@ separator       <- %s %(";" / "\n") %z
void P_separator::actionAtParse(SyntaxTree*){}
void ST_separator::compile(Context*){}

//============================================================
//@ # クラス定義 
//@ defClass        <- defClassStart $1 %separator classBody %'end'
//@ defClass$1 = "';' or end of line required"
void P_defClass::actionAtParse(SyntaxTree*){}
void ST_defClass::compile(Context* context)
{
    // classInitの実行結果を返す
    PRE_COMPILE_HOOK(defClass);
    context->popLastVal();
    get(0)->compile(context); // defClassStart: new class context pushed
    Context* bodyContext = Context::current();
    HMD_DEBUG_ASSERT(0 == bodyContext->classInfo()->numDefaultVal());
    get(1)->compile(bodyContext);       // classBody
    bodyContext->setSourceInfo(str.endPos-3);
    bodyContext->needLastVal();
    bodyContext->addCode<OP_return>();
    bodyContext->postCompileProcess(0, 0);
    if (! bodyContext->classInfo()->checkCppClass()) {
        compileError_pos(str.startPos,
                         M_M("in class %s: only 1 C++ class allowed in a Hayat class ; including inherited class"),
                         bodyContext->classInfo()->className());
    }
    Context::pop();
    HMD_DEBUG_ASSERT(context == Context::current());
    context->addInnerClass(bodyContext);
    context->setSourceInfo(str.endPos);
    context->addCode<OP_classInit>(bodyContext->classInfo()->classSymbol());
    context->bPushCode = true;
    bodyContext->writeFfi();
}

//============================================================
//@ defClassStart   <- %'class' %spc $1 IDENT (%s %"<" %z $2 superClassList)?
//@ defClassStart$1 = "class name required"
//@ defClassStart$2 = "super class list required"
void P_defClassStart::actionAtParse(SyntaxTree*){}
void ST_defClassStart::compile(Context* context)
{
    PRE_COMPILE_HOOK(defClassStart);
    SymbolID_t classNameSym = gSymTable.symbolID(gpInp, get(0)->str);

    Var_t var = context->myIdent(gpInp, get(0)->str);
    switch (var.type) {
    case IT_CONST:
    case IT_METHOD:
    case IT_LOCAL:
        compileError_pos(get(0),
                         M_M("class name '%s' conflict with %s name"),
                         gSymTable.id2str(classNameSym),
                         VAR_IDENT_STR[var.type]);
    default:
        ;
    }

    Context* inner = context->createInnerClass(classNameSym);
    if (numChild() > 1) {
        SyntaxTree* superClassList = get(1);
        int numSuperClassList = superClassList->numChild();
        for (int i = 0; i < numSuperClassList; i++) {
            SymbolID_t superClassSym = gSymTable.symbolID(gpInp, superClassList->get(i)->str);
            inner->addSuper(superClassSym, context);
        }
    }
}

//============================================================
//@ superClassList  <- IDENT (%s %"," %z $1 IDENT)*
//@ superClassList$1 = "existing class name required"
void P_superClassList::actionAtParse(SyntaxTree*){}
void ST_superClassList::compile(Context* context){}

//============================================================
//@ # プログラム1かたまり 
// 複文コンパイル
static void compileCompound(SyntaxTree* st, Context* context)
{
    int numStatement = st->numChild();
    context->bPushCode = false;
    for (int i = 0; i < numStatement; i++) {
        st->get(i)->compile(context);
    }
}
//============================================================
//@ statement_end   <- %s $1 &(";" / "}" / "\n")
//@ statement_end$1 = "statement separator required"
void P_statement_end::actionAtParse(SyntaxTree*){}
void ST_statement_end::compile(Context*){}

//============================================================
//@ statements      <- (include %z)* statement %statement_end (%separator (include %z)* statement %statement_end)*
void P_statements::actionAtParse(SyntaxTree*){}
void ST_statements::compile(Context* context)
{
    PRE_COMPILE_HOOK(statements);
    compileCompound(this, context);
}

//============================================================
//@ progn_1         <- (include %z)* statement_1 %statement_end (%separator (include %z)* statement_1 %statement_end)*
void P_progn_1::actionAtParse(SyntaxTree*){}
void ST_progn_1::compile(Context* context)
{
    PRE_COMPILE_HOOK(progn_1);
    compileCompound(this, context);
}
//============================================================
//@ progn           <- progn_1?
void P_progn::actionAtParse(SyntaxTree*){}
void ST_progn::compile(Context* context)
{
    PRE_COMPILE_HOOK(progn);
    if (numChild() > 0)
        compileCompound(get(0), context);
    else
        context->bPushCode = false;
}

//============================================================
//@ statement       <- !'end' (defMethod / defFiber / defClass / ffi / require / using / statement_1)
void P_statement::actionAtParse(SyntaxTree*){}
void ST_statement::compile(Context* context)
{
    PRE_COMPILE_HOOK(statement);
    get(0)->get(0)->compile(context);
    //context->setSourceInfo(str.endPos);
}

//============================================================
//@ statement_1     <- if / loop / while / for / match
//@                  / break / next / label / goto 
//@                  / return / enum / try / throw / subst
//@                  / statement_exp / statement_0
void P_statement_1::actionAtParse(SyntaxTree*){}
void ST_statement_1::compile(Context* context)
{
    PRE_COMPILE_HOOK(statement_1);
    get(0)->get(0)->compile(context);
}

//============================================================
//@ statement_exp   <- exp
void P_statement_exp::actionAtParse(SyntaxTree*){}
void ST_statement_exp::compile(Context* context)
{
    PRE_COMPILE_HOOK(statement_exp);
    context->popLastVal();
    get(0)->compile(context);
    context->bPushCode = true;
}

//============================================================
//@ statement_0     <- &";"
void P_statement_0::actionAtParse(SyntaxTree*){}
void ST_statement_0::compile(Context* context)
{
    PRE_COMPILE_HOOK(statement_0);
}

//============================================================
//@ subst_or_exp    <- subst / exp
void P_subst_or_exp::actionAtParse(SyntaxTree*){}
void ST_subst_or_exp::compile(Context* context)
{
    PRE_COMPILE_HOOK(subst_or_exp);
    context->popLastVal();
    get(0)->get(0)->compile(context);
    context->bPushCode = true;
}



//============================================================
//@ # 空白,改行 
//@ #spcChar         <- " " / "\t" / "\r" / "　" / comment
//@ s               <% %spcChar*
void ST_s::compile(Context*){}

//============================================================
//@ spc             <% %spcChar+
void ST_spc::compile(Context*){}

//============================================================
//@ z               <% %(spcChar / "\n")*
void ST_z::compile(Context*){}

//============================================================
//@ # コメント 
//@ comment         <% %comment1 / %comment2
//@ #comment1        <- %"//" (!"\n" %.)*
//@ #comment2        <- %"/*" (!"*/" (%comment2 / %.))* %"*/"
void ST_comment::compile(Context*){}


//============================================================
//@ scopeMethodCall <- scopedIDENT %s argList
void P_scopeMethodCall::actionAtParse(SyntaxTree*){}
void ST_scopeMethodCall::compile(Context* context)
{
    PRE_COMPILE_HOOK(scopeMethodCall);
    get(1)->compile(context);   // argList
    hyu16 signatureID = (hyu16) context->tmp_int;
    Scope_t scope(4);
    context->tmp_ptr = &scope;
    context->setSourceInfo(str.startPos);
    get(0)->compile(context);   // scopedIDENT
    SymbolID_t methodSym = scope[-1];
    scope.remove(-1);
    if (context->classInfo()->checkScope_inSuper(&scope)) {
        // スコープがスーパークラスである
        Context::getPackage()->checkMethod(methodSym, signatureID, str.startPos);
        context->addCode<OP_getScopeClass>(&scope);
        context->setSourceInfo(str.endPos);
        context->addCode<OP_scopeMethod>(signatureID, methodSym);
    } else {
        // スコープがスーパークラスでない
        context->addCode<OP_getScopeClass>(&scope);
        context->setSourceInfo(str.endPos);
        context->addCode<OP_insMethod>(signatureID, methodSym);
    }
}




//============================================================
// 値取得
void compileVar(Context* context, Var_t& var)
{
    switch (var.type) {
    case IT_CLASS:
        if (context->bCompileSetVal) {
            compileError(M_M("'%s' is a class name. cannot be substituted"), gSymTable.id2str(var.symbol));
        }
        context->addCode<OP_getClass>(var.symbol);
        break;

    case IT_CONST:
        HMD_ASSERT(var.constInfo != NULL);
        if (context->bCompileSetVal) {
            compileError(M_M("constant '%s' value cannot be changed"), gSymTable.id2str(var.symbol));
        }
        switch (var.constInfo->type) {
        case CONST_TYPE_INT:
            context->addCodePushInt(var.constInfo->intValue);
            break;
        case CONST_TYPE_FLOAT:
            context->addCodePushFloat(var.constInfo->floatValue);
            break;
        default:
            context->addCode<OP_getConstVar>(var.symbol);
        }
        break;

    case IT_METHOD:
        if (context->bCompileSetVal) {
            compileError(M_M("'%s' is a method. cannot be substituted"), gSymTable.id2str(var.symbol));
        }
        context->addCode<OP_method_0>(var.symbol);
        break;

    case IT_LOCAL:
        {
            Context::LocalVar_t* p = context->getLocalVar(var.symbol);
            if (p == NULL)
                p = context->getOuterLocalVar(var.symbol);
            HMD_ASSERT(p != NULL);
            if (context->bCompileSetVal) {
                context->addCode<OP_setLocal>(p->idx);
            } else {
                context->addCode<OP_getLocal>(p->idx);
            }
        }
        break;

    default:
        if (context->bCompileSetVal) {
            LocalVarSymID_t lsym = gLocalVarSymbols.localVarSymID(gSymTable.id2str(var.symbol));
            Context::LocalVar_t& lv = context->getLocalVarCreate(lsym);
            context->addCode<OP_setLocal>(lv.idx);
        } else {
            if (compileOption.Wunknown >= 2) {
                compileError(M_M("unknown identifier '%s'"),
                             gSymTable.id2str(var.symbol));
            } else if (compileOption.Wunknown >= 1) {
                outWarning(getCompileErrorPos(),
                           M_M("unknown identifier '%s' will be searched dinamically"),
                           gSymTable.id2str(var.symbol));
            }
            // todo: 後で判明できる場合には命令を差し替える
            context->addCode<OP_getSomething>(var.symbol);
        }
    }
}
        


//============================================================
//@ # メソッド呼び出し または ()演算子
//@ method_or_functor <- IDENT %s argList
void P_method_or_functor::actionAtParse(SyntaxTree*){}
void ST_method_or_functor::compile(Context* context)
{
    PRE_COMPILE_HOOK(method_or_functor);
    get(1)->compile(context);
    hyu16 signatureID = (hyu16) context->tmp_int;
    Var_t var = context->searchVar(gpInp, get(0)->str);
    if (var.type == IT_METHOD || var.type == IT_NONE) {
        // メソッド呼出し
        Context::getPackage()->checkMethod(var.symbol, signatureID, str.startPos);
        context->setSourceInfo(str.endPos);
        switch (signatureID) {
        case 0:
            context->addCode<OP_method_0>(var.symbol);
            break;
        case 1:
            context->addCode<OP_method_1>(var.symbol);
            break;
        case 2:
            context->addCode<OP_method_2>(var.symbol);
            break;
        default:
            context->addCode<OP_method>(signatureID, var.symbol);
        }     

    } else {
        // ()演算子
        context->setSourceInfo(str.endPos);
        context->bCompileSetVal = false;
        compileVar(context, var);
        switch (signatureID) {
        case 0:
            context->addCode<OP_insMethod_0>(HyCSymx_2829);
            break;
        case 1:
            context->addCode<OP_insMethod_1>(HyCSymx_2829);
            break;
        case 2:
            context->addCode<OP_insMethod_2>(HyCSymx_2829);
            break;
        default:
            context->addCode<OP_insMethod>(signatureID, HyCSymx_2829);
        }
    }     
}



//============================================================
// argListのchildをコンパイルしてSignatureを生成(argList自身はコンパイルしない)
static void compile_argList(Context* context, SyntaxTree* argList)
{
    Signature sig;
    int n = argList->numChild();
    for (int i = 0; i < n; ++i) {
        context->tmp_signature = &sig;
        argList->get(i)->compile(context);
    }
    hyu16 sigID = context->getSignatureID(sig.getSigBytes(), sig.getSigBytesLen());
    context->tmp_int = (hys32)sigID;
}

//============================================================
//@ argList1        <- parenArgs lambdaArgs?
void P_argList1::actionAtParse(SyntaxTree*){}
void ST_argList1::compile(Context* context)
{
    PRE_COMPILE_HOOK(argList1);
    compile_argList(context, this);
}

//============================================================
//@ argList         <- argList1 / lambdaArgs
void P_argList::actionAtParse(SyntaxTree*){}
void ST_argList::compile(Context* context)
{
    PRE_COMPILE_HOOK(argList);
    if (get(0)->chooseNum == 0)
        get(0)->get(0)->compile(context);
    else
        compile_argList(context, get(0)); // get(0)->get(0)を渡すと、lambdaArgsを飛び越して子供ノードをコンパイルしてしまうので注意
}

//============================================================
//@ parenArgs       <- %"(" %z (rightValueList %z)? $1 %")"
//@ parenArgs$1 = "')' required"
void P_parenArgs::actionAtParse(SyntaxTree*){}
void ST_parenArgs::compile(Context* context)
{
    PRE_COMPILE_HOOK(parenArgs);
    if (numChild() > 0)
        get(0)->compile(context);
}

//============================================================
//@ lambdaArgs      <- lambda2 (%s lambda2)*
void P_lambdaArgs::actionAtParse(SyntaxTree*){}
void ST_lambdaArgs::compile(Context* context)
{
    PRE_COMPILE_HOOK(lambdaArgs);
    int n = numChild();
    Signature* pSig = context->tmp_signature;
    for (int i = 0; i < n; ++i) {
        get(i)->compile(context);
        pSig->addSig(1);
    }
}



//============================================================
//@ # sayCommand 
static hys32 countLines(SyntaxTree* st)
{
    hys32 lines = 0;
    if (st->parser == p_sayString1 || st->parser == p_sayString2) {
        gpInp->setPos(st->str.startPos);
        hyu32 endPos = st->str.endPos;
        bool escape = false;
        while (gpInp->getPos() < endPos) {
            wchar_t c = gpInp->getChar();
            if (escape) {
                escape = false;
                if (c == L'n') ++lines; // \n 
            } else if (c == L'\\') {
                escape = true;
            } else if (c == L'\n') {
                ++lines;
            }
        }
    } else {
        int n = st->numChild();
        for (int i = 0; i < n; i++) {
            lines += countLines(st->get(i));
        }
    }
    return lines;
}

//@ sayCommand      <- exp0? %s say
void P_sayCommand::actionAtParse(SyntaxTree*){}
void ST_sayCommand::compile(Context* context)
{
    PRE_COMPILE_HOOK(sayCommand);
    SyntaxTree* say;
    if (numChild() > 1) {
        say = get(1);
        get(0)->compile(context);
        context->setSourceInfo(say->str.startPos);
    } else {
        say = get(0);
        context->setSourceInfo(say->str.startPos);
        context->addCode<OP_new>(HyCSym_StringBuffer, 0);
    }
    context->addCode<OP_push_i>(Context::sayCommandIndexCount());
    context->addCode<OP_push_i>(countLines(say) + 1);
    context->addCode<OP_copy_3rd>();    // self
    context->addCode<OP_insMethod_2>(HyCSym_sayCommandStart);
    context->addCode<OP_pop>();

    say->compile(context);

    context->setSourceInfo(str.endPos);
    context->addCode<OP_insMethod_0>(HyCSym_sayCommandEnd);
}

//============================================================
//@ say             <- %"「" sayElement1* %"」" / %"<<" sayElement2* %">>"
void P_say::actionAtParse(SyntaxTree*){}
void ST_say::compile(Context* context)
{
    PRE_COMPILE_HOOK(say);
    SyntaxTree* st = get(0);
    int n = st->numChild();
    for (int i = 0; i < n; i++) {
        st->get(i)->compile(context);
    }
}

//============================================================
//@ sayElement1     <- notSayExp / formatSayExp / sayExp / sayString1
void P_sayElement1::actionAtParse(SyntaxTree*){}
void ST_sayElement1::compile(Context* context)
{
    PRE_COMPILE_HOOK(sayElement1);
    get(0)->get(0)->compile(context);
}

//============================================================
//@ sayElement2     <- notSayExp / formatSayExp / sayExp / sayString2
void P_sayElement2::actionAtParse(SyntaxTree*){}
void ST_sayElement2::compile(Context* context)
{
    PRE_COMPILE_HOOK(sayElement2);
    get(0)->get(0)->compile(context);
}

//============================================================
//@ notSayExp       <- %"{#" %z $1 sayExpElems %z $2 %"}"
//@ notSayExp$1 = "expression or substitute statement required"
//@ notSayExp$2 = "'}' required in embedded sayCommand expression"
void P_notSayExp::actionAtParse(SyntaxTree*){}
void ST_notSayExp::compile(Context* context)
{
    PRE_COMPILE_HOOK(sayExp);
    context->setSourceInfo(str.startPos);
    get(0)->compile(context);
    context->addCode<OP_pop>();
}

//============================================================
//@ formatSayExp    <- %"{%" $1 format %s $2 %"%" %s $3 exp %z $4 %"}"
//@ formatSayExp$1 = "bad format string"
//@ formatSayExp$2 = "a '%' needed after format specifier"
//@ formatSayExp$3 = "expression to be formatted required after '%'"
//@ formatSayExp$4 = "'}' required for formatted sayCommand"
void P_formatSayExp::actionAtParse(SyntaxTree*){}
void ST_formatSayExp::compile(Context* context)
{
    PRE_COMPILE_HOOK(formatSayExp);
    context->setSourceInfo(str.startPos);
    get(0)->compile(context);   // format
    int strOffs = context->tmp_int;
    int n = get(0)->get(0)->chooseNum;
    if (n <= 7)                 // d i o u x X lc c
        n = 0;  // -> int
    else if (n <= 11)           // e E f F
        n = 1;  // -> float
    else                        // ls s
        n = 2;  // -> string
    get(1)->compile(context);   // exp
    context->addCode<OP_sprintf>(strOffs, n);
    context->addCode<OP_copy_2nd>();
    context->addCode<OP_insMethod_1>(HyCSym_sayCommand);
    context->addCode<OP_pop>();
}


//============================================================
//@ format          <- %"#"? %("-" / " " / "+")?
//@                    %[0-9]*
//@                    %("." [0-9]*)?
//@                    ("d" / "i" / "o" / "u" / "x" / "X"
//@                     / "lc" / "c"
//@                     / "e" / "E" / "f" / "F"
//@                     / "ls" / "s")
void P_format::actionAtParse(SyntaxTree*){}
void ST_format::compile(Context* context)
{
    PRE_COMPILE_HOOK(sayExp);
    Substr ss(str.startPos - 1, str.endPos);
    hyu32 len = ss.len();
    char* buf = (char*)HMD_ALLOCA(len + 1);
    HMD_STRNCPY(buf, gpInp->addr(ss.startPos), len+1);
    context->tmp_int = gStrTable.getOffs(buf);
}


//============================================================
//@ sayExp          <- %"{" %z $1 sayExpElems %z $2 %"}"
//@ sayExp$1 = "expression or substitute statement required"
//@ sayExp$2 = "'}' required in embedded sayCommand expression"
void P_sayExp::actionAtParse(SyntaxTree*){}
void ST_sayExp::compile(Context* context)
{
    PRE_COMPILE_HOOK(sayExp);
    context->setSourceInfo(str.startPos);
    get(0)->compile(context);
    context->addCode<OP_copy_2nd>();
    context->addCode<OP_insMethod_1>(HyCSym_sayCommand);
    context->addCode<OP_pop>();
}

//============================================================
//@ sayString1      <% (!"」" %SChar)+
//@ sayString2      <% (!">>" %SChar)+

// 空白文字判定 in hyCuParse.cpp
extern bool hyC_isSpace(wchar_t);

static void outStrconst(Context* context, hyu32 st, hyu32 ed, bool withEol)
{
    Substr ss(st,ed);
    context->setSourceInfo(st);
    hyu32 len = ed - st;
    char* buf = (char*)HMD_ALLOCA(len + 2);
    hys32 clen = MBCSConv::unescape(gpInp->addr(st), len, buf, len+2, withEol);
    if (clen < 0)
        compileError_pos(st, M_M("bad string"));
    hyu32 b2s = clen * 2 + 10;
    char* buf2 = (char*)HMD_ALLOCA(b2s);
    hyu32 tlen = MBCSConv::conv(buf, clen+1, buf2, b2s);
    if (tlen == (hyu32)-1)
        compileError_pos(st, M_M("bad string"));
    context->addCode<OP_push_strconst>(gStrTable.getOffs(buf2, tlen));
    context->addCode<OP_copy_2nd>();
    context->addCode<OP_insMethod_1>(HyCSym_sayCommand);
    context->addCode<OP_pop>();
}
// LFコードの前後の空白文字列を無視しながら、LF区切りでsayStringコンパイル
static void compileSayString(const Substr& ss, Context* context)
{
    gpInp->setPos(ss.startPos);
    hyu32 st, ed, curPos, endPos, spcTop;
    curPos = st = ed = ss.startPos;
    endPos = spcTop = ss.endPos;
    bool skipHeadSpace = false;
    bool escFlag = false;
    while (curPos < endPos) {
        wchar_t c = gpInp->getChar();
        if (hyC_isSpace(c)) {
            ed = gpInp->getPos();
            if (skipHeadSpace)
                st = ed;
            else if (!escFlag && (spcTop > curPos))
                spcTop = curPos;
            escFlag = false;
        } else {
            if (c == L'\\') {
                ed = gpInp->getPos();
                skipHeadSpace = false;
                escFlag = true;
            } else {
                if ((c == L'\n') && !escFlag) {
                    if (spcTop < ed)
                        ed = spcTop;
                    outStrconst(context, st, ed, true);
                    st = ed = gpInp->getPos();
                    skipHeadSpace = true;
                } else {
                    ed = gpInp->getPos();
                    skipHeadSpace = false;
                }
                escFlag = false;
            }
            spcTop = endPos;
        }
        curPos = ed;
    }

    if (st < ed)
        outStrconst(context, st, ed, false);
}
void ST_sayString1::compile(Context* context){ compileSayString(str, context); }
void ST_sayString2::compile(Context* context){ compileSayString(str, context); }

//============================================================
//@ sayExpElems        <- subst_or_exp (%separator subst_or_exp)*
void P_sayExpElems::actionAtParse(SyntaxTree*){}
void ST_sayExpElems::compile(Context* context)
{
    PRE_COMPILE_HOOK(sayExpElems);
    bool b = context->bPushCode;
    compileCompound(this, context);
    context->bPushCode = b;
}

//============================================================
//@ # メソッド定義 
//@ defMethod       <- %"def" %spc $1 methodName_def %s $2 paramList? %z $3 block
//@ defMethod$1 = "method name required"
//@ defMethod$2 = "method parameter required"
//@ defMethod$3 = "method body block required"
void P_defMethod::actionAtParse(SyntaxTree*){}
void ST_defMethod::compile(Context* context)
{
    // 直前の値はそのまま手をつけない
    PRE_COMPILE_HOOK(defMethod);
    Var_t var = context->myIdent(gpInp, get(0)->str);
    switch (var.type) {
    case IT_CONST:
    case IT_CLASS:
    case IT_LOCAL:
        {
            hyu32 bufSize = get(0)->str.len() + 4;
            char* buf = (char*)HMD_ALLOCA(bufSize);
            gpInp->copyStr(buf, bufSize, get(0)->str);
            compileError_pos(get(0),
                             M_M("method name '%s' conflict with %s name"),
                             buf,
                             VAR_IDENT_STR[var.type]);
        }
        break;
    default:
        ;
    }

    hyu16 defValOffs = context->classInfo()->numDefaultVal();
    Context* methodContext = Context::createChild();
    hyu16 sigID;
    SyntaxTree* block;
    if (numChild() > 2) {
        get(1)->compile(methodContext);     // paramList
        sigID = (hyu16) methodContext->tmp_int;
        block = get(2);
    } else {
        methodContext->createParamVar(LocalVarSym_args_);   // *args_
        sigID = Signature::ID_MULTI;
        block = get(1);
    }
    methodContext->setSourceInfo(block->str.startPos);
    block->compile(methodContext);
    context->setSourceInfo(block->str.endPos);
    methodContext->needLastVal();
    methodContext->addCode<OP_return>();
    methodContext->postCompileProcess(defValOffs, sigID);
    Context::pop();
    HMD_DEBUG_ASSERT(context == Context::current());
    get(0)->compile(context); // methodName --> tmp_symbol
    context->addMethod(context->tmp_symbol, methodContext);
}

//============================================================
//@ paramList       <- %"(" %z (params %z)? $1 %")"
//@ paramList$1 = "unclosed parameter list"
void P_paramList::actionAtParse(SyntaxTree*){}
void ST_paramList::compile(Context* context)
{
    PRE_COMPILE_HOOK(paramList);
    Signature sig;
    context->tmp_signature = &sig;
    if (numChild() > 0)
        get(0)->compile(context);
    sig.adjustDefaultValIdx();
    hyu16 sigID = context->getSignatureID(sig.getSigBytes(), sig.getSigBytesLen());
    context->tmp_int = sigID;
}

//============================================================
//@ lambdaParam     <- %"|" %z (params %z)? $1 %"|"
//@ lambdaParam$1 = "unclosed parameter list"
void P_lambdaParam::actionAtParse(SyntaxTree*){}
void ST_lambdaParam::compile(Context* context)
{
    PRE_COMPILE_HOOK(lambdaParam);
    Signature sig;
    context->tmp_signature = &sig;
    if (numChild() > 0)
        get(0)->compile(context);
    else {
        // no params, use '*args_'
        context->createParamVar(LocalVarSym_args_);
        context->tmp_signature->addSig(Signature::MULTI);
    }
    sig.adjustDefaultValIdx();
    hyu16 sigID = context->getSignatureID(sig.getSigBytes(), sig.getSigBytesLen());
    context->tmp_int = sigID;
}

//============================================================
//@ params          <- paramElem (%z %"," %z $1 paramElem)*
//@ params$1 = "parameter required"
void P_params::actionAtParse(SyntaxTree*){}
void ST_params::compile(Context* context)
{
    PRE_COMPILE_HOOK(params);
    hyu32 n = numChild();
    for (hyu32 i = 0; i < n; ++i) {
        try {
            get(i)->compile(context);
        } catch (Signature::IllegalArgException e) {
            compileError_pos(get(i), "%s", e.message);
        }
    }
}



// デフォルト引数コンパイル共通
// context->tmp_* は破壊される可能性あり
static void compileSetDefaultVal(Context* context, SyntaxTree* exp)
{
    context->setSourceInfo(exp->str.startPos);
    hyu16 defValIdx = context->classInfo()->defaultValAdded();
    exp->compile(context);
    context->addCode<OP_setDefaultVal>(defValIdx);
}

//============================================================
//@ paramElem       <- %"'(" %z $1 params? %z %")"
//@                  / IDENT (%s %"=" %s $2 exp)?
//@                  / %"*" %s $3 IDENT (%s %"=" %s $2 exp)?
//@ paramElem$1 = "unclosed parameter list"
//@ paramElem$2 = "default value expression required"
//@ paramElem$3 = "parameter variable name required"
void P_paramElem::actionAtParse(SyntaxTree*){}
void ST_paramElem::compile(Context* context)
{
    PRE_COMPILE_HOOK(paramElem);
    SyntaxTree* ch = get(0);
    switch (ch->chooseNum) {
    case 0:
        // %"'(" $1 %z params? %z %")"
        {
            Signature* sub = context->tmp_signature->openSub();
            if (ch->numChild() > 0) {
                context->tmp_signature = sub;
                ch->get(0)->compile(context);
            }
            context->tmp_signature = sub->close();
        }
        break;

    case 1:
        // IDENT (%s "=" $2 %s exp)?
        context->createParamVar(gLocalVarSymbols.localVarSymID(gpInp, ch->get(0)->str));
        if (ch->numChild() > 1) {
            Context* outerContext = context->outerContext();

            // expの中にlambdaが含まれている時、lambdaのcontext->outerContextを
            // ここのouterContextと同じにするためにpopする
            // 元々Contextのpush,popの設計がよろしくないのでlambdaのコンパイル
            // 時のContext::createChild()との関係でややこしくなっている
            HMD_DEBUG_ASSERT(context == Context::current());
            Context::pop();
            HMD_DEBUG_ASSERT(outerContext == Context::current());
            compileSetDefaultVal(outerContext, ch->get(1));
            Context::push(context);
            context->tmp_signature->addSig(Signature::DEFVAL);
        } else {
            context->tmp_signature->addSig(1);
        }

        break;

    case 2:
        // %"*" %s $3 IDENT (%s "=" $2 %s exp)?
        context->createParamVar(gLocalVarSymbols.localVarSymID(gpInp, ch->get(0)->str));
        if (ch->numChild() > 1) {
            Context* outerContext = context->outerContext();

            // expの中にlambdaが含まれている時、lambdaのcontext->outerContextを
            // ここのouterContextと同じにするためにpopする
            // 元々Contextのpush,popの設計がよろしくないのでlambdaのコンパイル
            // 時のContext::createChild()との関係でややこしくなっている
            HMD_DEBUG_ASSERT(context == Context::current());
            Context::pop();
            HMD_DEBUG_ASSERT(outerContext == Context::current());
            compileSetDefaultVal(outerContext, ch->get(1));
            Context::push(context);
            context->tmp_signature->addSig(Signature::DEFVAL_MULTI);
        } else {
            context->tmp_signature->addSig(Signature::MULTI);
        }
        break;

    default:
        HMD_FATAL_ERROR("compiler bug");
    }
}

//============================================================
//@ block           <- %"{" %z $1 progn %z $2 %"}"
//@ block$1 = "block body statements required"
//@ block$2 = "statement or '}' required"
void P_block::actionAtParse(SyntaxTree*){}
void ST_block::compile(Context* context)
{
    PRE_COMPILE_HOOK(block);
    get(0)->compile(context);       // progn
}

//============================================================
//@ methodName      <- IDENT
void P_methodName::actionAtParse(SyntaxTree*){}
void ST_methodName::compile(Context* context)
{
    PRE_COMPILE_HOOK(methodName);
    get(0)->compile(context);
}

//============================================================
//@ methodName_def  <- IDENT "="? / STRING
void P_methodName_def::actionAtParse(SyntaxTree*){}
void ST_methodName_def::compile(Context* context)
{
    PRE_COMPILE_HOOK(methodName_def);
    SyntaxTree* choice = get(0);
    if (choice->chooseNum == 1) {
        // STRING
        Substr ss = choice->get(0)->str;
        ss.startPos ++; ss.endPos --;   // delete '"'
        context->tmp_symbol = gSymTable.symbolID(gpInp, ss);
    } else {
        // IDENT "="?
        context->tmp_symbol = gSymTable.symbolID(gpInp, str);
    }
}

//============================================================
//@ # クロージャ
static void compileLambdaBody(Context* context, hyu16 defValOffs, hyu16 signatureID, SyntaxTree* block)
{
    block->compile(context);
    context->setSourceInfo(block->str.endPos);
    context->needLastVal();
    context->addCode<OP_return>();
    context->postCompileProcess(defValOffs, signatureID);
    Context* outerContext = context->outerContext();
    outerContext->addClosure(outerContext, context);
}

//============================================================
//@ lambda          <- %"lambda" %s paramList? %z $1 block
//@ lambda$1 = "lambda body block required"
void P_lambda::actionAtParse(SyntaxTree*){}
void ST_lambda::compile(Context* context)
{
    PRE_COMPILE_HOOK(lambda);
    hyu16 defValOffs = context->classInfo()->numDefaultVal();
    Context* closureContext = Context::createChild();
    closureContext->setClosureContext();
    if (numChild() < 2) {
        // no paramList, use '*args_'
        closureContext->createParamVar(LocalVarSym_args_);
        compileLambdaBody(closureContext, defValOffs, Signature::ID_MULTI, get(0)); // block
    } else {
        get(0)->compile(closureContext); // paramList
        hyu16 sigID = (hyu16) closureContext->tmp_int;
        compileLambdaBody(closureContext, defValOffs, sigID, get(1)); // block
    }
    Context::pop();
    // デフォルト値としてlambdaを与える場合は以下は成り立たない
    // HMD_DEBUG_ASSERT(context == Context::current());
}

//============================================================
//@ lambda2         <- %"{" %z lambdaParam? %z $1 progn %z $2 %"}"
//@ lambda2$1 = "lambda body block required"
//@ lambda2$2 = "statement or '}' required for lambda block"
void P_lambda2::actionAtParse(SyntaxTree*){}
void ST_lambda2::compile(Context* context)
{
    PRE_COMPILE_HOOK(lambda2);
    hyu16 defValOffs = context->classInfo()->numDefaultVal();
    Context* closureContext = Context::createChild();
    closureContext->setClosureContext();
    if (numChild() < 2) {
        // no lambdaParam, use '*args_'
        closureContext->createParamVar(LocalVarSym_args_);
        compileLambdaBody(closureContext, defValOffs, Signature::ID_MULTI, get(0)); // progn
    } else {
        get(0)->compile(closureContext); // lambdaParam
        hyu16 sigID = (hyu16) closureContext->tmp_int;
        compileLambdaBody(closureContext, defValOffs, sigID, get(1)); // progn
    }
    Context::pop();
    HMD_DEBUG_ASSERT(context == Context::current());
}

//============================================================
//@ # ファイバー定義
//@ defFiber        <- %'defFiber' %spc $1 methodName %s $2 paramList? %z $3 block
//@ defFiber$1 = "method name required"
//@ defFiber$2 = "method parameter required"
//@ defFiber$3 = "method body block required"
void P_defFiber::actionAtParse(SyntaxTree*){}
void ST_defFiber::compile(Context* context)
{
    // 直前の値はそのまま手をつけない
    PRE_COMPILE_HOOK(defFiber);
    Var_t var = context->myIdent(gpInp, get(0)->str);
    switch (var.type) {
    case IT_CONST:
    case IT_CLASS:
    case IT_LOCAL:
        {
            hyu32 bufSize = get(0)->str.len() + 4;
            char* buf = (char*)HMD_ALLOCA(bufSize);
            gpInp->copyStr(buf, bufSize, get(0)->str);
            compileError_pos(get(0),
                             M_M("fiber name '%s' conflict with %s name"),
                             buf,
                             VAR_IDENT_STR[var.type]);
        }
        break;
    default:
        ;
    }

    hyu16 defValOffs = context->classInfo()->numDefaultVal();
    Context* methodContext = Context::createChild();
    hyu16 sigID;
    SyntaxTree* block;
    if (numChild() > 2) {
        get(1)->compile(methodContext);     // paramList
        sigID = (hyu16) methodContext->tmp_int;
        block = get(2);
    } else {
        methodContext->createParamVar(LocalVarSym_args_);   // *args_
        sigID = Signature::ID_MULTI;
        block = get(1);
    }
    methodContext->setSourceInfo(block->str.startPos);
    methodContext->addCode<OP_beFiber>();
    block->compile(methodContext);
    methodContext->setSourceInfo(str.endPos);
    methodContext->needLastVal();
    methodContext->addCode<OP_return>();
    methodContext->postCompileProcess(defValOffs, sigID);
    Context::pop();
    HMD_DEBUG_ASSERT(context == Context::current());
    get(0)->compile(context); //methodName --> tmp_symbol
    context->addMethod(context->tmp_symbol, methodContext);
}

//============================================================
//@ # Foreign Function Interface 
//@ ffi         <- %"c++" $1 (%spc cppClassName)? %z %"{" %z $2 (cppInclude %z)? (ffiDecl %z)+ $3 %"}"
//@ ffi$1 = "only C++ class name can be written after 'c++' keyword"
//@ ffi$2 = "'includeFile' or ffi definition required"
//@ ffi$3 = "'}' required"
void P_ffi::actionAtParse(SyntaxTree*){}
void ST_ffi::compile(Context* context)
{
    // 直前の値はそのまま手をつけない
    PRE_COMPILE_HOOK(ffi);
    int n = numChild();
    for (int i = 0; i < n; i++)
        get(i)->compile(context);
}

//============================================================
//@ cppClassName    <- %IDENT_cpp (%"::" %IDENT_cpp)*
void P_cppClassName::actionAtParse(SyntaxTree*){}
void ST_cppClassName::compile(Context* context)
{
    PRE_COMPILE_HOOK(cppClassName);
    context->classInfo()->addCppClass(str);
}

//============================================================
//@ cppInclude      <- %"includeFile" %s STRING
void P_cppInclude::actionAtParse(SyntaxTree*){}
void ST_cppInclude::compile(Context* context)
{
    PRE_COMPILE_HOOK(cppInclude);
    Substr ss = get(0)->str;
    ++ ss.startPos;
    -- ss.endPos;
    context->classInfo()->addCppInclude(ss);
}

//============================================================
//@ ffiDecl     <- methodName_def %s $1 ffiSigDecl
//@ ffiDecl$1 = "ffi parameter definition required"
void P_ffiDecl::actionAtParse(SyntaxTree*){}
void ST_ffiDecl::compile(Context* context)
{
    PRE_COMPILE_HOOK(ffiDecl);
    Var_t var = context->myIdent(gpInp, get(0)->str);
    switch (var.type) {
    case IT_CONST:
    case IT_CLASS:
    case IT_LOCAL:
        {
            hyu32 bufSize = get(0)->str.len() + 4;
            char* buf = (char*)HMD_ALLOCA(bufSize);
            gpInp->copyStr(buf, bufSize, get(0)->str);
            compileError_pos(get(0),
                             M_M("ffi method name '%s' conflict with %s name"),
                             buf,
                             VAR_IDENT_STR[var.type]);
        }
        break;
    default:
        ;
    }

    get(0)->compile(context);       // methodName_def;
    SymbolID_t methodName = context->tmp_symbol;
    get(1)->compile(context);       // ffiSigDecl
    context->classInfo()->addFfi(methodName, context->tmp_ffiSigDecl);
}

//============================================================
//@ ffiSigDecl  <- argDecls (%s %":" $1 IDENT)?
//@ ffiSigDecl$1 = "type name required"
void P_ffiSigDecl::actionAtParse(SyntaxTree*){}
void ST_ffiSigDecl::compile(Context* context)
{
    PRE_COMPILE_HOOK(ffiSigDecl);
    FfiSigDecl_t ffiSigDecl;
    ffiSigDecl.defValOffs = context->classInfo()->numDefaultVal();
    get(0)->compile(context);
    TArray<ArgD_t>* argDecls = (TArray<ArgD_t>*) context->tmp_ptr;
    ffiSigDecl.argDecls = argDecls;
    if (numChild() > 1) {
        ffiSigDecl.retType = context->classInfo()->nameTableEntry(get(1)->str);
    } else
        ffiSigDecl.retType = NULL_STR_OFFS;
    Signature sig;
    hyu16 defValIdx = 0;
    if (argDecls != NULL) {
        hyu32 n = argDecls->size();
        for (hyu32 i = 0; i < n; ++i) {
            if (argDecls->nth(i).haveDefaultVal) {
                sig.addSig(Signature::defaultValIdx2sig(defValIdx++));
            } else
                sig.addSig(1);
        }
    } else {
        sig.addSig(Signature::MULTI2);
    }
    ffiSigDecl.signatureID = context->getSignatureID(sig.getSigBytes(), sig.getSigBytesLen());
    context->tmp_ffiSigDecl = ffiSigDecl;
}

//============================================================
//@ argDecls        <- %"(" %z $1 (argDeclList %z / "..." %z $2 &")")? %")"
//@ argDecls$1 = "ffi parameter required"
//@ argDecls$2 = "no parameter can be after '...'"
void P_argDecls::actionAtParse(SyntaxTree*){}
void ST_argDecls::compile(Context* context)
{
    PRE_COMPILE_HOOK(argDecls);
    if (numChild() > 0) {
        if (get(0)->chooseNum == 0) {
            // argDeclList
            get(0)->get(0)->compile(context);
        } else {
            // ...
            context->tmp_ptr = NULL;
        }
    } else {
        // no arg
        TArray<ArgD_t>* p = new TArray<ArgD_t>(1); // 中身空のまま
        p->setContentsMemID("ArgD");
        context->tmp_ptr = p;
    }
}

//============================================================
//@ argDeclList     <- argDeclElem (%z %"," %z argDeclElem)*
void P_argDeclList::actionAtParse(SyntaxTree*){}
void ST_argDeclList::compile(Context* context)
{
    PRE_COMPILE_HOOK(argDeclList);
    int n = numChild();
    TArray<ArgD_t>* lst = new TArray<ArgD_t>(n);
    lst->setContentsMemID("ArgD");
    for (int i = 0; i < n; i++) {
        get(i)->compile(context);
        lst->subst(i, context->tmp_argD, context->tmp_argD);
    }        
    context->tmp_ptr = lst;
}

//============================================================
//@ argDeclElem     <- argD (%s %"=" %s $1 exp)?
//@ argDeclElem$1 = "default value expression required"
void P_argDeclElem::actionAtParse(SyntaxTree*){}
void ST_argDeclElem::compile(Context* context)
{
    PRE_COMPILE_HOOK(argDeclElem);
    get(0)->compile(context);
    if (numChild() > 1) {
        ArgD_t saveArgD = context->tmp_argD;
        compileSetDefaultVal(context, get(1));
        saveArgD.haveDefaultVal = true;
        context->tmp_argD = saveArgD;
    } else {
        context->tmp_argD.haveDefaultVal = false;
    }
}


//============================================================
//@ argD            <- IDENT (%":" $1 IDENT)?
//@ argD$1 = "class name required after ':'"
void P_argD::actionAtParse(SyntaxTree*){}
void ST_argD::compile(Context* context)
{
    PRE_COMPILE_HOOK(argD);
    ArgD_t argD;
    argD.argName = context->classInfo()->nameTableEntry(get(0)->str);
    if (numChild() > 1)
        argD.typeName = context->classInfo()->nameTableEntry(get(1)->str);
    else
        argD.typeName = NULL_STR_OFFS;
    context->tmp_argD = argD;
}


//@ # 制御構造 

//============================================================
//@ oneStmt         <- block / statement_1
void P_oneStmt::actionAtParse(SyntaxTree*){}
void ST_oneStmt::compile(Context* context)
{
    PRE_COMPILE_HOOK(oneStmt);
    get(0)->get(0)->compile(context);
}

//============================================================
//@ condExp         <- block / exp
void P_condExp::actionAtParse(SyntaxTree*){}
void ST_condExp::compile(Context* context)
{
    PRE_COMPILE_HOOK(condExp);
    HMD_DEBUG_ASSERT(! context->bPushCode);
    if (get(0)->chooseNum == 0) {
        get(0)->get(0)->compile(context);
        context->needLastVal();
    } else {
        get(0)->get(0)->compile(context);
        context->bPushCode = true;
    }
}

//============================================================
//@ forInitExp      <- block / subst_or_exp? %s $1 &(";" / ")" / "\n")
//@ forInitExp$1 = "initialize expression is not terminated"
void P_forInitExp::actionAtParse(SyntaxTree*){}
void ST_forInitExp::compile(Context* context)
{
    PRE_COMPILE_HOOK(forInitExp);
    SyntaxTree* ch = get(0);
    if (ch->numChild() > 0)
        ch->get(0)->compile(context);
}


//============================================================
//@ if              <- %'if' %s $1 %"(" %z condExp %z %")" %z $2 oneStmt %s $3 &("\n" / ";" / 'else'/ "}" / EOF) else?
//@ if$1 = "'(conditional expression)' required after 'if'"
//@ if$2 = "1 statement or block required"
//@ if$3 = "'else' or ';' required after 'if' statement"
void P_if::actionAtParse(SyntaxTree*){}
void ST_if::compile(Context* context)
{
    PRE_COMPILE_HOOK(if);
    context->popLastVal();
    get(0)->compile(context);   // condExp
    hyu32 resolveAddrToElse = context->addGotoCode<OP_jump_ifFalse>();
    context->bPushCode = false;
    get(1)->compile(context);   // oneStmt
    if (numChild() > 2) {
        // else
        context->needLastVal();
        hyu32 resolveAddrToEnd = context->addGotoCode<OP_jump>();
        context->resolveJumpAddr(resolveAddrToElse);
        context->bPushCode = false;
        get(2)->compile(context);
        context->needLastVal();
        context->resolveJumpAddr(resolveAddrToEnd);
    } else {
        // no else
        if (context->bPushCode) {
            hyu32 resolveAddrToEnd = context->addGotoCode<OP_jump>();
            context->resolveJumpAddr(resolveAddrToElse);
            context->addCode<OP_push_nil>();
            context->resolveJumpAddr(resolveAddrToEnd);
        } else {
            context->resolveJumpAddr(resolveAddrToElse);
        }
    }
}

//============================================================
//@ else            <- %separator? %'else' %z $1 oneStmt
//@ else$1 = "1 statement or block required"
void P_else::actionAtParse(SyntaxTree*){}
void ST_else::compile(Context* context)
{
    PRE_COMPILE_HOOK(else);
    get(0)->compile(context);
}

//============================================================
//@ loop            <- %'loop' %s $1 oneStmt
//@ loop$1 = "1 statement or block required"
void P_loop::actionAtParse(SyntaxTree*){}
void ST_loop::compile(Context* context)
{
    PRE_COMPILE_HOOK(loop);
    context->setSourceInfo(str.startPos);
    context->popLastVal();
    context->jumpControlStart();
    context->addLabel(HyCSymS_jumpControlStart);
    get(0)->compile(context);   // oneStmt
    context->needLastVal();
    context->addLabel(HyCSymS_next);
    context->popLastVal();
    context->setSourceInfo(str.endPos);
    context->addGotoCode<OP_jump>(HyCSymS_jumpControlStart);
    context->addLabel(HyCSymS_break);
    context->addLabel(HyCSymS_jumpControlEnd);
    context->jumpControlEnd();
    context->bPushCode = true; // breakには値がある
}

//============================================================
//@ while           <- %'while' %s $1 %"(" %z condExp %z %")" %z $2 oneStmt
//@ while$1 = "'(conditional expression)' required after 'while'"
//@ while$2 = "1 statement or block required"
void P_while::actionAtParse(SyntaxTree*){}
void ST_while::compile(Context* context)
{
    PRE_COMPILE_HOOK(while);
    context->setSourceInfo(str.startPos);
    context->popLastVal();
    context->jumpControlStart();
    context->addLabel(HyCSymS_jumpControlStart);
    context->setSourceInfo(get(0)->str.startPos);
    get(0)->compile(context);   // condExp
    hyu32 resolveAddrToExitLoop = context->addGotoCode<OP_jump_ifFalse>();
    context->bPushCode = false; // jump_ifFalseで消費

    get(1)->compile(context);   // oneStmt
    context->needLastVal();
    context->addLabel(HyCSymS_next);
    context->popLastVal();
    context->setSourceInfo(str.endPos);
    context->addGotoCode<OP_jump>(HyCSymS_jumpControlStart);
    context->resolveJumpAddr(resolveAddrToExitLoop);
    context->addCode<OP_push_nil>();
    context->addLabel(HyCSymS_break);
    context->addLabel(HyCSymS_jumpControlEnd);
    context->jumpControlEnd();
    context->bPushCode = true;
}

//============================================================
//@ for             <- %'for' %s $1 %"(" %z forInitExp %z $2 %";" %z $3 condExp %z $2 %";" %z forInitExp %z $4 %")" %z $5 oneStmt
//@ for$1 = "'(' required after 'for'"
//@ for$2 = "';' required"
//@ for$3 = "conditional expression required"
//@ for$4 = "')' required"
//@ for$5 = "1 statement or block required"
void P_for::actionAtParse(SyntaxTree*){}
void ST_for::compile(Context* context)
{
    PRE_COMPILE_HOOK(for);
    context->setSourceInfo(str.startPos);
    context->popLastVal();
    context->setSourceInfo(get(0)->str.startPos);
    get(0)->compile(context);   // 初期化式
    context->setSourceInfo(get(0)->str.endPos);
    context->popLastVal();

    context->jumpControlStart();
    context->addLabel(HyCSymS_jumpControlStart);

    context->setSourceInfo(get(1)->str.startPos);
    get(1)->compile(context);   // 条件式
    context->setSourceInfo(get(1)->str.endPos);
    hyu32 resolveAddrToExitLoop = context->addGotoCode<OP_jump_ifFalse>();
    context->bPushCode = false; // jump_ifFalseで消費

    context->setSourceInfo(get(3)->str.startPos);
    get(3)->compile(context);   // 本体
    context->setSourceInfo(get(3)->str.endPos);

    context->needLastVal();
    context->addLabel(HyCSymS_next);
    context->popLastVal();

    context->setSourceInfo(get(2)->str.startPos);
    get(2)->compile(context);   // 再初期化式
    context->setSourceInfo(get(2)->str.endPos);
    context->popLastVal();

    context->addGotoCode<OP_jump>(HyCSymS_jumpControlStart);

    context->resolveJumpAddr(resolveAddrToExitLoop);
    context->addCode<OP_push_nil>();
    context->addLabel(HyCSymS_break);
    context->addLabel(HyCSymS_jumpControlEnd);
    context->jumpControlEnd();
    context->bPushCode = true;
}


//============================================================
//@ break           <- %'break'
void P_break::actionAtParse(SyntaxTree*){}
void ST_break::compile(Context* context)
{
    PRE_COMPILE_HOOK(break);
    context->setSourceInfo(str.endPos);
    context->needLastVal();
    context->addGotoCode<OP_jump>(HyCSymS_break);
    context->bPushCode = false;
}

//============================================================
//@ next            <- %'next'
void P_next::actionAtParse(SyntaxTree*){}
void ST_next::compile(Context* context)
{
    PRE_COMPILE_HOOK(next);
    context->setSourceInfo(str.endPos);
    context->needLastVal();
    context->addGotoCode<OP_jump>(HyCSymS_next);
    context->bPushCode = false;
}

//============================================================
//@ wait            <- %'wait' (%s %"(" %z exp %z %")")?
void P_wait::actionAtParse(SyntaxTree*){}
void ST_wait::compile(Context* context)
{
    PRE_COMPILE_HOOK(wait);
    if (numChild() > 0) {
        get(0)->compile(context);
        context->setSourceInfo(str.endPos);
        context->addCode<OP_waitTicks>();
    } else {
        context->setSourceInfo(str.endPos);
        context->addCode<OP_waitTick_1>();
    }
}

//============================================================
//@ label           <- %'label' %s $1 SYMBOL
//@ label$1 = "SYMBOL required"
void P_label::actionAtParse(SyntaxTree*){}
void ST_label::compile(Context* context)
{
    PRE_COMPILE_HOOK(label);
    context->needLastVal();
    Substr ss = get(0)->str;
    ++ ss.startPos;
    context->addLabel(gSymTable.symbolID(gpInp, ss));
}

//============================================================
//@ goto            <- %'goto' %s $1 (SYMBOL / exp)
//@ goto$1 = "a SYMBOL or a expression that returns SYMBOL required after 'goto'"
void P_goto::actionAtParse(SyntaxTree*){}
void ST_goto::compile(Context* context)
{
    PRE_COMPILE_HOOK(goto);
    context->needLastVal();
    if (get(0)->chooseNum == 0) {
        get(0)->get(0)->compile(context);       // SYMBOL
        context->addGotoCode<OP_jump>(context->tmp_symbol);
    } else {
        get(0)->get(0)->compile(context);       // exp
        context->addCode<OP_goto>();
    }
    context->bPushCode = false;
}

//============================================================
//@ return          <- %'return' %s exp?
void P_return::actionAtParse(SyntaxTree*){}
void ST_return::compile(Context* context)
{
    PRE_COMPILE_HOOK(return);
    context->popLastVal();
    if (numChild() > 0)
        get(0)->compile(context);
    else
        context->addCode<OP_push_nil>();
    context->setSourceInfo(str.endPos);
    if (context->isClosureContext())
        context->addCode<OP_returnAway>();
    else
        context->addCode<OP_return>();
    context->bPushCode = false;
}

//============================================================
//@ yield           <- %'yield' %s $1 exp
//@ yield$1 = "yield value required"
void P_yield::actionAtParse(SyntaxTree*){}
void ST_yield::compile(Context* context)
{
    PRE_COMPILE_HOOK(yield);
    get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    context->addCode<OP_yield>();
}



//============================================================
//@ try             <- %'try' %z $1 block catch? finally?
//@ try$1 = "block required for 'try'"
void P_try::actionAtParse(SyntaxTree*){}
void ST_try::compile(Context* context)
{
    PRE_COMPILE_HOOK(try);
    context->popLastVal();

    if (numChild() <= 1) {
        compileError_pos(str.endPos,
                         M_M("'try' has neither 'catch' nor 'finally'"));
    }

    SyntaxTree* catchST;
    if (get(1)->parser == p_catch)
        catchST = get(1);
    else 
        catchST = NULL;

    SyntaxTree* finallyST;
    if (numChild() > 2 && get(2)->parser == p_finally)
        finallyST = get(2);
    else if (get(1)->parser == p_finally)
        finallyST = get(1);
    else
        finallyST = NULL;
    
    HMD_DEBUG_ASSERT(catchST != NULL || finallyST != NULL);

    LocalVarSymID_t catchVar;
    if (catchST != NULL && catchST->numChild() > 1)
        catchVar = gLocalVarSymbols.localVarSymID(gpInp, catchST->get(0)->str); // IDENT
    else
        catchVar = SYMBOL_ID_ERROR;

    LocalVarSymID_t finallyValVar;
    if (finallyST != NULL) {
        finallyValVar = context->useFinallyValVar();
        if (finallyValVar == SYMBOL_ID_ERROR) {
            compileError_pos(finallyST, M_M("finally nesting level must less than %d"), MAX_FINALLY_NEST);
        }
    } else
        finallyValVar = SYMBOL_ID_ERROR;


    context->setSourceInfo(str.startPos);
    context->jumpControlStart(catchVar, finallyValVar);
    get(0)->compile(context);   // try block
    context->needLastVal();
    if (finallyST != NULL) {
        context->addCode<OP_jump_finally>();
        context->bPushCode = false;
    } else {
        context->addGotoCode<OP_jump>(HyCSymS_jumpControlEnd);
    }
    if (catchST != NULL) {
        context->addLabel(HyCSymS_catch);
        context->bPushCode = false;
        catchST->compile(context);
        context->needLastVal();
        context->setSourceInfo(catchST->str.endPos);
        if (finallyST != NULL) {
            context->addCode<OP_jump_finally>();
            context->bPushCode = false;
        } else {
            context->addGotoCode<OP_jump>(HyCSymS_jumpControlEnd);
        }
    }
    if (finallyST != NULL) {
        HMD_DEBUG_ASSERT(context->bPushCode == false);
        context->addLabel(HyCSymS_finally);
        finallyST->compile(context);
        context->needLastVal();
        context->endUseFinallyValVar();
    }
    context->addLabel(HyCSymS_jumpControlEnd);
    context->setSourceInfo(str.endPos);
    context->jumpControlEnd();
}

//============================================================
//@ catch           <- %z %'catch' (%s %"(" %z $1 IDENT %z %")")? %z $2 block
//@ catch$1 = "catch variable required"
//@ catch$2 = "block required for 'catch'"
void P_catch::actionAtParse(SyntaxTree*){}
void ST_catch::compile(Context* context)
{
    PRE_COMPILE_HOOK(catch);
    SyntaxTree* block;
    if (numChild() > 1)
        block = get(1);
    else
        block = get(0);
    context->setSourceInfo(block->str.startPos);
    block->compile(context);
}


//============================================================
//@ finally         <- %z %'finally' %z $1 block
//@ finally$1 = "block required for 'finally'"
void P_finally::actionAtParse(SyntaxTree*){}
void ST_finally::compile(Context* context)
{
    PRE_COMPILE_HOOK(finally);
    context->setSourceInfo(get(0)->str.startPos);
    get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    context->addCode<OP_end_finally>();
}

//============================================================
//@ throw           <- %'throw' %s $1 exp (%s %"," %z exp)?
//@ throw$1 = "throw value required"
void P_throw::actionAtParse(SyntaxTree*){}
void ST_throw::compile(Context* context)
{
    PRE_COMPILE_HOOK(throw);
    context->popLastVal();

    if (numChild() > 1) {
        get(0)->compile(context);
        get(1)->compile(context);
    } else {
        context->addCode<OP_push_symbol>(HyCSym_exception);
        get(0)->compile(context);
    }
    context->addCode<OP_throw>();
    context->bPushCode = false;
}


//@ # 式 

//============================================================
//@ exp0            <- %"(" %z exp %z %")"
//@                  / true / false / nil / self
//@                  / FLOAT / INTEGER / STRING / symbol
//@                  / lambda / yield / wait / scopeMethodCall / method_or_functor
//@                  / scopedVal / Var / membVar / classVar / globalVar
//@                  / arrayLiteral / hashLiteral / listLiteral
void P_exp0::actionAtParse(SyntaxTree*){}
void ST_exp0::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp0);
#ifdef HMD_DEBUG
    bool b = context->bPushCode;
#endif
    context->bCompileSetVal = false;
    get(0)->get(0)->compile(context);
#ifdef HMD_DEBUG
    HMD_DEBUG_ASSERT(b == context->bPushCode);
#endif
}

//============================================================
//@ exp             <- newObj
//@                  / sayCommand
//@                  / exp0

//expanded as below:
// exp <- exp__10
// exp__0 <- newObj / sayCommand / exp0
void P_exp::actionAtParse(SyntaxTree*){}
void ST_exp::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp);
    get(0)->compile(context);
}
void P_exp__0::actionAtParse(SyntaxTree*){}
void ST_exp__0::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__0);
    get(0)->get(0)->compile(context);
}



//============================================================
// and, or, &&, ||  演算子ショートカット共通
template <typename JumpOP> void compile_shortcut(Context* context, SyntaxTree* st)
{
    st->get(0)->compile(context);
    context->setSourceInfo(st->get(1)->str.startPos);
    context->addCode<OP_copy_top>();
    hyu32 resolveAddrToEnd = context->addGotoCode<JumpOP>();
    context->addCode<OP_pop>();
    st->get(2)->compile(context);
    context->resolveJumpAddr(resolveAddrToEnd);
    context->setSourceInfo(st->str.endPos);
}
inline void compile_shortcut_or(Context* context, SyntaxTree* st)
{
    compile_shortcut<OP_jump_ifTrue>(context, st);
}
inline void compile_shortcut_and(Context* context, SyntaxTree* st)
{
    compile_shortcut<OP_jump_ifFalse>(context, st);
}

//============================================================
//@ exp << infixl 10 %s 'or' %z
void ST_exp__10::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__10);
    compile_shortcut_or(context, this);
}

//============================================================
//@ exp << infixl 20 %s 'and' %z
void ST_exp__20::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__20);
    compile_shortcut_and(context, this);
}

//============================================================
//@ exp << prefix 30 'not' %z
void ST_exp__30::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__30);
    get(1)->compile(context);
    context->addCode<OP_insMethod_0>(HyCSymx_21); // !
}

//============================================================
//@ exp << ternary 50 %(s "?" z) %(z ":" z)
void ST_exp__50::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__50);
    get(0)->compile(context);   // 第1項
    hyu32 resolveAddrTo3T = context->addGotoCode<OP_jump_ifFalse>();
    get(1)->compile(context);   // 第2項
    hyu32 resolveAddrToEnd = context->addGotoCode<OP_jump>();
    context->resolveJumpAddr(resolveAddrTo3T);
    get(2)->compile(context);   // 第3項
    context->resolveJumpAddr(resolveAddrToEnd);
}

//============================================================
//@ exp << infixn 60 %s ("..<" / "..") %z
void ST_exp__60::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__60);
    get(2)->compile(context);
    get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    if (get(1)->chooseNum == 0)
        context->addCode<OP_insMethod_1>(HyCSymx_2e2e3c); // ..<
    else
        context->addCode<OP_insMethod_1>(HyCSymx_2e2e); // ..
}

//============================================================
//@ exp << infixl 100 %s ("||" !"=") %z
void ST_exp__100::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__100);
    compile_shortcut_or(context, this);
}

//============================================================
//@ exp << infixl 110 %s ("&&" !"=") %z
void ST_exp__110::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__110);
    compile_shortcut_and(context, this);
}

//============================================================
//@ exp << infixn 200 %s ("<=>" / "===" / "==" / "!=" / "<>" / "<=" / ">=" / "<" / ">") %z
void ST_exp__200::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__200);
    static SymbolID_t opsym[] = {
        HyCSymx_3c3d3e,         //"<=>"
        HyCSymx_3d3d3d,         //"==="
        HyCSymx_3d3d,           //"=="
        HyCSymx_213d,           //"!=",
        HyCSymx_3c3e,           //"<>"
        HyCSymx_3c3d,           //"<="
        HyCSymx_3e3d,           //">="
        HyCSymx_3c,             //"<"
        HyCSymx_3e,             //">"
    };
    get(2)->compile(context);
    get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    HMD_DEBUG_ASSERT(get(1)->chooseNum < (int)NUMELEMS(opsym));
    context->addCode<OP_insMethod_1>(opsym[get(1)->chooseNum]);
}

//============================================================
//@ exp << infixl 310 %s ("|" !("|" / "=") / "^" !"=") %z
void ST_exp__310::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__310);
    get(2)->compile(context);
    get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    if (get(1)->chooseNum == 0)
        context->addCode<OP_insMethod_1>(HyCSymx_7c); // |
    else
        context->addCode<OP_insMethod_1>(HyCSymx_5e); // ^
}

//============================================================
//@ exp << infixl 320 %s ("&" !("&" / "=")) %z
void ST_exp__320::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__320);
    get(2)->compile(context);
    get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    context->addCode<OP_insMethod_1>(HyCSymx_26); // &
}

//============================================================
//@ exp << prefix 330 "~" %s
void ST_exp__330::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__330);
    get(1)->compile(context);
    context->addCode<OP_insMethod_0>(HyCSymx_7e); // ~
}


//============================================================
//@ exp << infixl 350 %s ("+" !("+" / "=") / "-" !("-" / "=")) %z
void ST_exp__350::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__350);
    static const SymbolID_t opsym[] = {
        HyCSymx_2b, //"+"
        HyCSymx_2d, //"-"
    };
    get(2)->compile(context);
    get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    HMD_DEBUG_ASSERT(get(1)->chooseNum < (int)NUMELEMS(opsym));
    context->addCode<OP_insMethod_1>(opsym[get(1)->chooseNum]);
}

//============================================================
//@ exp << infixl 360 %s ("*" !("*" / "=") / "/" !("/" / "*" / "=") / "%" !"=") %z
void ST_exp__360::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__360);
    static const SymbolID_t opsym[] = {
        HyCSymx_2a, //"*"
        HyCSymx_2f, //"/"
        HyCSymx_25, //"%"
    };
    get(2)->compile(context);
    get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    HMD_DEBUG_ASSERT(get(1)->chooseNum < (int)NUMELEMS(opsym));
    context->addCode<OP_insMethod_1>(opsym[get(1)->chooseNum]);
}

//============================================================
//@ exp << infixr 370 %s "**" %z
void ST_exp__370::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__370);
    get(2)->compile(context);
    get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    context->addCode<OP_insMethod_1>(HyCSymx_2a2a);
}

//============================================================
//@ exp << prefix 400 (("+" !("+" / "=") / "-" !("-" / "=")) / "!" !"=") %s
void ST_exp__400::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__400);
    static const SymbolID_t opsym[] = {
        HyCSymx_2b40, //"+@"
        HyCSymx_2d40, //"-@"
        HyCSymx_21, //"!"
    };
    get(1)->compile(context);
    HMD_DEBUG_ASSERT(get(0)->chooseNum < (int)NUMELEMS(opsym));
    context->addCode<OP_insMethod_0>(opsym[get(0)->chooseNum]);
}

//============================================================
//@ exp << postfix 500 %s %"->" %z (lambda2 / lambda)
void ST_exp__500::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__500);
    get(1)->get(0)->compile(context);
    get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    context->addCode<OP_insMethod_1>(HyCSymx_2d3e); // ->
}

//============================================================
//@ exp << postfix 600 (array / methodCall / getMember / functor)
void ST_exp__600::compile(Context* context)
{
    PRE_COMPILE_HOOK(exp__600);
    context->tmp_ptr = get(0);
    get(1)->get(0)->compile(context);
}


//============================================================
//@ array           <- %s %"[" %z $1 exp %z $2 %"]"
//@ array$1 = "array index required"
//@ array$2 = "']' required for array index end"
void P_array::actionAtParse(SyntaxTree*){}
void ST_array::compile(Context* context)
{
    PRE_COMPILE_HOOK(array);
    SyntaxTree* exp = (SyntaxTree*)context->tmp_ptr;
    context->setSourceInfo(get(0)->str.startPos);
    get(0)->compile(context);   // [ ] 内の exp
    context->setSourceInfo(exp->str.startPos);
    context->bCompileSetVal = false;
    exp->compile(context);
    context->setSourceInfo(str.endPos);
    context->addCode<OP_insMethod_1>(HyCSymx_5b5d); // []
}

//============================================================
//@ methodCall      <- %"." IDENT %s argList
void P_methodCall::actionAtParse(SyntaxTree*){}
void ST_methodCall::compile(Context* context)
{
    PRE_COMPILE_HOOK(methodCall);
    SyntaxTree* exp = (SyntaxTree*)context->tmp_ptr;
    hyu16 signatureID;
    get(1)->compile(context);   // argList
    signatureID = (hyu16) context->tmp_int;
    get(0)->compile(context);   // IDENT
    SymbolID_t methodSym = context->tmp_symbol;
    Context::getPackage()->checkMethod(methodSym, signatureID, str.startPos);
    context->bCompileSetVal = false;
    exp->compile(context);
    context->setSourceInfo(str.endPos);
    switch (signatureID) {
    case 0:
        context->addCode<OP_insMethod_0>(methodSym);
        break;
    case 1:
        context->addCode<OP_insMethod_1>(methodSym);
        break;
    case 2:
        context->addCode<OP_insMethod_2>(methodSym);
        break;
    default:
        context->addCode<OP_insMethod>(signatureID, methodSym);
    }     
}


//============================================================
//@ getMember       <- %"." IDENT
void P_getMember::actionAtParse(SyntaxTree*){}
void ST_getMember::compile(Context* context)
{
    PRE_COMPILE_HOOK(getMember);
    SyntaxTree* exp = (SyntaxTree*)context->tmp_ptr;
    context->bCompileSetVal = false;
    exp->compile(context);
    get(0)->compile(context);   // IDENT
    SymbolID_t memberSym = context->tmp_symbol;
    context->setSourceInfo(get(0)->str.endPos);
    context->addCode<OP_getMember>(memberSym);
}

//============================================================
//@ functor         <- %s argList
void P_functor::actionAtParse(SyntaxTree*){}
void ST_functor::compile(Context* context)
{
    PRE_COMPILE_HOOK(functor);
    SyntaxTree* exp = (SyntaxTree*)context->tmp_ptr;
    get(0)->compile(context);   // argList
    hyu16 signatureID = (hyu16) context->tmp_int;
    context->bCompileSetVal = false;
    exp->compile(context);
    context->setSourceInfo(str.endPos);
    switch (signatureID) {
    case 0:
        context->addCode<OP_insMethod_0>(HyCSymx_2829);
        break;
    case 1:
        context->addCode<OP_insMethod_1>(HyCSymx_2829);
        break;
    case 2:
        context->addCode<OP_insMethod_2>(HyCSymx_2829);
        break;
    default:
        context->addCode<OP_insMethod>(signatureID, HyCSymx_2829);
    }     
}



//============================================================
//@ newObj          <- %'new' %spc IDENT %s argList
void P_newObj::actionAtParse(SyntaxTree*){}
void ST_newObj::compile(Context* context)
{
    PRE_COMPILE_HOOK(newObj);
    hyu16 signatureID;
    get(1)->compile(context);   // argList
    signatureID = (hyu16) context->tmp_int;
    get(0)->compile(context);   // IDENT
    SymbolID_t classSym = context->tmp_symbol;
    context->setSourceInfo(str.endPos);
    context->addCode<OP_new>(classSym, signatureID);
}

//============================================================
//@ true            <- %'true'
void P_true::actionAtParse(SyntaxTree*){}
void ST_true::compile(Context* context)
{
    PRE_COMPILE_HOOK(true);
    context->setSourceInfo(str.endPos);
    context->addCode<OP_push_true>();
}

//============================================================
//@ false           <- %'false'
void P_false::actionAtParse(SyntaxTree*){}
void ST_false::compile(Context* context)
{
    PRE_COMPILE_HOOK(false);
    context->setSourceInfo(str.endPos);
    context->addCode<OP_push_false>();
}

//============================================================
//@ nil             <- %'nil'
void P_nil::actionAtParse(SyntaxTree*){}
void ST_nil::compile(Context* context)
{
    PRE_COMPILE_HOOK(nil);
    context->setSourceInfo(str.endPos);
    context->addCode<OP_push_nil>();
}

//============================================================
//@ self            <- %'self'
void P_self::actionAtParse(SyntaxTree*){}
void ST_self::compile(Context* context)
{
    PRE_COMPILE_HOOK(self);
    context->setSourceInfo(str.endPos);
    context->addCode<OP_push_self>();
}

//============================================================
//@ symbol          <- SYMBOL
void P_symbol::actionAtParse(SyntaxTree*){}
void ST_symbol::compile(Context* context)
{
    PRE_COMPILE_HOOK(symbol);
    get(0)->compile(context);
    context->addCode<OP_push_symbol>(context->tmp_symbol);
}



//============================================================
//@ # 配列リテラル 
//@ arrayLiteral    <- %"[" %z (expList %z)? %("," z)? %"]"
void P_arrayLiteral::actionAtParse(SyntaxTree*){}
void ST_arrayLiteral::compile(Context* context)
{
    PRE_COMPILE_HOOK(arrayLiteral);
    context->setSourceInfo(str.startPos);
    if (numChild() < 1) {
        context->addCode<OP_new>(HyCSym_Array, 0);
        return;
    }
    SyntaxTree* expList = get(0);
    int n = expList->numChild();
    if (n == 0) {
        context->addCode<OP_new>(HyCSym_Array, 0);
    } else {
        context->addCodePushInt(n);
        context->addCode<OP_new>(HyCSym_Array, 1);
    }

    int i = 0;
    while (n > 0) {
        int m = (n < 10) ? n : 10;
        for (int k = 0; k < m; k++)
            expList->get(i++)->compile(context);
        context->addCode<OP_substArray>(m);
        n -= m;
    }
}

//============================================================
//@ expList         <- exp (%z %"," %z exp)*
void P_expList::actionAtParse(SyntaxTree*){}
void ST_expList::compile(Context* context)
{
    PRE_COMPILE_HOOK(expList);
    int n = numChild();
    HMD_ASSERT(n < 127);
    for (int i = 0; i < n; i++) {
        get(i)->compile(context);
    }
    context->tmp_int = n;
    // tmp_int に exp の個数が入っている
}

//============================================================
//@ # ハッシュリテラル 
//@ hashLiteral     <- %"{" %z (hashPair (%z %"," %z hashPair)* %z)? %("," z)? %"}"
void P_hashLiteral::actionAtParse(SyntaxTree*){}
void ST_hashLiteral::compile(Context* context)
{
    PRE_COMPILE_HOOK(hashLiteral);
    context->setSourceInfo(str.startPos);
    int n = numChild();
    if (n < 1) {
        context->addCodePushInt(2);
        context->addCode<OP_new>(HyCSym_Hash, 1);
        return;
    }
    int capa = (n + 1) * HASH_DEFAULT_LOAD_FACTOR / 100;
    if (capa < 2) capa = 2;
    context->addCodePushInt(capa);
    context->addCode<OP_new>(HyCSym_Hash, 1);
    int i = 0;
    while (n > 0) {
        int m = (n < 5) ? n : 5;
        for (int k = 0; k < m; k++)
            get(i++)->compile(context);
        context->addCode<OP_substHash>(m);
        n -= m;
    }
}

//============================================================
//@ hashPair        <- exp %s %"=>" %z exp
void P_hashPair::actionAtParse(SyntaxTree*){}
void ST_hashPair::compile(Context* context)
{
    PRE_COMPILE_HOOK(hashPair);
    get(0)->compile(context);
    get(1)->compile(context);
}

//============================================================
//@ # リストリテラル 
//@ listLiteral     <- %"'(" %z (exp ( %z %"," %z exp)* %z)? %("," z)? %")"
void P_listLiteral::actionAtParse(SyntaxTree*){}
void ST_listLiteral::compile(Context* context)
{
    PRE_COMPILE_HOOK(listLiteral);
    context->setSourceInfo(str.startPos);
    context->addCode<OP_pushEmptyList>();
    int n = numChild();
    while (--n >= 0) {
        get(n)->compile(context);
        context->addCode<OP_cons>();
    }
}


//@ # 変数 

//============================================================
//@ Var             <- IDENT
void P_Var::actionAtParse(SyntaxTree*){}
void ST_Var::compile(Context* context)
{
    PRE_COMPILE_HOOK(Var);
    Var_t var = context->searchVar(gpInp, str);
    context->setSourceInfo(str.endPos);
    compileVar(context, var);
}

//============================================================
//@ membVar         <- %"@" IDENT
void P_membVar::actionAtParse(SyntaxTree*){}
void ST_membVar::compile(Context* context)
{
    PRE_COMPILE_HOOK(membVar);
    get(0)->compile(context);
    context->classInfo()->addMembVar(context->tmp_symbol);
    context->setSourceInfo(str.endPos);
    if (context->bCompileSetVal) {
        context->addCode<OP_setMembVar>(context->tmp_symbol);
    } else {
        context->addCode<OP_getMembVar>(context->tmp_symbol);
    }
}

//============================================================
//@ classVar        <- %"@@" IDENT
void P_classVar::actionAtParse(SyntaxTree*){}
void ST_classVar::compile(Context* context)
{
    PRE_COMPILE_HOOK(classVar);
    get(0)->compile(context);
    context->classInfo()->addClassVar(context->tmp_symbol);
    context->setSourceInfo(str.endPos);
    if (context->bCompileSetVal) {
        context->addCode<OP_setClassVar>(context->tmp_symbol);
    } else {
        context->addCode<OP_getClassVar>(context->tmp_symbol);
    }
}

//============================================================
//@ globalVar       <- %"$" IDENT
void P_globalVar::actionAtParse(SyntaxTree*){}
void ST_globalVar::compile(Context* context)
{
    PRE_COMPILE_HOOK(globalVar);
    get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    if (context->bCompileSetVal) {
        context->addCode<OP_setGlobal>(context->tmp_symbol);
    } else {
        context->addCode<OP_getGlobal>(context->tmp_symbol);
    }
}

//============================================================
//@ # クラス内定数、インナークラス
//@ scopedVal       <- scopedIDENT
void P_scopedVal::actionAtParse(SyntaxTree*){}
void ST_scopedVal::compile(Context* context)
{
    PRE_COMPILE_HOOK(scopedVal);
    context->setSourceInfo(str.endPos);

    TArray<SymbolID_t> arr(4);
    context->tmp_ptr = &arr;
    get(0)->compile(context);   // scopedIDENT
    SymbolID_t varSym = arr[-1];
    arr.remove(-1);

    Var_t var = context->classInfo()->getScopedVal(arr, varSym);
    switch (var.type) {
    case IT_CONST:
        {
            ConstVar_t* cv = var.constInfo;
            switch (cv->type) {
            case CONST_TYPE_INT:
                context->addCodePushInt(cv->intValue);
                return;
            case CONST_TYPE_FLOAT:
                context->addCodePushFloat(cv->floatValue);
                return;
            default:
                ;
            }
            // todo: 静的にスコープが決定できなかった場合はwarningにする？
    
            context->addCode<OP_getScopeClass>(&arr);
            context->addCode<OP_getClassConst>(varSym);
        }
        break;

    case IT_CLASS:
        {
            arr.add(varSym);
            context->addCode<OP_getScopeClass>(&arr);
        }
        break;

    default:
        context->addCode<OP_getScopeClass>(&arr);
        context->addCode<OP_getScopedVal>(varSym);
    }
}

//============================================================
//@ enum            <- %'enum' %z $1 %"{" %z $2 IDENT (%z %"," %z !"}" $3 IDENT)* %z %","? %z $4 %"}"
//@ enum$1 = "'{' required for 'enum'"
//@ enum$2 = "constant variable name required"
//@ enum$3 = "constant variable name required"
//@ enum$4 = "closing '}' of enum required"
void P_enum::actionAtParse(SyntaxTree*){}
void ST_enum::compile(Context* context)
{
    PRE_COMPILE_HOOK(enum);

    int k = 0;

    hyu32 n = numChild();
    for (hyu32 i = 0; i < n; i++) {
        context->popLastVal();
        get(i)->compile(context);
        SymbolID_t varSym = context->tmp_symbol;
        ConstVar_t* pv = context->classInfo()->getConstVar(varSym);
        if (pv != NULL)
            compileError_pos(get(i), M_M("multiple definition of const variable"));
        pv = context->classInfo()->createConstVar(varSym);
        pv->type = CONST_TYPE_INT;
        pv->intValue = (hys32)i;

        context->addCodePushInt(k++);
        context->addCode<OP_setConstVar>(varSym);
        context->bPushCode = true;
    }
}

//============================================================
//@ scopedIDENT     <- IDENT? "::" IDENT (%"::" IDENT)*
void P_scopedIDENT::actionAtParse(SyntaxTree*){}
void ST_scopedIDENT::compile(Context* context)
{
    PRE_COMPILE_HOOK(scopedIDENT);
    // tmp_ptr に、結果を入れるべき TArray<SymbolID_t>* が入っている
    Scope_t* pScope = (TArray<SymbolID_t>*) context->tmp_ptr;
    HMD_DEBUG_ASSERT(pScope != NULL);
    hys32 i = 0;
    if (gpInp->cmpStr(get(0)->str, "::")) {
        // IDENT? が空で "::" から始まっている
        pScope->add(HyCSym_nil);
        i = 1;
    } else {
        pScope->add(gSymTable.symbolID(gpInp, get(0)->str));
        i = 2;                  // "::" をスキップ
    }
    for ( ; i < numChild(); i++) {
        pScope->add(gSymTable.symbolID(gpInp, get(i)->str));
    }
}

//============================================================
//@ subst           <- singleSubst / multiSubst
void P_subst::actionAtParse(SyntaxTree*){}
void ST_subst::compile(Context* context)
{
    PRE_COMPILE_HOOK(subst);
    get(0)->get(0)->compile(context);
}


//============================================================

// 代入文コンパイル共通ルーチン
enum {
    SUBST_LEFT_MODE_PREP, // 事前に計算しておく必要のある部分のコンパイル
    SUBST_LEFT_MODE_R_ADJ, // 右辺値がスタックトップにある場合の代入前スタック調整
    SUBST_LEFT_MODE_COPY,    // SUBST_LEFT_MODE_PREPで計算した値を複製
    SUBST_LEFT_MODE_GET,        // 値を取得
    SUBST_LEFT_MODE_SET,        // 値を代入
    SUBST_LEFT_MODE_GET_N, // SUBST_LEFT_MODE_PREP + SUBST_LEFT_MODE_GET
    SUBST_LEFT_MODE_DROP, // SUBST_LEFT_MODE_PREPの値を捨てる(演算子ショートカット用)
};
static void substCompile(Context* context,
                         SyntaxTree* left,  // 左辺 leftValue
                         SyntaxTree* right, // 右辺 : NULL ならスタック上にある
                         SyntaxTree* opr = NULL) // 演算子
{
    HMD_ASSERT(!(right == NULL && opr != NULL));
    // 多重代入(right==NULL) かつ 代入演算子(opr!=NULL) である事はない

#if 0
    printf("substCompile: ");
    gpInp->fprintStr(stdout, left->str);
    printf(" ");
    if (opr != NULL)
        gpInp->fprintStr(stdout, opr->str);
    else
        printf("=");
    printf(" ");
    if (right != NULL)
        gpInp->fprintStr(stdout, right->str);
    else
        printf("(stack)");
    printf("\n");
#endif

    if (opr == NULL) {
        // 単純代入
        context->tmp_int = SUBST_LEFT_MODE_PREP;
        left->compile(context);
        if (right != NULL) {
            right->compile(context);
            context->tmp_int = SUBST_LEFT_MODE_R_ADJ;
            left->compile(context);
        }
        context->tmp_int = SUBST_LEFT_MODE_SET;
        left->compile(context);
    } else {
        // 代入演算子
        //HMD_ASSERT(right != NULL);
        context->tmp_int = SUBST_LEFT_MODE_PREP;
        left->compile(context);
        context->tmp_int = SUBST_LEFT_MODE_COPY;
        left->compile(context);
        context->tmp_int = SUBST_LEFT_MODE_GET;
        left->compile(context);

        int op;
        if (gpInp->cmpStr(opr->str, "&&="))
            op = 1;
        else if (gpInp->cmpStr(opr->str, "||="))
            op = 2;
        else
            op = 0;

        if (op == 0) {
            right->compile(context);
            context->addCode<OP_swap>();
            opr->compile(context);  // 演算コード生成
            context->tmp_int = SUBST_LEFT_MODE_R_ADJ;
            left->compile(context);
            context->tmp_int = SUBST_LEFT_MODE_SET;
            left->compile(context);
        } else {
            // 演算子shortcut
            context->addCode<OP_copy_top>();
            hyu32 resolveAddrToNoOp;
            if (op == 1)
                resolveAddrToNoOp = context->addGotoCode<OP_jump_ifFalse>();
            else
                resolveAddrToNoOp = context->addGotoCode<OP_jump_ifTrue>();
            context->addCode<OP_pop>();
            right->compile(context);
            context->tmp_int = SUBST_LEFT_MODE_R_ADJ;
            left->compile(context);
            context->tmp_int = SUBST_LEFT_MODE_SET;   // leftへ値を代入するコード生成
            left->compile(context);
            hyu32 resolveAddrToEnd = context->addGotoCode<OP_jump>();
            context->resolveJumpAddr(resolveAddrToNoOp);
            context->tmp_int = SUBST_LEFT_MODE_DROP;
            left->compile(context);
            context->resolveJumpAddr(resolveAddrToEnd);
        }
    }
}


/*

    if (bLeftIsArray) {
        // 配列代入
        if (opr == NULL) {
            // 単純代入
            if (right != NULL)
                right->compile(context);
            left->compile(context);
            context->setSourceInfo(left->str.endPos);
            context->addCode<OP_insMethod_2>(HyCSymx_5b5d3d); // []=
        } else {
            // 代入演算子
            left->compile(context);
            context->addCode<OP_copy_top_2nd>();
            context->addCode<OP_insMethod_1>(HyCSymx_5b5d); // []
            context->setSourceInfo(opr->str.startPos);

            int op;
            if (gpInp->cmpStr(opr->str, "&&="))
                op = 1;
            else if (gpInp->cmpStr(opr->str, "||="))
                op = 2;
            else
                op = 0;

            if (op == 0) {
                right->compile(context);
                context->addCode<OP_swap>();
                opr->compile(context);
                context->setSourceInfo(left->str.endPos);
                context->addCode<OP_rotate_r>();
                context->addCode<OP_insMethod_2>(HyCSymx_5b5d3d); // []=
            } else {
                // shortcut
                context->addCode<OP_copy_top>();
                hyu32 resolveAddrToNoOp;
                if (op == 1)
                    resolveAddrToNoOp = context->addGotoCode<OP_jump_ifFalse>();
                else
                    resolveAddrToNoOp = context->addGotoCode<OP_jump_ifTrue>();
                context->addCode<OP_pop>();
                right->compile(context);
                context->setSourceInfo(left->str.endPos);
                context->addCode<OP_rotate_r>();
                context->addCode<OP_insMethod_2>(HyCSymx_5b5d3d); // []=
                hyu32 resolveAddrToEnd = context->addGotoCode<OP_jump>();

                context->resolveJumpAddr(resolveAddrToNoOp);
                context->addCode<OP_rotate_r>();
                context->addCode<OP_pop>();
                context->addCode<OP_pop>();
                context->resolveJumpAddr(resolveAddrToEnd);
             }
        }
    } else {
        // 左辺が配列でない代入
        if (opr == NULL) {
            // 単純代入
            if (right != NULL)
                right->compile(context);
            context->setSourceInfo(left->str.endPos);
            context->bCompileSetVal = true;     // leftにsetモード
            left->compile(context);
        } else {
            // 代入演算子

            int op;
            if (gpInp->cmpStr(opr->str, "&&="))
                op = 1;
            else if (gpInp->cmpStr(opr->str, "||="))
                op = 2;
            else
                op = 0;

            if (op == 0) {
                context->bCompileSetVal = false;    // getモード
                right->compile(context);
                left->compile(context);
                context->setSourceInfo(opr->str.startPos);
                opr->compile(context);
                context->setSourceInfo(opr->str.endPos);
                context->bCompileSetVal = true;     // leftにsetモード
                left->compile(context);
            } else {
                context->bCompileSetVal = false;    // leftからgetモード
                left->compile(context);
                context->setSourceInfo(opr->str.startPos);

                context->addCode<OP_copy_top>();
                hyu32 resolveAddrToEnd;
                if (op == 1)
                    resolveAddrToEnd = context->addGotoCode<OP_jump_ifFalse>();
                else
                    resolveAddrToEnd = context->addGotoCode<OP_jump_ifTrue>();
                context->addCode<OP_pop>();
                right->compile(context);
                context->bCompileSetVal = true;     // leftにsetモード
                left->compile(context);
                context->resolveJumpAddr(resolveAddrToEnd);
            }
        }
        context->bCompileSetVal = false;
    }
}
*/


//============================================================
//@ singleSubst     <- leftValue %s substOp %z $1 exp
//@ singleSubst$1 = "right value of substitution statement required"
void P_singleSubst::actionAtParse(SyntaxTree*){}
void ST_singleSubst::compile(Context* context)
{
    PRE_COMPILE_HOOK(singleSubst);
    context->popLastVal();
    substCompile(context,
                 get(0),                                // leftValue
                 get(2),                                // exp
                 (get(1)->get(0)->chooseNum == 0) ? NULL : get(1)); // substOp
    context->bPushCode = true;
}

//============================================================
//@ substOp         <- ("=" !"=") / "*=" / "/=" / "%=" / "+=" / "-="
//@                  / "&=" / "|=" / "^=" / "&&=" / "||="
void P_substOp::actionAtParse(SyntaxTree*){}
void ST_substOp::compile(Context* context)
{
    static SymbolID_t opsym[] = {
        HyCSymx_2a, // "*"
        HyCSymx_2f, // "/"
        HyCSymx_25, // "%"
        HyCSymx_2b, // "+"
        HyCSymx_2d, // "-"
        HyCSymx_26, // "&"
        HyCSymx_7c, // "|"
        HyCSymx_5e, // "^"
        HyCSymx_2626, // "&&"
        HyCSymx_7c7c, // "||"
    };

    context->setSourceInfo(str.endPos);
    int n = get(0)->chooseNum - 1;
    HMD_ASSERTMSG(0 <= n && n <= 9, M_M("compiler bug: subst op"));
    context->addCode<OP_insMethod_1>(opsym[n]);
}


//============================================================

// ST_leftValueListのバイトコード生成

static void compileMultiSubstLeft(Context* context, ST_leftValueList* lvList)
{
    hyu32 n = lvList->numChild();
    while (n-- > 0) {
        SyntaxTree* lvElem = lvList->get(n);
        SyntaxTree* ch = lvElem->get(0);
        if (ch->chooseNum == 0) {
            // recursive leftValueList
            if (ch->numChild() > 0)
                compileMultiSubstLeft(context, (ST_leftValueList*) ch->get(0));
            else {
                // '() = x
                if (n > 0) {
                    continue;
                } else {
                    context->addCode<OP_pushEmptyList>();
                    return;
                }
            }
        } else {
            substCompile(context,
                         ch->get(0),    // leftValue
                         NULL,          // 右辺はスタック上
                         NULL);         // 代入演算無し
        }
        if (n > 0)
            context->addCode<OP_pop>(); // 最後の値以外はpop
    }
}


//============================================================
//@ multiSubst      <- leftValueList %s %("=" !"=") %z $1 rightValueList
//@ multiSubst$1 = "right value list of multi substitution statement required"
void P_multiSubst::actionAtParse(SyntaxTree*){}
void ST_multiSubst::compile(Context* context)
{
    PRE_COMPILE_HOOK(multiSubst);
    context->popLastVal();

    hyu8 idbuf[sizeof(hyu16)];

    // 右辺
    hyu32 rightIDCodePos = context->codeAddr() + 1;
    context->addCode<OP_ms_right>((hyu16)-1);

    Signature rightSig;
    context->tmp_signature = &rightSig;
    get(1)->compile(context);   // rightValueList
    hyu16 rightID = context->getSignatureID(rightSig.getSigBytes(), rightSig.getSigBytesLen());
    Endian::pack<hyu16>(idbuf, rightID);
    context->bytecode().replaceCodes(rightIDCodePos, idbuf, sizeof(hyu16));


    hyu32 leftIDCodePos = context->codeAddr() + 1;
    context->addCode<OP_ms_testLeft>((hyu16)-1, 0);
    hyu32 resolveAddrFail = context->codeAddr() - OPR_RELATIVE::SIZE;

    // 左辺Signatureのコンパイル
    Signature leftSig;
    context->tmp_signature = &leftSig;
    get(0)->compile(context);   // leftValueList
    hyu16 leftID = context->getSignatureID(leftSig.getSigBytes(), leftSig.getSigBytesLen());
    Endian::pack<hyu16>(idbuf, leftID);
    context->bytecode().replaceCodes(leftIDCodePos, idbuf, sizeof(hyu16));
    // 左辺代入部分
    compileMultiSubstLeft(context, (ST_leftValueList*)get(0));

    context->addCode<OP_jump>(-2);
    hyu32 resolveAddrEnd = context->codeAddr() - OPR_RELATIVE::SIZE;

    // 代入失敗部分
    context->resolveJumpAddr(resolveAddrFail);
    context->addCode<OP_push_symbol>(HyCSym_multisubst_failed);
    context->addCode<OP_push_symbol>(HyCSym_multisubst_failed); // 値も同じシンボル
    context->addCode<OP_throw>();
    
    // 終了
    context->resolveJumpAddr(resolveAddrEnd);
    context->bPushCode = true;
}


//============================================================
//@ leftValue       <- leftExp / leftVar / membVar / classVar / globalVar
void P_leftValue::actionAtParse(SyntaxTree*){}
void ST_leftValue::compile(Context* context)
{
    // context->tmp_int の SUBST_LEFT_MODE_* によって動作が変わる
    PRE_COMPILE_HOOK(leftValue);
    if (get(0)->chooseNum == 0) {
        get(0)->get(0)->compile(context);
    } else {
        switch (context->tmp_int) {
        case SUBST_LEFT_MODE_GET:
            context->bCompileSetVal = false;
            get(0)->get(0)->compile(context);
            break;
        case SUBST_LEFT_MODE_SET:
            context->bCompileSetVal = true;
            get(0)->get(0)->compile(context);
            break;
        }
    }
}

//============================================================
//@ leftExp         <- exp0 (Larray / LmethodCall / Lmember / Lfunctor)+
void P_leftExp::actionAtParse(SyntaxTree*){}
void ST_leftExp::compile(Context* context)
{
    // context->tmp_int の SUBST_LEFT_MODE_* によって動作が変わる
    PRE_COMPILE_HOOK(leftExp);
    int mode = context->tmp_int;
    hyu32 n = numChild();
    SyntaxTree* glast = get(n-1);
    hyu32 ln = glast->chooseNum;
    if (ln == 1 || ln == 3) {
        compileError_pos(glast, M_M("method call or functor cannot be a left value"));
    }
    if (mode == SUBST_LEFT_MODE_PREP) {
        context->setSourceInfo(get(0)->str.startPos);
        context->bCompileSetVal = false;
        get(0)->compile(context);
        for (hyu32 i = 1; i < n-1; ++i) {
            context->setSourceInfo(get(i)->str.startPos);
            context->tmp_int = SUBST_LEFT_MODE_GET_N;
            get(i)->get(0)->compile(context);
        }
    }
    context->setSourceInfo(glast->str.startPos);
    context->tmp_int = mode;
    glast->get(0)->compile(context);
}


//============================================================
//@ Larray          <- %s %"[" %z $1 exp %z $2 %"]"
void P_Larray::actionAtParse(SyntaxTree*){}
void ST_Larray::compile(Context* context)
{
    PRE_COMPILE_HOOK(Larray);
    switch (context->tmp_int) {
    case SUBST_LEFT_MODE_PREP:  // [ins] -> [exp ins]
        get(0)->compile(context);
        break;
    case SUBST_LEFT_MODE_R_ADJ: // [val . .] -> [. . val]
        context->addCode<OP_rotate_r>();
        break;
    case SUBST_LEFT_MODE_COPY:  // [exp ins] -> [exp ins exp ins]
        context->addCode<OP_copy_top_2nd>();
        break;
    case SUBST_LEFT_MODE_GET:   // [exp ins] -> [x]
        context->addCode<OP_swap>();
        context->addCode<OP_insMethod_1>(HyCSymx_5b5d); // []
        break;
    case SUBST_LEFT_MODE_SET:   // [exp ins val] -> [val]
        context->addCode<OP_swap>();
        context->addCode<OP_insMethod_2>(HyCSymx_5b5d3d); // []=
        break;
    case SUBST_LEFT_MODE_GET_N: // [ins] -> [exp ins] -> [x]
        get(0)->compile(context);
        context->addCode<OP_swap>();
        context->addCode<OP_insMethod_1>(HyCSymx_5b5d); // []
        break;
    case SUBST_LEFT_MODE_DROP:  // [x exp ins] -> [x]
        context->addCode<OP_rotate_r>();
        context->addCode<OP_pop>();
        context->addCode<OP_pop>();
        break;
    default:
        HMD_FATAL_ERROR("compiler bug: substLeftMode=%d", context->tmp_int);
    }
}

//============================================================
//@ LmethodCall     <- %"." IDENT %s argList
void P_LmethodCall::actionAtParse(SyntaxTree*){}
void ST_LmethodCall::compile(Context* context)
{
    PRE_COMPILE_HOOK(LmethodCall);
    HMD_ASSERT(context->tmp_int == SUBST_LEFT_MODE_GET_N);
    hyu16 signatureID;
    get(1)->compile(context);   // argList
    signatureID = (hyu16) context->tmp_int;
    get(0)->compile(context);   // IDENT_m
    SymbolID_t methodSym = context->tmp_symbol;
    Context::getPackage()->checkMethod(methodSym, signatureID, str.startPos);
    context->setSourceInfo(str.endPos);
    switch (signatureID) {
    case 0:
        context->addCode<OP_insMethod_0>(methodSym);
        break;
    case 1:
        context->addCode<OP_insMethodR_1>(methodSym);
        break;
    case 2:
        context->addCode<OP_insMethodR_2>(methodSym);
        break;
    default:
        context->addCode<OP_insMethodR>(signatureID, methodSym);
    }     
}


//============================================================
//@ Lmember         <- %"." $1 IDENT_l
void P_Lmember::actionAtParse(SyntaxTree*){}
void ST_Lmember::compile(Context* context)
{
    PRE_COMPILE_HOOK(Lmember);
    SyntaxTree* member = get(0);
    switch (context->tmp_int) {
    case SUBST_LEFT_MODE_PREP:
    case SUBST_LEFT_MODE_DROP:
        break;
    case SUBST_LEFT_MODE_R_ADJ: // [val ins] -> [ins val]
        context->addCode<OP_swap>();
        break;
    case SUBST_LEFT_MODE_COPY:  // [ins] -> [ins ins]
        context->addCode<OP_copy_top>();
        break;
    case SUBST_LEFT_MODE_GET:
    case SUBST_LEFT_MODE_GET_N: // [ins] -> [x]
        {
            member->compile(context);
            SymbolID_t memberSym = context->tmp_symbol;
            context->addCode<OP_getMember>(memberSym);
        }
        break;
    case SUBST_LEFT_MODE_SET:   // [ins val] -> [val]
        {
            member->compile(context);
            SymbolID_t memberSym = context->tmp_symbol;
            hyu32 len = member->str.len();
            char* buf = (char*)HMD_ALLOCA(len+2);
            gpInp->copyStr(buf, len, member->str);
            buf[len] = '=';
            buf[len+1] = '\0';
            SymbolID_t setterMethodSym = gSymTable.symbolID(buf);
            context->setSourceInfo(member->str.endPos);
            context->addCode<OP_setMember>(memberSym, setterMethodSym);
        }
        break;
    default:
        HMD_FATAL_ERROR("compiler bug: substLeftMode=%d", context->tmp_int);
    }
}

//============================================================
//@ Lfunctor        <- %s argList
void P_Lfunctor::actionAtParse(SyntaxTree*){}
void ST_Lfunctor::compile(Context* context)
{
    PRE_COMPILE_HOOK(Lfunctor);
    HMD_ASSERT(context->tmp_int == SUBST_LEFT_MODE_GET_N);
    get(0)->compile(context);   // argList
    hyu16 signatureID = (hyu16) context->tmp_int;
    context->setSourceInfo(str.endPos);
    switch (signatureID) {
    case 0:
        context->addCode<OP_insMethod_0>(HyCSymx_2829);
        break;
    case 1:
        context->addCode<OP_insMethodR_1>(HyCSymx_2829);
        break;
    case 2:
        context->addCode<OP_insMethodR_2>(HyCSymx_2829);
        break;
    default:
        context->addCode<OP_insMethodR>(signatureID, HyCSymx_2829);
    }     
}



//============================================================
static bool isConstIdent(InputBuffer* inp, Substr_st& ss)
{
    if (ss.len() < 1) return false;
    wchar_t c = inp->getCharAt(ss.startPos);
    if (c < L'A' || c > L'Z') return false;
    for (hyu32 pos = ss.startPos + 1; pos < ss.endPos; ++pos) {
        c = inp->getCharAt(pos);
        if (!(c >= L'A' && c <= L'Z')
            && !(c >= L'0' && c <= L'9')
            && (c != L'_'))
            return false;
    }
    return true;
}


//============================================================
//@ leftVar         <- %'const' %s $1 IDENT / IDENT
//@ leftVar$1 = "no valid identifier specified after 'const'"
void P_leftVar::actionAtParse(SyntaxTree*){}
void ST_leftVar::compile(Context* context)
{
    PRE_COMPILE_HOOK(leftVar);
    context->setSourceInfo(str.endPos);
    SyntaxTree* ident = get(0)->get(0);

    Var_t var = context->searchVar(gpInp, ident->str);

    bool bConst;
    if (get(0)->chooseNum == 0) {
        Var_t myVar = context->myIdent(gpInp, ident->str);
        if (myVar.type != IT_NONE) {
            compileError_pos(ident,
                             M_M("constant name confilts with %s name"),
                             VAR_IDENT_STR[myVar.type]);
        }
        if (context->bCompileSetVal == false) {
            compileError_pos(ident,
                             M_M("cannot get constant value before initialized"));
        }
        bConst = true;
//'local' 機能は封印
//    } else if (get(0)->chooseNum == 1) {
//        if (context->bCompileSetVal == false) {
//            compileError_pos(ident,
//                             M_M("cannot get variable's value before initialized"));
//        }
//        bConst = false;
    } else {
        if (context->bCompileSetVal == false) {
            compileVar(context, var);
            return;
        }
        if (var.type == IT_CONST) {
            bConst = true;
        } else if (var.type == IT_NONE) {
            bConst = isConstIdent(gpInp, ident->str);
        } else if (var.type == IT_LOCAL) {
            bConst = false;
        } else {
            bConst = false;
        }
    }
        
    if (bConst) {
        // const
        ident->compile(context);
        SymbolID_t varSym = context->tmp_symbol;
        ConstVar_t* pv = context->classInfo()->getLocalConstVar(varSym);
        if (pv != NULL) {
            compileError_pos(this, M_M("multiple definition of const variable"));
        }
        if (var.type == IT_CONST) {
            if (compileOption.Wconstscope == 2) {
                compileError_pos(ident,
                                 M_M("cannot override constant variable '%s'"),
                                 gSymTable.id2str(var.symbol));
            } else if (compileOption.Wconstscope == 1) {
                outWarning(ident->str.endPos,
                           M_M("overriding constant variable '%s'"),
                           gSymTable.id2str(var.symbol));
            }
        }
        pv = context->classInfo()->createConstVar(varSym);
        HMD_DEBUG_ASSERT(pv != NULL);
        // todo: 右辺が定数かどうか判定して、定数ならclassInfoに値を記録
        pv->type = CONST_TYPE_NONE;
        context->addCode<OP_setConstVar>(varSym);
    } else {
        // local variable
        Var_t myVar = context->myIdent(gpInp, ident->str);
        if (myVar.type != IT_NONE && myVar.type != IT_LOCAL) {
            hyu32 bufSize = ident->str.len() + 4;
            char* buf = (char*)HMD_ALLOCA(bufSize);
            gpInp->copyStr(buf, bufSize, ident->str);
            compileError_pos(ident,
                             M_M("local variable '%s' overrides %s name"),
                             buf,
                             VAR_IDENT_STR[myVar.type]);
        }
        LocalVarSymID_t lsym = gLocalVarSymbols.localVarSymID(gpInp, ident->str);
        if (var.type == IT_CONST || var.type == IT_CLASS || var.type == IT_METHOD) {
            if (compileOption.Wconstscope >= 1) {
                outWarning(ident->str.endPos,
                           M_M("local variable '%s' overrides %s"),
                           gLocalVarSymbols.id2str(lsym),
                           VAR_IDENT_STR[var.type]);
            }
        }
        Context::LocalVar_t& lv = context->getLocalVarCreate(lsym);
        context->addCode<OP_setLocal>(lv.idx);
    }
}


//============================================================
//@ leftValueList   <- lvElem (%s %"," %z lvElem)*
void P_leftValueList::actionAtParse(SyntaxTree*){}
void ST_leftValueList::compile(Context* context)
{
    // ここでのコンパイルは Signature 構築のみ。
    // バイトコード生成は compileMultiSubstLeft() が担当。
    PRE_COMPILE_HOOK(leftValueList);
    hyu32 n = numChild();
    for (hyu32 i = 0; i < n; ++i) {
        try {
            get(i)->compile(context);
        } catch (Signature::IllegalArgException e) {
            compileError_pos(get(i), "%s", e.message);
        }
    }
}

//============================================================
//@ lvElem          <- %"'(" %z leftValueList? %z %")" / %"*" %s leftValue / leftValue
void P_lvElem::actionAtParse(SyntaxTree*){}
void ST_lvElem::compile(Context* context)
{
    // ここでのコンパイルは Signature 構築のみ。
    // バイトコード生成は compileMultiSubstLeft() が担当。
    PRE_COMPILE_HOOK(lvElem);
    switch (get(0)->chooseNum) {
    case 0:
        {
            Signature* sub = context->tmp_signature->openSub();
            if (get(0)->numChild() > 0) {
                context->tmp_signature = sub;
                get(0)->get(0)->compile(context);
            }
            context->tmp_signature = sub->close();
        }
        break;
    case 1:
        context->tmp_signature->addSig(Signature::MULTI);
        break;
    case 2:
        context->tmp_signature->addSig(1);
        break;
    }
}

//============================================================
//@ rightValueList  <- rvElem (%s %"," %z rvElem)*
void P_rightValueList::actionAtParse(SyntaxTree*){}
void ST_rightValueList::compile(Context* context)
{
    PRE_COMPILE_HOOK(rightValueList);
    hyu32 n = numChild();
    for (hyu32 i = 0; i < n; ++i) {
        try {
            get(i)->compile(context);   // rvElem
        } catch (Signature::IllegalArgException e) {
            compileError_pos(get(i), "%s", e.message);
        }
    }
}

//============================================================
//@ rvElem          <- %"*" %s exp
//@                  / exp
//@                  / %"'(" %z rightValueList %z %("," z)? %")"
void P_rvElem::actionAtParse(SyntaxTree*){}
void ST_rvElem::compile(Context* context)
{
    PRE_COMPILE_HOOK(rvElem);
    SyntaxTree* ch = get(0); // choice
    Signature* sig = context->tmp_signature;
    switch (ch->chooseNum) {
    case 0: // %"*" %s exp
        ch->get(0)->compile(context); // exp
        sig->addSig(Signature::MULTI);
        break;
    case 1: // exp
        ch->get(0)->compile(context); // exp
        sig->addSig(1);
        break;
    case 2: // %"'(" %z rightValueList %z %("," z)? %")"
        {
            Signature* sub = sig->openSub();
            context->tmp_signature = sub;
            if (ch->numChild() > 0)
                ch->get(0)->compile(context);       // rightValueList
            else
                context->addCode<OP_pushEmptyList>();
            Signature* tmp = sub->close();
            HMD_DEBUG_ASSERT(tmp == sig);
        }
        break;
    default:
        ;
    }
    context->tmp_signature = sig;
}


//============================================================
//@ match           <- %'match' %s $1 %"(" %z rightValueList %z %")" %z $2 %"{" %z $3 caseOfMatch (%separator caseOfMatch)* defaultOfMatch %z $4 %"}"
//@ match$1 = "right value list required after 'match'"
//@ match$2 = "'{' required for 'match' block"
//@ match$3 = "'case' or 'default' statement required"
//@ match$4 = "closing '}' of match block required"

void P_match::actionAtParse(SyntaxTree*){}
void ST_match::compile(Context* context)
{
    PRE_COMPILE_HOOK(match);
    hyu8 idbuf[sizeof(hyu16)];

    context->popLastVal();

    // 値リスト
    hyu32 rightIDCodePos = context->codeAddr() + 1;
    context->setSourceInfo(str.startPos);
    context->addCode<OP_ms_right>((hyu16)-1);

    Signature rightSig;
    context->tmp_signature = &rightSig;
    get(0)->compile(context);   // rightValueList
    hyu16 rightID = context->getSignatureID(rightSig.getSigBytes(), rightSig.getSigBytesLen());
    Endian::pack<hyu16>(idbuf, rightID);
    context->bytecode().replaceCodes(rightIDCodePos, idbuf, sizeof(hyu16));

    hyu32 n = numChild();
    HMD_DEBUG_ASSERT(n >= 3);
    hyu32* resolveAddrs = (hyu32*)HMD_ALLOCA((n-2) * sizeof(hyu32));
    for (hyu32 i = 1; i < n-1; ++i) {
        get(i)->compile(context);       // caseOfMatch
        resolveAddrs[i-1] = (hyu32) context->tmp_int;
    }

    HMD_DEBUG_ASSERT(get(n-1)->parser == p_defaultOfMatch);
    get(n-1)->compile(context);         // defaultOfMatch

    // 終了
    for (hyu32 i = 0; i < n - 2; ++i)
        context->resolveJumpAddr(resolveAddrs[i]);
}

//============================================================
//@ caseOfMatch     <- %'case' %s $1 %"(" $2 %z leftValueList? %z %")" (%z block / %z stmtsInSwitch / %s) %endOfCasePred
//@ caseOfMatch$1 = "left value(s) required after 'case'"
//@ caseOfMatch$2 = "only left value list can be written inside '()' of 'case'"
void P_caseOfMatch::actionAtParse(SyntaxTree*){}
void ST_caseOfMatch::compile(Context* context)
{
    PRE_COMPILE_HOOK(caseOfMatch);
    hyu8 idbuf[sizeof(hyu16)];

    hyu32 leftIDCodePos = context->codeAddr() + 1;
    context->setSourceInfo(str.startPos);
    context->addCode<OP_ms_testLeft>((hyu16)-1, 0);
    hyu32 resolveAddrFail = context->codeAddr() - OPR_RELATIVE::SIZE;

    // Signatureのコンパイル
    Signature leftSig;
    context->tmp_signature = &leftSig;
    if (numChild() > 1)
        get(0)->compile(context);   // leftValueList
    leftSig.adjustDefaultValIdx();
    hyu16 leftID = context->getSignatureID(leftSig.getSigBytes(), leftSig.getSigBytesLen());
    Endian::pack<hyu16>(idbuf, leftID);
    context->bytecode().replaceCodes(leftIDCodePos, idbuf, sizeof(hyu16));
    // 代入部分
    if (numChild() > 1) {
        compileMultiSubstLeft(context, (ST_leftValueList*)get(0));
        context->addCode<OP_pop>();     // 代入式の値を捨てる
    }
    
    // block / stmtsInSwitch / %s
    context->bPushCode = false;
    if (numChild() > 0)
        get(numChild()-1)->get(0)->compile(context);
    context->needLastVal();
    // 成功したので終了
    context->addCode<OP_jump>(-2);
    context->tmp_int = (hys32) (context->codeAddr() - OPR_RELATIVE::SIZE);

    // マッチしなかった場合のジャンプアドレス解決
    context->resolveJumpAddr(resolveAddrFail);
}

//============================================================
//@ defaultOfMatch  <- (%separator %'default' %z (block / stmtsInSwitch / %s) %endOfDefaultPred)?
void P_defaultOfMatch::actionAtParse(SyntaxTree*){}
void ST_defaultOfMatch::compile(Context* context)
{
    PRE_COMPILE_HOOK(defaultOfMatch);

    context->addCode<OP_ms_end>();
    context->bPushCode = false;
    if (numChild() > 0)
        get(0)->get(0)->compile(context);
    context->needLastVal();
}


//============================================================
//@ endOfCasePred   <- &(separator 'case') / &(separator 'default') / %separator? &"}"
void P_endOfCasePred::actionAtParse(SyntaxTree*){}
void ST_endOfCasePred::compile(Context*){}
//============================================================
//@ endOfDefaultPred <- %separator? &"}"
void P_endOfDefaultPred::actionAtParse(SyntaxTree*){}
void ST_endOfDefaultPred::compile(Context*){}
//============================================================
//@ oneStmtInSwitch <- !(%'case' / %'default' / "}") oneStmt
void P_oneStmtInSwitch::actionAtParse(SyntaxTree*){}
void ST_oneStmtInSwitch::compile(Context* context)
{
    PRE_COMPILE_HOOK(oneStmtInSwitch);
    get(0)->compile(context);
}
//============================================================
//@ stmtsInSwitch   <- oneStmtInSwitch (%separator oneStmtInSwitch)*
void P_stmtsInSwitch::actionAtParse(SyntaxTree*){}
void ST_stmtsInSwitch::compile(Context* context)
{
    PRE_COMPILE_HOOK(stmtsInSwitch);

    hyu32 n = numChild();
    for (hyu32 i = 0; i < n; ++i) {
        // oneStmt
        get(i)->compile(context);
    }
}



//============================================================
//@ switch          <- %'switch' %s $1 %"(" %z exp %z %")" %z $2 %"{" %z $3 caseOfSwitch (%separator caseOfSwitch)* defaultOfSwitch %z $4 %"}"
void P_switch::actionAtParse(SyntaxTree*){}
void ST_switch::compile(Context* context)
{
    PRE_COMPILE_HOOK(switch);
    context->setSourceInfo(str.startPos);
    context->popLastVal();
    get(0)->compile(context);   // exp

    hyu32 n = numChild();
    HMD_DEBUG_ASSERT(n >= 3);
    TArray<hyu32> resAddrs(n-1);
    for (hyu32 i = 1; i < n-1; ++i) {
        // caseOfSwitch
        get(i)->compile(context);
        resAddrs.add(context->tmp_hyu32);
    }
    HMD_DEBUG_ASSERT(get(n-1)->parser == p_defaultOfSwitch);
    get(n-1)->compile(context);

    TArrayIterator<hyu32> itr(&resAddrs);
    while (itr.hasMore()) {
        context->resolveJumpAddr(itr.next());
    }
}


//============================================================
//@ caseOfSwitch    <- %'case' %s $1 expList $2 (%z block / %separator stmtsInSwitch / %s) %endOfCasePred
//@ caseOfSwitch$1 = "expression(s) required after 'case'"
//@ caseOfSwitch$2 = "separator required after case expression(s)"
void P_caseOfSwitch::actionAtParse(SyntaxTree*){}
void ST_caseOfSwitch::compile(Context* context)
{
    PRE_COMPILE_HOOK(caseOfSwitch);
    SyntaxTree* expList = get(0);
    hyu32 n = expList->numChild();
    TArray<hyu32> resAddrs(n);
    for (hyu32 i = 0; i < n; ++i) {
        context->addCode<OP_copy_top>(); // expをコピー
        expList->get(i)->compile(context);
        context->addCode<OP_insMethod_1>(HyCSymx_3d3d); // == で比較
        resAddrs.add(context->addGotoCode<OP_jump_ifTrue>());
    }        
    hyu32 noMatchResAddr = context->addGotoCode<OP_jump>();
    for (hyu32 i = 0; i < n; ++i) {
        context->resolveJumpAddr(resAddrs[i]);
    }

    context->addCode<OP_pop>();    // expを捨てる

    context->bPushCode = false;
    if (numChild() > 0)
        get(1)->get(0)->compile(context); // block / stmtsInSwitch / %s
    context->needLastVal();
    context->tmp_hyu32 = context->addGotoCode<OP_jump>();

    context->resolveJumpAddr(noMatchResAddr);
}


//============================================================
//@ defaultOfSwitch <- (%separator %'default' %z (block / stmtsInSwitch / %s) %endOfDefaultPred)?
void P_defaultOfSwitch::actionAtParse(SyntaxTree*){}
void ST_defaultOfSwitch::compile(Context* context)
{
    PRE_COMPILE_HOOK(defaultOfSwitch);

    context->addCode<OP_pop>();    // expを捨てる

    context->bPushCode = false;
    if (numChild() > 0)
        get(0)->get(0)->compile(context); // block / stmtsInSwitch / %s
    context->needLastVal();
}


//============================================================
//@ # パッケージリンク 
//@ require         <- %'require' %spc $1 STRING
//@ require$1 = "package file name required"
void P_require::actionAtParse(SyntaxTree*){}
void ST_require::compile(Context* context)
{
    // 直前の値はそのまま手をつけない
    Substr ss = get(0)->str;
    ss.startPos ++; ss.endPos --;   // delete '"'
    hyu32 len = ss.len();
    char* buf = (char*)HMD_ALLOCA(len + 6);
    gpInp->copyStr(buf, len, ss);
    HMD_STRNCPY(&buf[len], ".hyb", 5);
    SymbolID_t pkgSym = gSymTable.symbolID(buf);
    context->require(pkgSym);
}

//============================================================
//@ # include 
//@ include         <- %'include' %spc $1 STRING
//@ include$1 = "include file name required"
void P_include::actionAtParse(SyntaxTree* st)
{
    Substr ss = st->get(0)->str;
    ++ss.startPos;
    --ss.endPos;
    hyu32 size = ss.len() + 1;
    char* p = (char*)HMD_ALLOCA(size);
    gpInp->copyStr(p, size, ss);
    gpInp->include(p);
}
void ST_include::compile(Context* context){}

//============================================================
//@ # using
//@ using           <- %'using' %spc $1 (IDENT / scopedIDENT)
//@ using$1 = "scope required"
void P_using::actionAtParse(SyntaxTree*){}
void ST_using::compile(Context* context)
{
    // 直前の値はそのまま手をつけない
    TArray<SymbolID_t>* pArr = new TArray<SymbolID_t>(2);
    pArr->setContentsMemID("usng");
    if (get(0)->chooseNum == 0) {
        // IDENT
        get(0)->get(0)->compile(context);
        pArr->add(context->tmp_symbol);
    } else {
        context->tmp_ptr = pArr;
        get(0)->get(0)->compile(context);
    }
    context->classInfo()->addUsing(pArr);
}


//@ # 識別子 

//============================================================
//@ IDENT           <% %ident_u1 %ident_u2* %ident_u3?
void ST_IDENT::compile(Context* context)
{
    PRE_COMPILE_HOOK(IDENT);
    context->tmp_symbol = gSymTable.symbolID(gpInp, str);
}
//============================================================
//@ IDENT_cpp       <% (%[A-Z] / %[a-z] / %"_") (%[A-Z] / %[a-z] / %"_" / %[0-9])*
void ST_IDENT_cpp::compile(Context* context) {}


//============================================================
//@ SYMBOL          <% %":" %(IDENT / STRING)
void ST_SYMBOL::compile(Context* context)
{
    PRE_COMPILE_HOOK(SYMBOL);
    Substr ss = str;
    ++ ss.startPos;
    if ((gpInp->getCharAt(ss.startPos) == L'"') && (gpInp->getCharAt(ss.endPos-1) == L'"')) {
        ++ ss.startPos; -- ss.endPos;
        hyu32 len = ss.len();
        char* buf = (char*)HMD_ALLOCA(len + 1);
        hys32 clen = MBCSConv::unescape(gpInp->addr(ss.startPos), len, buf, len+1);
        if (clen < 0)
            compileError_pos(this, M_M("bad symbol string"));
        hyu32 b2s = clen * 2 + 10;
        char* buf2 = (char*)HMD_ALLOCA(b2s);
        hyu32 tlen = MBCSConv::conv(buf, clen, buf2, b2s);
        if (tlen == (hyu32)-1)
            compileError_pos(this, M_M("bad string"));
        context->tmp_symbol = gSymTable.symbolID(buf2);
    } else {
        context->tmp_symbol = gSymTable.symbolID(gpInp, ss);
    }
}

//============================================================
//@ FLOAT           <- FLOAT_1 / FLOAT_2
void P_FLOAT::actionAtParse(SyntaxTree*){}
void ST_FLOAT::compile(Context* context)
{
    PRE_COMPILE_HOOK(FLOAT);
    hyf32 v = (hyf32) atof(gpInp->addr(str.startPos));
    context->setSourceInfo(str.endPos);
    context->addCodePushFloat(v);
}

//============================================================
//@ FLOAT_1         <- %INTEGER_10 %("e" / "E") %("+" / "-")? %INTEGER_10
void P_FLOAT_1::actionAtParse(SyntaxTree*){}
void ST_FLOAT_1::compile(Context* context){}

//============================================================
//@ FLOAT_2         <- %INTEGER_10 %"." %INTEGER_10 %(("e" / "E") ("+" / "-")? INTEGER_10)?
void P_FLOAT_2::actionAtParse(SyntaxTree*){}
void ST_FLOAT_2::compile(Context* context){}

//============================================================
//@ INTEGER         <- INTEGER_2 / INTEGER_8 / INTEGER_16 / INTEGER_10
void P_INTEGER::actionAtParse(SyntaxTree*){}
void ST_INTEGER::compile(Context* context)
{
    PRE_COMPILE_HOOK(INTEGER);
    get(0)->get(0)->compile(context);
    context->setSourceInfo(str.endPos);
    context->addCodePushInt(context->tmp_int);
}

//============================================================
//@ INTEGER_2       <- %("0b" / "0B") %[0-1]+
void P_INTEGER_2::actionAtParse(SyntaxTree*){}
void ST_INTEGER_2::compile(Context* context)
{
    PRE_COMPILE_HOOK(INTEGER_2);
    context->tmp_int = (hys32) strtol(gpInp->addr(str.startPos + 2), NULL, 2);
}

//============================================================
//@ INTEGER_8       <- %("0o" / "0O") %[0-7]+
void P_INTEGER_8::actionAtParse(SyntaxTree*){}
void ST_INTEGER_8::compile(Context* context)
{
    PRE_COMPILE_HOOK(INTEGER_8);
    context->tmp_int = (hys32) strtol(gpInp->addr(str.startPos + 2), NULL, 8);
}

//============================================================
//@ INTEGER_16      <- %("0x" / "0X") %Hex1+
void P_INTEGER_16::actionAtParse(SyntaxTree*){}
void ST_INTEGER_16::compile(Context* context)
{
    PRE_COMPILE_HOOK(INTEGER_16);
    context->tmp_int = (hys32) strtol(gpInp->addr(str.startPos + 2), NULL, 16);
}

//============================================================
//@ INTEGER_10      <% %[0-9]+
void ST_INTEGER_10::compile(Context* context)
{
    PRE_COMPILE_HOOK(INTEGER_10);
    context->tmp_int = (hys32) strtol(gpInp->addr(str.startPos), NULL, 10);
}

//@ #not_ident <- "　" / "「" / "」"
//@ #ident_u1 <- %[A-Z] / %[a-z] / "_" / !not_ident %[\u0080-\uffff]
//@ #ident_u2 <- %ident_u1 / %[0-9]
//@ #ident_u3 <- "?" / "!"

//============================================================
//@ STRING          <% %"\"" (!"\"" %XChar)* %"\""
//@ #XChar <- %XCharH / %XCharE / %XCharA
//@ #SChar <- %XCharH / %XCharE / %XCharS
//@ #XCharH <- %"\\" %("x" / "X") Hex1 Hex1
//@ #XCharE <- %"\\" .
//@ #XCharA <- !"\n" %.
//@ #XCharS <- !"{" %.
void ST_STRING::compile(Context* context)
{
    PRE_COMPILE_HOOK(STRING);
    Substr ss(str.startPos + 1, str.endPos - 1);
    hyu32 len = ss.len();
    char* buf = (char*)HMD_ALLOCA(len + 1);
    hys32 clen = MBCSConv::unescape(gpInp->addr(ss.startPos), len, buf, len+1);
    if (clen < 0)
        compileError_pos(this, M_M("bad string"));
    hyu32 b2s = clen * 2 + 10;
    char* buf2 = (char*)HMD_ALLOCA(b2s);
    hyu32 tlen = MBCSConv::conv(buf, clen+1, buf2, b2s);
    if (tlen == (hyu32)-1)
        compileError_pos(this, M_M("bad string"));
    context->setSourceInfo(str.startPos);
    context->addCode<OP_push_strconst>(gStrTable.getOffs(buf2, tlen));
}


//============================================================
//@ Hex1 <- %[0-9] / %[A-F] / %[a-f]
void P_Hex1::actionAtParse(SyntaxTree*){}
void ST_Hex1::compile(Context* context){}

//============================================================
//@ # not token predicate
//@ NotTokenPred <% !ident_a2
void ST_NotTokenPred::compile(Context* context){}
