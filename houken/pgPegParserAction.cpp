
#include "pgPegParser.h"
#include "hpOperator.h"
#include <stdarg.h>

using namespace Hayat::Common;
using namespace Hayat::Parser;


void SyntaxTree::action(void)
{
#if 0
    // for debugging
    fprintf(stderr, "undefined action of parser %s at: '",parser->name());
    gpInp->fprintStr(stderr, str);
    fprintf(stderr, "'\n");
    fflush(stderr);
    exit(1);
#endif
}


static TArray<Substr> rules;
static TArray<Substr> genRules;
static TArray<const char*> actionFuncs;
static TArray<const char*> actionArgs;
static TArray<Substr> errMes_ruleName;
typedef struct {
    hyu32         number;
    Substr      string;
} ErrMes_st;
static TArray< TArray<ErrMes_st>* > errMes_Number_String;

static bool haveErrorCut;
static bool notCreateErrorCutMessage;

extern char* tab_basepath;      // from main.cpp
extern char* tab_basename;      // from main.cpp

static FILE* hfp = NULL;
static FILE* cppfp = NULL;
static FILE* tmplfp = NULL;
static FILE* upfp = NULL;
static FILE* uafp = NULL;
static Operator* op = NULL;

static void fpOut(FILE* fp, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);
}

static void hOutActionDecl(void)
{
    for (int i = 0; (hyu32)i < actionFuncs.size(); i++) {
        const char* funname = actionFuncs[i];
        const char* args = actionArgs[i];
        if (args[0] != '\0')
            fpOut(hfp, "      void %s(%s); \\\n", funname, args);
        else
            fpOut(hfp, "      void %s(void); \\\n", funname);
    }
}

static void hOutOpen(void)
{
    hyu32 bnsize = HMD_STRLEN(tab_basepath);
    char* tab_filename = gMemPool->allocT<char>(bnsize + 8, "fnth");
    HMD_STRNCPY(tab_filename, tab_basepath, bnsize+1);
    HMD_STRSCAT(tab_filename, ".tab.h", bnsize+8);
    hfp = hmd_fopen(tab_filename, "w");

    char* p = tab_filename;
    while (*p != '\0') {
        if ((*p >= 'a') && (*p <= 'z'))
            *p -= 'a' - 'A';
        else if (*p == '.')
            *p = '_';
        ++p;
    }
#ifdef WIN32
    if (MBCSConv::is_utf8_in()) {
        fpOut(hfp, "\xef\xbb\xbf");       // UTF-8 BOM 
    }
#endif
    fpOut(hfp,
          "/* this file is generated by 'houken' parser generator */\n"
          "#ifndef m_%s_TAB_H_\n"
          "#define m_%s_TAB_H_\n"
          "\n"
          , tab_basename, tab_basename
          );
    fpOut(hfp,
          "#include \"hpParser.h\"\n"
          "\n"
          "using namespace Hayat::Parser;\n"
          "\n"
          "extern void p__init(void);\n"
          "extern void errorCutMessage(hyu32 pos, const char* message);\n"
          "\n"
          "#define USER_PARSER(name)                       \\\n"
          "  class ST_##name : public SyntaxTree {         \\\n"
          "  public:                                       \\\n"
          "      ST_##name(Parser* p, hyu32 startPos, hyu32 endPos, SyntaxTree::Childs* ch = NULL) : SyntaxTree(p, startPos, endPos, ch) {} \\\n"
          );
    hOutActionDecl();
    fpOut(hfp,
          "  };                                            \\\n"
          "  class P_##name : public UserParser {          \\\n"
          "  public: P_##name(void) : UserParser(#name) {} \\\n"
          "      SyntaxTree* uParse(void);                 \\\n"
          "      SyntaxTree* createSyntaxTree(hyu32 startPos, SyntaxTree::Childs* childs = NULL) { \\\n"
          "        return new ST_##name(this, startPos, m_curPos(), childs); } \\\n"
          "  };                                            \\\n"
          "  extern P_##name *      p_##name\n"
          "\n"
          "#define GEN_PARSER(name)                       \\\n"
          "  class ST_##name : public SyntaxTree {        \\\n"
          "  public:                                      \\\n"
          "      ST_##name(Parser* p, hyu32 startPos, hyu32 endPos, SyntaxTree::Childs* ch = NULL) : SyntaxTree(p, startPos, endPos, ch) {} \\\n"
          );
    hOutActionDecl();
    fpOut(hfp,
          "  };                                           \\\n"
          "  class P_##name : public GenParser {          \\\n"
          "  public: P_##name(void) : GenParser(#name) {} \\\n"
          "      void construct(void);                    \\\n"
          "      void actionAtParse(SyntaxTree*);         \\\n"
          "      SyntaxTree* createSyntaxTree(hyu32 startPos, SyntaxTree::Childs* childs = NULL) { \\\n"
          "        return new ST_##name(this, startPos, m_curPos(), childs); } \\\n"
          "  };                                           \\\n"
          "  extern P_##name *      p_##name\n"
          "\n"
          "#define GEN_PARSER_E(name)                     \\\n"
          "  class ST_##name : public SyntaxTree {        \\\n"
          "  public:                                      \\\n"
          "      ST_##name(Parser* p, hyu32 startPos, hyu32 endPos, SyntaxTree::Childs* ch = NULL) : SyntaxTree(p, startPos, endPos, ch) {} \\\n"
          );
    hOutActionDecl();
    fpOut(hfp,
          "  };                                           \\\n"
          "  class P_##name : public GenParser {          \\\n"
          "  public: P_##name(void) : GenParser(#name) {} \\\n"
          "      void construct(void);                    \\\n"
          "      void actionAtParse(SyntaxTree*);         \\\n"
          "      void errorMessage(int errorId, hyu32 pos); \\\n"
          "      SyntaxTree* createSyntaxTree(hyu32 startPos, SyntaxTree::Childs* childs = NULL) { \\\n"
          "        return new ST_##name(this, startPos, m_curPos(), childs); } \\\n"
          "  };                                           \\\n"
          "  extern P_##name *      p_##name\n"
          "\n"
          "#define OPERATOR_PARSER(name)                  \\\n"
          "  class ST_##name : public SyntaxTree {        \\\n"
          "  public:                                      \\\n"
          "      ST_##name(Parser* p, hyu32 startPos, hyu32 endPos, SyntaxTree::Childs* ch = NULL) : SyntaxTree(p, startPos, endPos, ch) {} \\\n"
          );
    hOutActionDecl();
    fpOut(hfp,
          "  };                                                \\\n"
          "  class P_##name : public OperatorParser {          \\\n"
          "  public: P_##name(void) : OperatorParser(#name) {} \\\n"
          "      void construct(void);                         \\\n"
          "      SyntaxTree* createSyntaxTree(hyu32 startPos, SyntaxTree::Childs* childs = NULL) { \\\n"
          "        return new ST_##name(this, startPos, m_curPos(), childs); } \\\n"
          "      SyntaxTree* createSyntaxTree(hyu32 startPos, hyu32 endPos, SyntaxTree::Childs* childs = NULL) { \\\n"
          "        return new ST_##name(this, startPos, endPos, childs); } \\\n"
          "  };                                                \\\n"
          "  extern P_##name *      p_##name\n"
          "\n"
          );
    gMemPool->free(tab_filename);
}

