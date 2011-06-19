/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


/*
 * �G���W�����f�o�b�O����ǂ�ł��炸�A�V���{����\�[�X�R�[�h�ʒu��
 * �킩��Ȃ��f�o�b�O�\���������ꍇ�ɁA�������͂��Ă킩��₷��
 * �\������c�[���ł��B
 * �W�����͂�1�s����͂��āA�W���o�͂ɏo�͂��܂��B
 *
 *
 * gokou [-I path] [-M memsize] [-f symbols.sym] [-s]
 *
 * -I path
 *      �f�o�b�O����ǂݍ��ރp�X��ǉ����܂��B path �y�� path/out ��
 *      �p�X���ɒǉ�����܂��B�p�X���ɂ��� symbols.sym�A*.hdb�A*.hy 
 *      �t�@�C����T���ēǂݍ��݂܂��B������ -I �I�v�V����������ꍇ�A
 *      ��Ɏw�肵���p�X���猟�����n�߂܂��B
 *      �����w�肵�Ȃ��Ă� . �� ./out ���Ō�ɒǉ�����܂��B
 *
 * -M memsize
 *      gokou���ŏ��Ɋm�ۂ��郁�����ʂ�ύX���܂��B
 *      �o�C�g�����w�肵�܂��B���� K ��t����� 1024�{�A
 *      M ��t����� 1024*1024�{ ���ꂽ���l�Ɖ��߂��܂��B
 *      �f�t�H���g�� 16M �ł��B
 *
 * -f symbols.sym
 *      ���[�h����symbols.sym�t�@�C�����w�肵�܂��B
 *      �w�肪������΁A�p�X���猟�����ă��[�h���܂��B
 *
 * -s
 *      �\�[�X�R�[�h�ʒu�����o�͂���ۂɁA�\�[�X�R�[�h�̊Y������ӏ���
 *      �ǂݍ���œ����ɏo�͂��܂��B
 *
 */




#include "hySymbolTable.h"
#include "hyBMap.h"
#include "hpSourceInfo.h"
#include "hyDebugSourceInfo.h"
#include "hyCCommandLine.h"


#define DEBUGINFO_EXT   ".hdb"
#define DEFAULT_MEMSIZE (16 * 1024 * 1024)


const char** HMD_LOADPATH = NULL;
hyu32 numLoadPath = 0;

bool bPrintSource = false;

using namespace Hayat::Common;
using namespace Hayat::Engine;
using namespace Hayat::Compiler;




BMap<SymbolID_t, DebugInfos*>  m_debInfos(0);




static void skipLine(hyu8** ppc)
{
    hyu8 c1 = **ppc;
    hyu8 c2;
    while (c1 != '\0') {
        c2 = *++*ppc;
        if (c1 == '\n' || c1 == '\r') {
            if ((c1 == '\r' && c2 == '\n') || (c1 == '\n' && c2 == '\r'))
                ++*ppc;
            return;
        }
        c1 = c2;
    }
}

DebugInfos* getDebInfo(const char* package)
{
    SymbolID_t pkgid = gSymbolTable.symbolID(package);
    DebugInfos** pdsi = m_debInfos.find(pkgid);
    if (pdsi != NULL)
        return *pdsi;
    hyu32 len = HMD_STRLEN(package);
    TArray<char>* fname = new TArray<char>(len + HMD_STRLEN(DEBUGINFO_EXT) + 1);
    memcpy(fname->addSpaces(len), package, len);
    len = HMD_STRLEN(DEBUGINFO_EXT);
    memcpy(fname->addSpaces(len), DEBUGINFO_EXT, len);
    fname->add('\0');

    return new DebugInfos(fname->top());
}

DebugSourceInfo_t resolveAddr(const char* package, hyu32 addr)
{
    DebugInfos* dsi = getDebInfo(package);
    if (dsi == NULL) return NULL_DebugSourceInfo;
    return dsi->searchDebugInfo(addr);
}

