
#include "pgPegParser.h"
#include "hpPrintSyntaxTree.h"



#define THIS_NAME       "houken"
#define VERSION         "version 0.2"
#if defined(__CYGWIN__) || defined(m_WIN32)
#define EXE_FILENAME    "houken.exe"
#else
#define EXE_FILENAME    "houken"
#endif

char* tab_basepath;     // export to PegParserAction.cpp
char* tab_basename;     // export to PegParserAction.cpp

// for machdep.cpp
const char** HMD_LOADPATH = NULL;


using namespace Hayat::Common;
using namespace Hayat::Parser;


void printError(InputBuffer* inp)
{
    int n = Parser::numErrors();
    for (--n; n >= 0; --n) {
        Parser::Error_t err = Parser::getError(n);
        char pbuf[128];
        inp->sprintSourceInfo(pbuf, 128, err.pos);
        const char* nam = err.parser->name();
        if ((nam != NULL) && (*nam != '('))
            HMD_PRINTF("parser=%s, %s\n", nam, pbuf);
    }
}

void errorCutMessage(hyu32 pos, const char* message)
{
    SourceInfo si;
    gpInp->buildSourceInfo(&si, pos);
    HMD_PRINTF("%s:%d:%d: %s\n",
               si.fname, si.line, si.col,
               message);
}

void usage(void)
{
    HMD_PRINTF("usage: " EXE_FILENAME " [options] PEGfile\n");
    HMD_PRINTF("option:\n");
#ifdef m_WIN32
    HMD_PRINTF("  -c <charset>: set PEGfile's character set (CodePage number)\n");
#else
    HMD_PRINTF("  -c <charset>: set PEGfile's character set (locale)\n");
#endif
    HMD_PRINTF("  -m : not use memoization\n");
    HMD_PRINTF("  -M <size> : memory allocate size for " EXE_FILENAME " (default 1M)\n");
    HMD_PRINTF("  -s : print syntax tree\n");
    HMD_PRINTF("  -d : print intermediate parsing result\n");
    HMD_PRINTF("  -d2 : print detail intermediate parsing result\n");
}

int main(int argc, const char* argv[])
{
    const char* pegFileName = NULL;
    bool bPrintSyntaxTree = false;
    int  printIntermediateLevel = 0;
    bool bUseMemoize = true;
    int exitValue = 0;
    const char* ctype = NULL;
    hyu32 memSize = 1 * 1024 * 1024;

    HMD_PRINTF("PEG parser generator " THIS_NAME " " VERSION "\n");
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-')
            for (const char* p = &argv[i][1]; *p != '\0'; ++p) {
                switch (*p) {
                case 'c':
                    if (p[1] != '\0')
                        ctype = &p[1];
                    else if (++i < argc)
                        ctype = argv[i];
                    else {
                        HMD_PRINTF("-c option needs <charset>\n");
                        usage();
                        return 1;
                    }
                    break;
                case 's':
                    bPrintSyntaxTree = true;
                    break;
                case 'd':
                    if (p[1] >= '0' && p[1] <= '9')
                        printIntermediateLevel = *++p - '0';
                    else
                        printIntermediateLevel = 1;
                    break;
                case 'm':
                    bUseMemoize = false;
                    break;
                case 'M':
                    if (argv[i][2] != '\0')
                        p = &argv[i][2];
                    else if (++i < argc)
                        p = &argv[i][0];
                    else {
                        HMD_PRINTF("-M option needs <memorySize>\n");
                        usage();
                        return 1;
                    }
                    memSize = 0;
                    while('0' <= *p && *p <= '9') {
                        memSize *= 10;
                        memSize += (*p++ - '0');
                    }
                    if (*p == '\0') {
                        --p;
                    } else if (*p == 'k' || *p == 'K') {
                        memSize *= 1024;
                    } else if (*p == 'm' || *p == 'M') {
                        memSize *= 1024 * 1024;
                    } else {
                        printf("bad parameter for -M\n");
                        memSize = 1 * 1024 * 1024;
                    }
                    break;
                    
                default:
                    HMD_PRINTF("unknown option: %s\n", argv[i]);
                    usage();
                    return 1;
                }
            }
        else {
            if (pegFileName != NULL) {
                HMD_PRINTF("error: only 1 PEGfile required\n");
                usage();
                return 1;
            }
            pegFileName = argv[i];
        }
    }

    if (pegFileName == NULL) {
        HMD_PRINTF("error: PEGfile required\n");
        usage();
        return 1;
    }

    void* houkenMemory = HMD_ALLOC(memSize);
    MemPool::initGMemPool(houkenMemory, memSize);
    SyntaxTree::initializePool(bUseMemoize);

    const char* p = strrchr(pegFileName, '.');
    hyu32 len = HMD_STRLEN(pegFileName);
    if (HMD_STRCASECMP(p, ".peg") == 0)
        len -= 4;
    tab_basepath = gMemPool->allocT<char>(len+1, "fnbp");
    HMD_STRNCPY(tab_basepath, pegFileName, len+1);
    tab_basepath[len] = '\0';
    p = strrchr(tab_basepath, PATH_DELIM);
    if (p == NULL) {
        p = tab_basepath;
    } else {
        ++p;
        len -= (hyu32)(p - tab_basepath);
    }
    tab_basename = gMemPool->allocT<char>(len+1, "fnbn");
    HMD_STRNCPY(tab_basename, p, len+1);
    tab_basename[len] = '\0';
    HMD_PRINTF("generating %s.tab.* from %s\n", tab_basename, pegFileName);

    FileInputBuffer inp(pegFileName);
    gpInp = &inp;
    MBCSConv::initialize(ctype);
    Parser::initialize(40);
    Parser::setUseMemoize(bUseMemoize);
    Parser::setPrintIntermediateLevel(printIntermediateLevel);
    SyntaxTree* st = p_Start->parse();
    if (st->isValidTree()) {
        if (bPrintSyntaxTree)
            printSyntaxTree(st, &inp, 0);
        st->action();
        HMD_PRINTF("done.\n");
        delete st;
    } else {
        HMD_PRINTF("parse error\n");
        printError(&inp);
        exitValue = 1;
    }

    gMemPool->free(tab_basepath);

    inp.finalize();
    Parser::finalize();
    SyntaxTree::finalizePool();

    HMD_FREE(houkenMemory);

    return exitValue;
 }