static void hOutClose(void)
{
    fpOut(hfp,
          "#undef USER_PARSER\n"
          "#undef GEN_PARSER\n"
          "#undef GEN_PARSER_E\n"
          "#undef OPERATOR_PARSER\n"
          "\n"
          "\n"
          "#endif\n"
          );            
    fclose(hfp);
}

static void cppOutOpen(void)
{
    hyu32 bnsize = HMD_STRLEN(tab_basepath);
    char* tab_filename = gMemPool->allocT<char>(bnsize + 10, "fntc");
    HMD_STRNCPY(tab_filename, tab_basepath, bnsize+1);
    HMD_STRSCAT(tab_filename, ".tab.cpp", bnsize+10);
    cppfp = hmd_fopen(tab_filename, "w");

    HMD_STRNCPY(tab_filename, tab_basepath, bnsize+1);
    HMD_STRSCAT(tab_filename, ".tab.h", bnsize+10);
#ifdef WIN32
    if (MBCSConv::is_utf8_in()) {
        fpOut(cppfp, "\xef\xbb\xbf");       // UTF-8 BOM 
    }
#endif
    fpOut(cppfp,
          "/* this file is generated by 'houken' parser generator */\n"
          "#include <stdio.h>\n"
          "#include <string.h>\n"
          "#include \"%s.tab.h\"\n"
          "\n"
          "\n"
          "using namespace Hayat::Parser;\n"
          "\n"
          "Parser_EOF* p__eof;\n"
          "Parser_AnyChar* p__anychar;\n"
          , tab_basename
          );
    gMemPool->free(tab_filename);
}

