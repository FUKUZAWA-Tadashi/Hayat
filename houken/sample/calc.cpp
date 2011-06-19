
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "calc.tab.h"
#include "hpPrintSyntaxTree.h"


using namespace Hayat::Parser;

void SyntaxTree::calc(int*) {}


//============================================================
//@ Start <- %Spc Exp %EOF
void P_Start::actionAtParse(SyntaxTree*){}
void ST_Start::calc(int* vp)
{
    get(0)->calc(vp);
}


//------------------------------------------------------------
//@

//============================================================
//@ Exp <- Number / Paren
//expanded as below:
// Exp <- Exp__5
// Exp__0 <- Number / Paren
void P_Exp::actionAtParse(SyntaxTree*){}
void ST_Exp::calc(int* vp)
{
    get(0)->calc(vp);
}
void P_Exp__0::actionAtParse(SyntaxTree*){}
void ST_Exp__0::calc(int* vp)
{
    get(0)->get(0)->calc(vp);
}

//============================================================
//@ Exp << infixl 10 ("+" / "-") %Spc
//expanded nearly as below (avoiding left recursion):
// Exp__10 <- (Exp__10 (("+" / "-") %Spc))? Exp__20
void ST_Exp__10::calc(int* vp)
{
    int rh;
    get(0)->calc(vp);
    if (numChild() >= 3) {
        get(2)->calc(&rh);
        switch (get(1)->chooseNum) {
        case 0: // "+"
            *vp += rh;
            break;
        case 1: // "-"
            *vp -= rh;
            break;
        default:
            printf("error on P_Exp__10\n");
            *vp = 0;
        }
    }
    deleteAllChild();
}


//============================================================
//@ Exp << infixl 20 ("*" / "/") %Spc
//expanded nearly as below (avoiding left recursion):
// Exp__20 <- (Exp__20 (("*" / "/") %Spc))? Exp__30
void ST_Exp__20::calc(int* vp)
{
    int rh;
    get(0)->calc(vp);
    if (numChild() >= 3) {
        get(2)->calc(&rh);
        switch (get(1)->chooseNum) {
        case 0: // "*"
            *vp *= rh;
            break;
        case 1: // "/"
            *vp /= rh;
            break;
        default:
            printf("error on P_Exp__20\n");
            *vp = 0;
        }
    }
    deleteAllChild();
}


//============================================================
//@ Exp << infixr 30 "**" %Spc
//expanded as below:
// Exp__30 <- Exp__40 ("**" %Spc Exp__30)?
void ST_Exp__30::calc(int* vp)
{
    int rh;
    get(0)->calc(vp);
    if (numChild() >= 3) {
        get(2)->calc(&rh);
        *vp = (int)powf((float)*vp, (float)rh);
    }
    deleteAllChild();
}

//============================================================
//@ Exp << prefix 40 ("+" / "-") %Spc
//expanded as below:
// Exp__40 <- (("+" / "-") %Spc)? Exp__0
void ST_Exp__40::calc(int* vp)
{
    if (numChild() == 1) {
        get(0)->calc(vp);
    } else {
        get(1)->calc(vp);
        switch (get(0)->chooseNum) {
        case 0: // "+"
            break;
        case 1: // "-"
            *vp = - *vp;
            break;
        default:
            printf("error on P_Exp__40\n");
            *vp = 0;
        }
    }
    deleteAllChild();
}

//============================================================
//@ Exp << ternary 5 %("?" Spc) %(":" Spc)
//expanded as below:
// Exp__5 <- Exp__10 (%("?" Spc) Exp__10 %(":" Spc) Exp__10)?
void ST_Exp__5::calc(int* vp)
{
    get(0)->calc(vp);

    if (numChild() >= 3) {
        if (*vp != 0)
            get(1)->calc(vp);
        else
            get(2)->calc(vp);
    }
    deleteAllChild();
}

//------------------------------------------------------------
//@
//@

//============================================================
//@ Number <- %[0-9]+ %Spc
void P_Number::actionAtParse(SyntaxTree*){}
void ST_Number::calc(int* vp)
{
    int res = 0;
    for (hyu32 pos = str.startPos; pos < str.endPos; ++pos) {
        int c = gpInp->getCharAt(pos) - L'0';
        if ((c < 0) || (c > 9))
            break;
        res *= 10;
        res += c;
    }
    *vp = res;
}


//============================================================
//@ Paren <- %"(" %Spc Exp %")" %Spc
void P_Paren::actionAtParse(SyntaxTree*){}
void ST_Paren::calc(int* vp)
{
    get(0)->calc(vp);
}


//============================================================
//@ Spc <- %(" " / "\t")*
void P_Spc::actionAtParse(SyntaxTree*){}
void ST_Spc::calc(int*)
{
    // never called because all used as %Spc
    printf("error on P_Spc\n");
}









//============================================================
//
//
//                      main routine
//
//
//============================================================


// for machdep.cpp
const char** HMD_LOADPATH = NULL;


void printError(InputBuffer* inp)
{
    int n = Parser::numErrors();
    for (--n; n >= 0; --n) {
        Parser::Error_t err = Parser::getError(n);
        char pbuf[128];
        inp->sprintSourceInfo(pbuf, 128, err.pos);
        const char* nam = err.parser->name();
        if ((nam != NULL) && (*nam != '('))
            printf("parser=%s, %s\n", nam, pbuf);
    }
}


int calc(const char* exp, bool ptree)
{
    int result = 0;
    StringInputBuffer inp(exp);
    gpInp = &inp;
    SyntaxTree::initializePool(true);
    Parser::initialize(40);
    //Parser::setPrintIntermediateLevel(1);
    SyntaxTree* st = p_Start->parse();
    if (st->isValidTree()) {
        if (ptree)
            printSyntaxTree(st, gpInp, 0);
        st->calc(&result);
    } else {
        printf("parse error\n");
        printError(gpInp);
    }
    Parser::finalize();
    SyntaxTree::finalizePool();
    return result;
}

int main(int argc, const char* argv[])
{
    bool done = false;
    bool ptree = false;
    bool verbose = false;
    void* mem = HMD_ALLOC(1 * 1024 * 1024);
    MemPool::initGMemPool(mem, 1 * 1024 * 1024);
    for (int i = 1; i < argc; ++i) {
        if (HMD_STRCASECMP(argv[i], "-p") == 0)
            ptree = true;
        else if (HMD_STRCASECMP(argv[i], "-v") == 0)
            verbose = true;
        else {
            int v = calc(argv[i], ptree);
            if (verbose) {
                printf("exp: %s\n", argv[i]);
                printf("result: %d\n", v);
            } else {
                printf("%d\n", v);
            }
            done = true;
        }
    }
    HMD_FREE(mem);

    if (!done) {
        printf("expression required as argument\n");
        return 1;
    }
    return 0;
}