void printSource(DebugSourceInfo_t si)
{
    if (si.path == NULL) return;
    HMD_PRINTF("%s:%d:%d\n", si.path, si.line, si.col);
    if (!bPrintSource) return;

    hyu8* fileBuf;
    hyu32 len = hmd_loadFile_inPath(si.path, &fileBuf);
    if (len == 0) {
        HMD_PRINTF("could not load file '%s'\n", si.path);
        return;
    }
    hyu8* endp = fileBuf + len;
    hyu8* p = fileBuf;
    for (hyu32 i = 1; i < si.line; ++i) {
        skipLine(&p);
        if (p >= endp) {
            p = endp;
            break;
        }
    }
    hyu8* q = p;
    if (p < endp) {
        skipLine(&q);
        if (q >= endp)
            q = endp;
    }
    len = q - p;
    char* buf = (char*)HMD_ALLOCA(len+2);
    char* r = buf;
    while (p < q) {
        if (*p == '\n' || *p == '\r')
            break;
        if (*p == '\t') {
            *r++ = ' ';
            p++;
        } else {
            *r++ = (char)*p++;
        }
    }
    *r = '\n';
    *++r = '\0';
    HMD_PRINTF((char*)buf);

    for (hyu32 i = 0; i < si.col; ++i)
        HMD_PRINTF("-");
    HMD_PRINTF("^\n");

    hmd_freeFileBuf(fileBuf);
}


static int getNum(const char** pStr, int base = 10)
{
    char* endp;
    long int n = strtol(*pStr, &endp, base);
    if (*pStr == endp)
        return -1;              // ��������������
    *pStr = endp;
    return (int)n;
}

// "{����}" ���p�[�X���Đ�����Ԃ��B�p�[�X���s�� -1 ��Ԃ��B
// �����Ȃ� pLine �� '}' �̎����w���B
static int getSymIDNum(const char** pLine)
{
    const char* p = *pLine;
    if (*p++ != '{') return -1; // '{'�Ŏn�܂��ĂȂ�����
    int n = getNum(&p, 10);
    if (n < 0) return -1;       // ��������������
    if (*p++ != '}') return -1; // '}'�ŕ����ĂȂ�����
    *pLine = p;
    return n;
}
TArray<char>* replaceSymbol(const char* line)
{
    TArray<char>* buf = new TArray<char>(HMD_STRLEN(line)+1);
    const char* p = line;
    char c;
    while ('\0' != (c = *p)) {
        if (c == '{') {
            int n = getSymIDNum(&p);
            if (n >= 0) {
                const char* s = (const char*)gSymbolTable.id2str((SymbolID_t)n);
                if (s != NULL) {
                    hyu32 len = HMD_STRLEN(s);
                    memcpy(buf->addSpaces(len), s, len);
                    continue;
                }
            }
        }
        buf->add(c);
        ++p;
    }
    buf->add('\0');
    return buf;
}

static inline bool isIdent(char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
}
static const char* skipSpace(const char* p)
{
    while (*p == ' ' || *p == '\t')
        ++p;
    return p;
}

void printResolvedLine(const char* line)
{
    const char* p1 = skipSpace(line);
    const char* h;
    const char* pkg1;
    const char* pkg2;
    int adr;
    int n = getNum(&p1, 10);
    if (n >= 0 && *p1 == ':') {
        h = ++p1;
    } else {
        p1 = line;
        h = NULL;
    }
    pkg2 = pkg1 = skipSpace(p1);
    while (isIdent(*pkg2)) ++pkg2;
    if (pkg2 == pkg1) goto through;
    p1 = pkg2;
    if (*p1++ != ':') goto through;
    if (*p1++ != ':') goto through;
    p1 = skipSpace(p1);
    adr = getNum(&p1, 16);
    if (adr < 0) goto through;
    
    {
        char* buf = (char*)HMD_ALLOCA(pkg2 - pkg1 + 1);
        memcpy(buf, pkg1, pkg2 - pkg1);
        buf[pkg2 - pkg1] = '\0';
        DebugSourceInfo_t si = resolveAddr(buf, (hyu32)adr);
        if (si.path == NULL) goto through;

        if (h != NULL) {
            char* buf2 = (char*)HMD_ALLOCA(h-line+1);
            memcpy(buf2, line, h-line);
            buf2[h-line] = '\0';
            HMD_PRINTF(buf2);
        }
        printSource(si);
        HMD_PRINTF(p1);
    }
    return;

 through:
    HMD_PRINTF(line);
}