static void cppOutDecl(void)
{
    // output rule decls
    TArrayIterator<Substr> ruleItr(&rules);
    char n[256];
    while (ruleItr.hasMore()) {
        Substr s = ruleItr.next();
        gpInp->copyStr(n, 256, s);
        fpOut(cppfp, "P_%s* p_%s;\n", n,n);
    }        
    TArrayIterator<Substr> expItr(op->expIdents());
    while (expItr.hasMore()) {
        Substr ss = expItr.next();
        gpInp->copyStr(n, 256, ss);
        TArray<hyu32>* precs = op->getPrecs(ss);
        TArrayIterator<hyu32> precItr(precs);
        fpOut(cppfp, "P_%s__0* p_%s__0;\n", n,n);
        while (precItr.hasMore()) {
            hyu32 prec = precItr.next();
            fpOut(cppfp, "P_%s__%d* p_%s__%d;\n", n,prec,n,prec);
        }
    }


    // output init func
    fpOut(cppfp,
          "\n"
          "void p__init(void)\n"
          "{\n"
          "  p__eof = new Parser_EOF();\n"
          "  p__anychar = new Parser_AnyChar();\n"
          );

    ruleItr.rewind();
    while (ruleItr.hasMore()) {
        Substr s = ruleItr.next();
        gpInp->copyStr(n, 256, s);
        fpOut(cppfp, "  p_%s = new P_%s();\n", n,n);
    }        
    expItr.rewind();
    while (expItr.hasMore()) {
        Substr ss = expItr.next();
        gpInp->copyStr(n, 256, ss);
        TArray<hyu32>* precs = op->getPrecs(ss);
        TArrayIterator<hyu32> precItr(precs);
        fpOut(cppfp, "  p_%s__0 = new P_%s__0();\n", n,n);
        while (precItr.hasMore()) {
            hyu32 prec = precItr.next();
            fpOut(cppfp, "  p_%s__%d = new P_%s__%d();\n", n,prec,n,prec);
        }
    }


    fpOut(cppfp, "\n");
    TArrayIterator<Substr> genRuleItr(&genRules);
    while (genRuleItr.hasMore()) {
        Substr s = genRuleItr.next();
        gpInp->copyStr(n, 256, s);
        fpOut(cppfp, "  p_%s->construct();\n", n);
    }        
    expItr.rewind();
    while (expItr.hasMore()) {
        Substr ss = expItr.next();
        gpInp->copyStr(n, 256, ss);
        TArray<hyu32>* precs = op->getPrecs(ss);
        TArrayIterator<hyu32> precItr(precs);
        fpOut(cppfp, "  p_%s__0->construct();\n", n);
        while (precItr.hasMore()) {
            hyu32 prec = precItr.next();
            fpOut(cppfp, "  p_%s__%d->construct();\n", n,prec);
        }
    }
    fpOut(cppfp, "}\n");

}

static void cppOutClose(void)
{
    if (! notCreateErrorCutMessage) {
        char n[256];
        int size = errMes_ruleName.size();
        for (int i = 0; i < size; ++i) {
            gpInp->copyStr(n, 256, errMes_ruleName[i]);
            fpOut(cppfp, "void P_%s::errorMessage(int errorId, hyu32 pos)\n{\n", n);
            fpOut(cppfp, "  switch (errorId) {\n");
            TArray<ErrMes_st>* nsArr = errMes_Number_String[i];
            for (int j = 0; (hyu32)j < nsArr->size(); ++j) {
                ErrMes_st& ns = nsArr->nth(j);
                fpOut(cppfp, "  case %d:\n", ns.number);
                gpInp->copyStr(n, 256, ns.string);
                fpOut(cppfp, "    errorCutMessage(pos, %s);\n", n);
                fpOut(cppfp, "    break;\n");
            }
            fpOut(cppfp, "  default:\n");
            fpOut(cppfp, "    GenParser::errorMessage(errorId, pos);\n");
            fpOut(cppfp, "  }\n");
            fpOut(cppfp, "}\n");
        }
    }
    fclose(cppfp);
}

static void tmplOutOpen(void)
{
    hyu32 bnsize = HMD_STRLEN(tab_basepath);
    char* tab_filename = gMemPool->allocT<char>(bnsize + 12, "fntt");
    HMD_STRNCPY(tab_filename, tab_basepath, bnsize+1);
    HMD_STRSCAT(tab_filename, ".tab.tmpl", bnsize+12);
    tmplfp = hmd_fopen(tab_filename, "w");

    HMD_STRNCPY(tab_filename, tab_basepath, bnsize+1);
    HMD_STRSCAT(tab_filename, ".tab.h", bnsize+12);
#ifdef WIN32
    if (MBCSConv::is_utf8_in()) {
        fpOut(tmplfp, "\xef\xbb\xbf");       // UTF-8 BOM 
    }
#endif
    fpOut(tmplfp,
          "#include \"%s\"\n"
          "\n"
          "\n"
          , tab_filename
          );
    gMemPool->free(tab_filename);
}

static void tmplOutClose(void)
{
    fclose(tmplfp);
}

static void upOutOpen(void)
{
    hyu32 bnsize = HMD_STRLEN(tab_basepath);
    char* tab_filename = gMemPool->allocT<char>(bnsize + 12, "fntu");
    HMD_STRNCPY(tab_filename, tab_basepath, bnsize+1);
    HMD_STRSCAT(tab_filename, ".tab.up", bnsize+12);
    upfp = hmd_fopen(tab_filename, "w");

    HMD_STRNCPY(tab_filename, tab_basepath, bnsize+1);
    HMD_STRSCAT(tab_filename, ".tab.h", bnsize+12);
#ifdef WIN32
    if (MBCSConv::is_utf8_in()) {
        fpOut(upfp, "\xef\xbb\xbf");       // UTF-8 BOM 
    }
#endif
    fpOut(upfp,
          "#include \"%s\"\n"
          "\n"
          "/* template file of uParse() functions */\n"
          "\n"
          , tab_filename
          );
    gMemPool->free(tab_filename);
}

static void upOutClose(void)
{
    fclose(upfp);
}

static void uaOutOpen(void)
{
    hyu32 bnsize = HMD_STRLEN(tab_basepath);
    char* tab_filename = gMemPool->allocT<char>(bnsize + 12, "fnta");
    HMD_STRNCPY(tab_filename, tab_basepath, bnsize+1);
    HMD_STRSCAT(tab_filename, ".tab.ua", bnsize+12);
    uafp = hmd_fopen(tab_filename, "w");

#ifdef WIN32
    if (MBCSConv::is_utf8_in()) {
        fpOut(uafp, "\xef\xbb\xbf");       // UTF-8 BOM 
    }
#endif
    fpOut(uafp,
          "/* template file of userActionFuncDef.h */\n\n"
          );
    gMemPool->free(tab_filename);
}

static void uaOutClose(void)
{
    fclose(uafp);
}

static void fpOutComment(FILE* fp, Substr str)
{
    hyu32 len = str.len();
    char* buf = gMemPool->allocT<char>(len+2, "fpoc");
    gpInp->copyStr(buf, len, str);
    char* p = buf;
    char* z = buf + len;
    while (p < z) {
        char* q = p;
        while (q < z) {
            if (*q == '\n' || *q == '\r' || *q == '\0')
                break;
            ++q;
        }
        if (q != p) {
            *q = '\0';
            fpOut(fp, "//@ %s\n", p);
        }
        while (++q < z) {
            if (*q != '\n' && *q != '\r' && *q != '\0')
                break;
        }
        p = q;
    }
    gMemPool->free(buf);
}


static hyu32 toNum(Substr& ss)
{
    const char* p = gpInp->addr(ss.startPos);
    const char* q = gpInp->addr(ss.endPos);
    hyu32 num = 0;
    while (p < q) {
        wchar_t c = gpInp->getCharAt(&p);
        if ((c < L'0') || (c > L'9'))
            break;
        num *= 10;
        num += c - L'0';
    }
    return num;
}



static int errorMesEntry(Substr& ident)
{
    int s = errMes_ruleName.size();
    int i;
    for (i = 0; i < s; ++i) {
        if (gpInp->cmpStr(errMes_ruleName[i], ident))
            break;
    }
    if (i >= s) {
        errMes_ruleName.add(ident);
        errMes_Number_String.add(new TArray<ErrMes_st>(1));
    }
    return i;
}







// comment for gaterRule.rb
//
//@ #
//@ # syntax definition of PEG itself
//@ #
//@
//@ $action     action()
//@
//


//============================================================
//@ Start <- Directives %Spc Rules %EOF
void P_Start::actionAtParse(SyntaxTree*) {}
void ST_Start::action(void)
{
    notCreateErrorCutMessage = false;
    tmplOutOpen();
    upOutOpen();
    uaOutOpen();
    get(0)->action();
    get(1)->action();
    tmplOutClose();
    upOutClose();
    uaOutClose();
}

//------------------------------------------------------------
//@

//============================================================
//@ Directives	<- (%Spc (Direc_userAction / Direc_ownMes))*
void P_Directives::actionAtParse(SyntaxTree*){}
void ST_Directives::action(void)
{
    fpOutComment(tmplfp, str);
    TArrayIterator<SyntaxTree*> itr(childs);
    while (itr.hasMore())
        itr.next()->get(0)->action();

    if (actionFuncs.size() == 0) {
        char* p = gMemPool->allocT<char>(7, "acfn");
        HMD_STRNCPY(p, "action", 7);
        actionFuncs.add(p);
        p = gMemPool->allocT<char>(1, "acfa");
        *p = '\0';
        actionArgs.add(p);
    }
    for (int i = 0; (hyu32)i < actionFuncs.size(); i++) {
        const char* funname = actionFuncs[i];
        const char* args = actionArgs[i];
        
        fpOut(uafp, "#ifdef m_AT_USERACTION_DECL\n");
        fpOut(uafp, "// declarations before class SyntaxTree\n");
        fpOut(uafp, "#endif\n");
        fpOut(uafp, "#ifdef m_IN_SYNTAXTREE_DECL\n");
        if (args[0] != '\0')
            fpOut(uafp, "virtual void %s(%s);\n", funname, args);
        else
            fpOut(uafp, "virtual void %s(void);\n", funname);
        fpOut(uafp, "#endif\n");
    }
}