void usage(bool detail = false)
{
    HMD_PRINTF("GOKOU :- Hayat debug message analyzer\n");
    HMD_PRINTF("usage: gokou [-I path] [-M memsize] [-f symbols.sym] [-s]\n");
    if (detail) {
        HMD_PRINTF("  -I : add path for searching debug info files\n");
        HMD_PRINTF("  -M : set memory size   (ex.) -M8M , -M500K\n");
        HMD_PRINTF("  -f : specify symbols.sym file\n");
        HMD_PRINTF("  -s : print source line\n");
        HMD_PRINTF("  -h : print this\n");
    }
}

int main(int argc, const char* argv[])
{
    hyu32 memSize = DEFAULT_MEMSIZE;
    const char* param;
    const char* symbolFile = NULL;
    const char* incPath[32];
    int numIncPath = 0;
    void* hayatMemory = NULL;

    numLoadPath = 0;
    HMD_LOADPATH = NULL;
    bPrintSource = false;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
            case 'I':
                // include�p�X
                if (numIncPath >= 32) {
                    usage();
                    HMD_PRINTF("too many include path (must <=32)\n");
                    return 1;
                }
                param = getParam(i, argc, argv, "-I");
                if (param == NULL) {
                    usage();
                    HMD_PRINTF("parameter for -I is not specified\n");
                    return 1;
                }
                incPath[numIncPath++] = param;
                break;

            case 'M':
                param = getParam(i, argc, argv, "-M");
                if (param == NULL) {
                    usage();
                    HMD_PRINTF("parameter for -M is not specified\n");
                    return 1;
                } else {
                    memSize = 0;
                    while('0' <= *param && *param <= '9') {
                        memSize *= 10;
                        memSize += (*param++ - '0');
                    }
                    if (*param == 'k' || *param == 'K') {
                        memSize *= 1024;
                    } else if (*param == 'm' || *param == 'M') {
                        memSize *= 1024 * 1024;
                    } else if (*param != '\0') {
                        printf("bad parameter for -M\n");
                        memSize = DEFAULT_MEMSIZE;
                    }
                }
                break;

            case 'f':
                param = getParam(i, argc, argv, "-f");
                if (param == NULL) {
                    HMD_PRINTF("symbol file is not specified\n");
                    return 1;
                }
                symbolFile = param;
                break;
                    
            case 's':
                bPrintSource = true;
                break;

            case 'h':
                usage(true);
                return 1;

            default:
                usage(true);
                HMD_PRINTF("unknown option %s\n", argv[i]);
                return 1;
            }
        } else {
            usage(true);
            HMD_PRINTF("unknown option %s\n", argv[i]);
            return 1;
        }
    }

    hayatMemory = HMD_ALLOC(memSize);
    MemPool::initGMemPool(hayatMemory, memSize);


    for (int i = 0; i < numIncPath; ++i) {
        addLoadPath(incPath[i], "out");
    }
    addLoadPath(".", "out");

    if (symbolFile != NULL) {
        if (! gSymbolTable.readFile(symbolFile)) {
            HMD_PRINTF("symbol file '%s' load failed\n", symbolFile);
            HMD_FREE(hayatMemory);
            return 1;
        }
    } else {
        hyu8* fileBuf;
        hyu32 len = hmd_loadFile_inPath("symbols.sym", &fileBuf);
        if (len != 0)
            gSymbolTable.readTable(fileBuf);
        else
            HMD_PRINTF("could not load symbols.sym\n");
    }

    
    // main loop

    static const size_t BUF_SIZE = 1024;
    char buf[BUF_SIZE];
    while (fgets(buf, BUF_SIZE, stdin) != NULL) {
        TArray<char>* s = replaceSymbol(buf);
        printResolvedLine(s->top());
        s->finalize();
    }



    // normal exit
    gSymbolTable.finalize();
    HMD_FREE(hayatMemory);
    return 0;
}