//============================================================
//@ Direc_userAction <- %"$action" $1 %(" " / "\t")+ Ident %"(" ActionParam %")"
void P_Direc_userAction::actionAtParse(SyntaxTree*){}
void ST_Direc_userAction::action(void)
{
    Substr funnameSS = get(0)->str;
    char* funname = gMemPool->allocT<char>(funnameSS.len() + 4, "uafn");
    gpInp->copyStr(funname, funnameSS.len() + 4, funnameSS);
    actionFuncs.add(funname);
    Substr argsSS = get(1)->str;
    char* args = gMemPool->allocT<char>(argsSS.len() + 4, "uafa");
    gpInp->copyStr(args, argsSS.len() + 4, argsSS);
    actionArgs.add(args);
}

//============================================================
//@ ActionParam     <- (!(")" / "\n" / "\r") %.)*
void P_ActionParam::actionAtParse(SyntaxTree*){}
void ST_ActionParam::action(void){}

//============================================================
//@ Direc_ownMes    <- %"$ownErrorMessage"
void P_Direc_ownMes::actionAtParse(SyntaxTree*){}
void ST_Direc_ownMes::action(void)
{
    notCreateErrorCutMessage = true;
}


//============================================================
//@ Rules <- (Rule / OperatorDef / DummyRule / ErrorMesDef)*
void P_Rules::actionAtParse(SyntaxTree*){}
void ST_Rules::action(void)
{
    if (numChild() < 1)
        return;
    hOutOpen();
    cppOutOpen();
    op = new Operator();
    TArrayIterator<SyntaxTree*> itr(childs);
    bool errFlag = false;
    while (itr.hasMore()) {
        SyntaxTree* choice = itr.next();
        SyntaxTree* st = choice->get(0);
        SyntaxTree* ident;
        switch (choice->chooseNum) {
        case 0: // Rule
            ident = st->get(0);
            rules.add(ident->str);
            genRules.add(ident->str);
            break;
        case 1: // OperatorDef
            {
                ident = st->get(0)->get(0)->get(0);
                hyu32 num = toNum(st->get(0)->get(0)->get(2)->str);   // Number
                errFlag |= !op->entry(ident->str, num);
            }
            break;
        case 2: // DummyRule
            ident = st->get(0);
            rules.add(ident->str);
            break;
        case 3: // ErrorMesDef
            break;
        default:
            fprintf(stderr, "internal error: bug in ST_Rules\n");
            errFlag = true;
        }
    }

    if (! errFlag) {
        op->sort();
        cppOutDecl();
        itr.rewind();
        while (itr.hasMore())
            itr.next()->get(0)->action();
    }
    hOutClose();
    cppOutClose();
    delete op;
    rules.finalize();
    genRules.finalize();
    errMes_ruleName.finalize();
    for (int i = 0; (hyu32)i < actionFuncs.size(); i++) {
        gMemPool->free((void*)actionFuncs[i]);
        gMemPool->free((void*)actionArgs[i]);
    }
    actionFuncs.finalize();
    actionArgs.finalize();
    TArrayIterator< TArray<ErrMes_st>* > ensi(&errMes_Number_String);
    while (ensi.hasMore())
        delete ensi.next();
    errMes_Number_String.finalize();
    if (errFlag) {
        //delete output files ?
        fflush(stderr);
        exit(1);
    }
}


//------------------------------------------------------------
//@

//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------

static void ruleOut(SyntaxTree* st, bool bDummy)
{
    TArrayIterator<SyntaxTree*> itr(st->childs);
    SyntaxTree* ident = itr.next();
    char n[256];
    gpInp->copyStr(n, 256, ident->str);
    bool bOperand = op->isOperand(ident->str);
    fpOut(cppfp, "\n");
    if (bDummy)
        fpOut(cppfp, "/*\n");
    fpOut(cppfp, "void P_%s::construct(void) {\n", n);
    fpOut(cppfp, "  m_parser = ");
    if (bOperand) {
        hyu32 lowest = op->getPrecs(ident->str)->nth(0);
        fpOut(cppfp, "p_%s__%d;\n}\n", n,lowest);
        fpOut(cppfp, "\nvoid P_%s__0::construct(void) {\n", n);
        fpOut(cppfp, "  m_parser = ");
    }
    haveErrorCut = false;
    while (itr.hasMore()) {
        itr.next()->action();
    }
    fpOut(cppfp, ";\n}\n");
    const char* ug = "GEN";
    if (bDummy) {
        fpOut(cppfp, "*/\n");
        ug = "USER";
    }

    if (haveErrorCut)
        errorMesEntry(ident->str);
    const char* he = haveErrorCut ? "_E" : "";
    if (bOperand) {
        fpOut(hfp, "GEN_PARSER(%s);\n",n);
        fpOut(hfp, "%s_PARSER%s(%s__0);\n",ug,he,n);
    } else {
        fpOut(hfp, "%s_PARSER%s(%s);\n",ug,he,n);
    }

    fpOut(tmplfp, "\n//============================================================\n");
    fpOutComment(tmplfp, st->str);
    const char* opr = bOperand ? "__0" : "";
    if (bDummy) {
        // dummy rule
        fpOut(upfp, "\n//============================================================\n");
        fpOutComment(upfp, st->str);
        fpOut(upfp, "SyntaxTree* P_%s%s::uParse(void){}\n",n,opr);
    } else {
        fpOut(tmplfp, "void P_%s%s::actionAtParse(SyntaxTree*){}\n",n,opr);
        if (haveErrorCut) {
            fpOut(tmplfp, "void P_%s%s::errorMessage(int errorId, hyu32 pos)\n",n,opr);
            fpOut(tmplfp, "{ GenParser::errorMessage(errorId, pos); }\n");
        }
    }
    for (int i = 0; (hyu32)i < actionFuncs.size(); i++) {
        const char* funname = actionFuncs[i];
        const char* args = actionArgs[i];
        if (args[0] != '\0')
            fpOut(tmplfp, "void ST_%s%s::%s(%s){}\n",n,opr,funname,args);
        else
            fpOut(tmplfp, "void ST_%s%s::%s(void){}\n",n,opr,funname);
    }

}

//============================================================
//@ Rule <- Ident %Spc %"<-" %Spc Disjunction
void P_Rule::actionAtParse(SyntaxTree*){}
void ST_Rule::action(void)
{
    ruleOut(this, false);
}

//============================================================
//@ DummyRule <- Ident %Spc %"<%" %Spc Disjunction
void P_DummyRule::actionAtParse(SyntaxTree*){}
void ST_DummyRule::action(void)
{
    ruleOut(this, true);
}


//============================================================
//@ ErrorMesDef     <- Ident %"$" Number %Spc %"=" %Spc String %Spc
void P_ErrorMesDef::actionAtParse(SyntaxTree*){}
void ST_ErrorMesDef::action(void)
{
    int i = errorMesEntry(get(0)->str);
    TArray<ErrMes_st>* nsArr = errMes_Number_String[i];
    ErrMes_st* ns = nsArr->addSpaces(1);
    ns->number = toNum(get(1)->str);
    ns->string = get(2)->str;
}

//============================================================
//@ Disjunction <- Conjunction (%"/" %Spc Conjunction)*
void P_Disjunction::actionAtParse(SyntaxTree*){}
void ST_Disjunction::action(void)
{
    TArrayIterator<SyntaxTree*> itr(childs);
    itr.next()->action();
    while (itr.hasMore()) {
        fpOut(cppfp, "->choice(");
        itr.next()->action();
        fpOut(cppfp, ")");
    }
}

//============================================================
//@ Conjunction <- (PrefixExp / ErrorCut)+
void P_Conjunction::actionAtParse(SyntaxTree*){}
void ST_Conjunction::action(void)
{
    TArrayIterator<SyntaxTree*> itr(childs);
    itr.next()->get(0)->action();
    while (itr.hasMore()) {
        fpOut(cppfp, "->seq(");
        itr.next()->get(0)->action();
        fpOut(cppfp, ")");
    }
}

//============================================================
//@ PrefixExp <- (AndPred / NotPred / NoTree)? SuffixExp
void P_PrefixExp::actionAtParse(SyntaxTree*){}
void ST_PrefixExp::action(void)
{
    if (numChild() == 1) {
        get(0)->action();
    } else {
        get(1)->action();
        get(0)->get(0)->action();
    }
}

//============================================================
//@ SuffixExp <- Term %Spc (Optional / ZeroOrMore / OneOrMore)?
void P_SuffixExp::actionAtParse(SyntaxTree*){}
void ST_SuffixExp::action(void)
{
    if (numChild() == 1) {
        get(0)->action();
    } else {
        get(0)->action();
        get(1)->get(0)->action();
    }
}

//============================================================
//@ Term <- EndOfFile / Term1 / Term2 / String / Token / Range / Char / AnyChar
void P_Term::actionAtParse(SyntaxTree*){}
void ST_Term::action(void)
{
    get(0)->get(0)->action();
}

//============================================================
//@ Term1 <- %Ident !(Spc "<")
void P_Term1::actionAtParse(SyntaxTree*){}
void ST_Term1::action(void)
{
    hyu32 len = str.len();
    char* n = (char*)HMD_ALLOCA(len+1);
    gpInp->copyStr(n, len, str);
    fpOut(cppfp, "p_%s", n);
}

//============================================================
//@ Term2 <- %"(" %Spc Disjunction %")"
void P_Term2::actionAtParse(SyntaxTree*){}
void ST_Term2::action(void)
{
    get(0)->action();
}

//============================================================
//@ ErrorCut <- %"$" $1 Number %Spc
void P_ErrorCut::actionAtParse(SyntaxTree*){}
void ST_ErrorCut::action(void)
{
    hyu32 errId = toNum(get(0)->str);
    fpOut(cppfp, "(new Parser_ErrorCut(%d))", errId);
    haveErrorCut = true;
}


//------------------------------------------------------------
//@

//============================================================
//@ OperatorDef	<- UnaryOp / BinaryOp / TernaryOp
void P_OperatorDef::actionAtParse(SyntaxTree*){}
void ST_OperatorDef::action(void)
{
    get(0)->get(0)->action();
    fpOut(tmplfp, "\n//============================================================\n");
    fpOutComment(tmplfp, str);
}

//============================================================
//@ UnaryOp         <- Ident %Spc %"<<" %Spc ('prefix' / 'postfix') $1 %Spc Number $2 %Spc Disjunction
void P_UnaryOp::actionAtParse(SyntaxTree*){}
void ST_UnaryOp::action(void)
{
    SyntaxTree* ident = get(0);
    char id[256];
    gpInp->copyStr(id, 256, ident->str);
    hyu32 prec = toNum(get(2)->str);
    hyu32 higher = op->getHigher(ident->str, prec);
    const char* name;
    switch (get(1)->chooseNum) {
    case 0:
        name = "Prefix";        break;
    case 1:
        name = "Postfix";        break;
    default:
        fpOut(stderr, "internal error: bug in ST_UnaryOp\n");
        fflush(stderr);
        exit(1);
    }
    fpOut(hfp, "OPERATOR_PARSER(%s__%d);\n",id,prec);
    fpOut(cppfp, "\nvoid P_%s__%d::construct(void) {\n", id,prec);
    fpOut(cppfp, "  m_parser = new Parser_%s(p_%s__%d, p_%s__%d, ", name, id,prec, id,higher);
    get(3)->action();
    fpOut(cppfp, ");\n}\n");
}


//============================================================
//@ BinaryOp	<- Ident %Spc %"<<" %Spc ('infixl' / 'infixr' / 'infixn') $1 %Spc Number $2 %Spc Disjunction
void P_BinaryOp::actionAtParse(SyntaxTree*){}
void ST_BinaryOp::action(void)
{
    SyntaxTree* ident = get(0);
    char id[256];
    gpInp->copyStr(id, 256, ident->str);
    hyu32 prec = toNum(get(2)->str);
    hyu32 higher = op->getHigher(ident->str, prec);
    const char* name;
    switch (get(1)->chooseNum) {
    case 0:
        name = "InfixL";        break;
    case 1:
        name = "InfixR";        break;
    case 2:
        name = "InfixN";        break;
    default:
        fpOut(stderr, "internal error: bug in ST_BinaryOp\n");
        fflush(stderr);
        exit(1);
    }
    fpOut(hfp, "OPERATOR_PARSER(%s__%d);\n",id,prec);
    fpOut(cppfp, "\nvoid P_%s__%d::construct(void) {\n", id,prec);
    fpOut(cppfp, "  m_parser = new Parser_%s(p_%s__%d, p_%s__%d, ", name, id,prec, id,higher);
    get(3)->action();
    fpOut(cppfp, ");\n}\n");
}


//============================================================
//@ TernaryOp	<- Ident %Spc %"<<" %Spc 'ternary' $1 %Spc Number $2 %Spc PrefixExp %Spc PrefixExp %Spc
void P_TernaryOp::actionAtParse(SyntaxTree*){}
void ST_TernaryOp::action(void)
{
    SyntaxTree* ident = get(0);
    char id[256];
    gpInp->copyStr(id, 256, ident->str);
    hyu32 prec = toNum(get(2)->str);
    hyu32 higher = op->getHigher(ident->str, prec);
    fpOut(hfp, "OPERATOR_PARSER(%s__%d);\n",id,prec);
    fpOut(cppfp, "\nvoid P_%s__%d::construct(void) {\n", id,prec);
    fpOut(cppfp, "  m_parser = new Parser_Ternary(p_%s__%d, p_%s__%d, ", id,prec, id,higher);
    get(3)->action();
    fpOut(cppfp, ", ");
    get(4)->action();
    fpOut(cppfp, ");\n}\n");
}

//============================================================
//@ Number		<% %[0-9]+
void ST_Number::action(void){}

//------------------------------------------------------------
//@

//============================================================
//@ AndPred <- %"&" %Spc
void P_AndPred::actionAtParse(SyntaxTree*){}
void ST_AndPred::action(void)
{
    fpOut(cppfp, "->andPred()");
}

//============================================================
//@ NotPred <- %"!" %Spc
void P_NotPred::actionAtParse(SyntaxTree*){}
void ST_NotPred::action(void)
{
    fpOut(cppfp, "->notPred()");
}

//============================================================
//@ NoTree <- %"%" %Spc
void P_NoTree::actionAtParse(SyntaxTree*){}
void ST_NoTree::action(void)
{
    fpOut(cppfp, "->noTree()");
}

//============================================================
//@ Optional <- %"?" %Spc
void P_Optional::actionAtParse(SyntaxTree*){}
void ST_Optional::action(void)
{
    fpOut(cppfp, "->optional()");
}

//============================================================
//@ ZeroOrMore <- %"*" %Spc
void P_ZeroOrMore::actionAtParse(SyntaxTree*){}
void ST_ZeroOrMore::action(void)
{
    fpOut(cppfp, "->star()");
}

//============================================================
//@ OneOrMore <- %"+" %Spc
void P_OneOrMore::actionAtParse(SyntaxTree*){}
void ST_OneOrMore::action(void)
{
    fpOut(cppfp, "->plus()");
}

//------------------------------------------------------------
//@

//============================================================
//@ Ident <% %Ident1 %Ident2*
void ST_Ident::action(void){}

//============================================================
//@ NotTokenPred <% !Ident2
void ST_NotTokenPred::action(void){}

//------------------------------------------------------------
//@

//============================================================
//@ EndOfFile <- %'EOF'
void P_EndOfFile::actionAtParse(SyntaxTree*){}
void ST_EndOfFile::action(void)
{
    fpOut(cppfp, "p__eof");
}

//============================================================
//@ AnyChar <- %"."
void P_AnyChar::actionAtParse(SyntaxTree*){}
void ST_AnyChar::action(void)
{
    fpOut(cppfp, "p__anychar");
}

//============================================================
//@ String <- %"\"" (!"\"" %XChar)+ %"\""
void P_String::actionAtParse(SyntaxTree*){}
void ST_String::action(void)
{
    char n[256];
    gpInp->copyStr(n, 256, str);
    fpOut(cppfp, "(new Parser_String(L%s))", n);
}

//============================================================
//@ Token <- %"'" Ident %"'"
void P_Token::actionAtParse(SyntaxTree*){}
void ST_Token::action(void)
{
    SyntaxTree* st = get(0);
    char n[256];
    gpInp->copyStr(n, 256, st->str);
    fpOut(cppfp, "(new Parser_String(L\"%s\"))->seq(p_NotTokenPred->noTree())", n);
}

//============================================================
//@ Range <- %"[" XChar %"-" XChar %"]"
void P_Range::actionAtParse(SyntaxTree*){}
void ST_Range::action(void)
{
    fpOut(cppfp, "(new Parser_RangeChar(");
    get(0)->action();
    fpOut(cppfp, ",");
    get(1)->action();
    fpOut(cppfp, "))");
}

//============================================================
//@ XChar <- XCharH / XCharE / XCharA
void P_XChar::actionAtParse(SyntaxTree*){}
void ST_XChar::action(void)
{
    get(0)->get(0)->action();
}

//============================================================
//@ XCharH <- %"\\" %("x" / "X") Hex1 Hex1
static int wcharhex2dec(wchar_t wc)
{
    int v = 0;
    if ((wc >= L'0') && (wc <= L'9'))
        v = wc - L'0';
    else if ((wc >= L'A') && (wc <= L'F'))
        v = wc - L'A' + 10;
    else if ((wc >= L'a') && (wc <= L'f'))
        v = wc - L'A' + 10;
    else
        HMD_FATAL_ERROR("wcharhex2dec() never take illegal arg");
    return v;
}
void P_XCharH::actionAtParse(SyntaxTree*){}
void ST_XCharH::action(void)
{
    int i = 16 * wcharhex2dec(gpInp->getCharAt(get(0)->str.startPos));
    i += wcharhex2dec(gpInp->getCharAt(get(1)->str.startPos));
    fpOut(cppfp, "((wchar_t)%d)", i);
}

//============================================================
//@ XCharU <- %"\\" %("u" / "U") Hex1 Hex1 Hex1 Hex1
void P_XCharU::actionAtParse(SyntaxTree*){}
void ST_XCharU::action(void)
{
    int i = 0;
    for (int j = 0; j < 4; ++j) {
        i *= 16;
        i += wcharhex2dec(gpInp->getCharAt(get(j)->str.startPos));
    }
    fpOut(cppfp, "((wchar_t)%d)", i);
}

//============================================================
//@ XCharE <- %"\\" .
void P_XCharE::actionAtParse(SyntaxTree*){}
void ST_XCharE::action(void)
{
    wchar_t wc = gpInp->getCharAt(get(0)->str.startPos);
    fpOut(cppfp, "L'\\%lc'", wc);
}

//============================================================
//@ XCharA <- !("\n" / "\r") %.
void P_XCharA::actionAtParse(SyntaxTree*){}
void ST_XCharA::action(void)
{
    wchar_t wc = gpInp->getCharAt(str.startPos);
    fpOut(cppfp, "L'%lc'", wc);
}

//============================================================
//@ Hex1 <- %[0-9] / %[A-F] / %[a-f]
void P_Hex1::actionAtParse(SyntaxTree*){}
void ST_Hex1::action(void){}

//============================================================
//@ Spc <- %(Comment / " " / "\t" / "\n" / "\r" / Comment)*
void P_Spc::actionAtParse(SyntaxTree*){}
void ST_Spc::action(void){}

//============================================================
//@ Comment <% %"#" %XCharA* %("\n" / "\r")+
void ST_Comment::action(void){}
