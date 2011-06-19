/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyEndian.h"
#include "hySymbolTable.h"
#include "hyBMap.h"
#include "hyStack.h"
#include "hpSourceInfo.h"
#include "hyBytecodeIf.h"
#include "hyDebugSourceInfo.h"
#include "hyCCommandLine.h"
#include "hySignatureBase.h"
#include "opcode_def.h"
#include "htPrintOpcode.h"


#define DEBUGINFO_EXT   ".hdb"
#define DEFAULT_MEMSIZE (16 * 1024 * 1024)


const char** HMD_LOADPATH = NULL;
hyu32   numLoadPath = 0;
bool    bPrintPredefinedSignatures = false;


using namespace Hayat::Common;
using namespace Hayat::Engine;
using namespace Hayat::Compiler;
using namespace Hayat::Tool;




static void printSymbols(SymbolID_t symArr[], int numSym, const char* prefix)
{
    int i = 0;
    hyu8 sbuf[256];
    while (i < numSym) {
        SymbolID_t sym = symArr[i++];
        gSymbolTable.id2str_n(sbuf, 256, sym);
        HMD_PRINTF(prefix);
        HMD_PRINTF((char*)sbuf);
        HMD_PRINTF((i < numSym) ? ", " : "\n");
    }
}



class DBytecode;
class DClass {

public:
    DClass(hyu8* code, const hyu8* bottom, DBytecode* pBytecode);
    ~DClass();
    static void* operator new(size_t size);
    static void operator delete(void* p);


    bool        validate(void);
    void        dump(TArray<SymbolID_t>& parents);
    void        dumpMethod(const hyu8* start, const hyu8* end);

    DBytecode*  bytecode;
    SymbolID_t  classSymbol;
    hyu16       flags;
    bool        bPrimitive;
    hyu16       numMethods;
    hyu16       numInnerClasses;
    TArray<DClass*>     innerClasses;
    hyu16       numSuper;
    hyu16       numClassVar;
    hyu16       numMembVar;
    hyu16       numConstVar;
    hyu16       numDefaultVal;
    hyu16       numClosures;
    SymbolID_t* methodSymTable;
    SymbolID_t* classSymTable;
    TArray<SymbolID_t>  classVarSymArr;
    TArray<SymbolID_t>  membVarSymArr;
    TArray<SymbolID_t>  constVarSymArr;
    hyu32*      methodTable;
    hyu32*      innerClassTable;
    hyu32*      superTable;
    hyu16       numUsingPaths;
    SymbolID_t* usingPaths;
    const hyu8* topAddr;
    const hyu8* bottomAddr;
    const hyu8* codes;

    class JumpControlTbl {
    public:
        TArray< BMap<hys32, SymbolID_t>* > tbl;

        JumpControlTbl(const hyu8** pCode)
            : tbl(0)
        {
            const hyu8* code = *pCode;
            hyu16 endOffs = Endian::unpack<hyu16>(code);
            const hyu8* endAddr = code + endOffs;
            code += 2;
            hyu8 numTbl = *code;
            code += 2;
            hyu16* tblOffs = (hyu16*) code;
            code += sizeof(hyu16) * numTbl;
            alignPtr<4>(&code);

            tbl.initialize(numTbl);
            for (hyu8 i = 0; i < numTbl; ++i) {
                HMD_ASSERTMSG(tblOffs[i] == code - *pCode, "offs=%x, code=%x, start=%x",tblOffs[i],code,*pCode);
                hyu8 numLabel = *code;
                code += 4;
                BMap<hys32, SymbolID_t>* addrLabel = new BMap<hys32, SymbolID_t>(numLabel);
                SymbolID_t* labels = (SymbolID_t*) code;
                hys32* addrs = (hys32*)(code + sizeof(SymbolID_t) * numLabel);
                alignPtr<4>(&addrs);
                for (hyu8 j = 0; j < numLabel; ++j) {
                    addrLabel->forceAdd(addrs[j], labels[j]);
                }
                tbl.add(addrLabel);
                code = (const hyu8*) (addrs + numLabel);
            }
            HMD_ASSERT(code == endAddr);
            *pCode = code;
        }
        ~JumpControlTbl() {
            for (int i = tbl.size() - 1; i >= 0; --i) {
                delete tbl[i];
            }
            tbl.finalize();
        }

        hyu32   size(void) { return tbl.size(); }
        BMap<hys32, SymbolID_t>* get(hyu8 idx) { return tbl[idx]; }
    };

};

class DBytecode : public BytecodeIf {
public:
    DBytecode(hyu8* bytecode, hyu32 bytecodeSize);
    virtual ~DBytecode();
    static void* operator new(size_t size);
    static void operator delete(void* p);

    bool        validate(void);
    void        dump(void);

    void readDebugInfo(const char* path) { debugInfos.readDebugInfo(path); }
    void debugGetInfo(char* buf, hyu32 bufSize, const hyu8* ptr) const;
    const hyu8* codeStartAddr(void) const { return topAddr; }
    const hyu8* getString(hyu32 offs) const;
    const SignatureBase::Sig_t* getSignatureBytes(hyu16 id) const;

    hyu8*       topAddr;
    hyu8*       bottomAddr;
    SymbolID_t  mySymbol;
    hyu32       stringTableSize;
    const hyu8* stringTable;
    hyu32       signatureTableNum;
    const hyu32* signatureTableOffs;
    const hyu8* signatureTable;
    const hyu8* signatureArityTable;
    hyu16       numLinkBytecodes;
    const SymbolID_t*   linkBytecodes;
    const hyu8* linkBytecodeName;
    DClass*     mainClass;
    DebugInfos  debugInfos;

};


DClass::DClass(hyu8* code, const hyu8* bottom, DBytecode* pBytecode)
    : innerClasses(0),
      classVarSymArr(0), membVarSymArr(0), constVarSymArr(0)
{
    bytecode = pBytecode;
    topAddr = code;
    bottomAddr = bottom;
    flags = Endian::unpack<hyu16>(code);
    bPrimitive = ((flags & 1) != 0);
    numMethods = Endian::unpack<hyu16>(code + 2);
    numInnerClasses = Endian::unpack<hyu16>(code + 4);
    numSuper = Endian::unpack<hyu16>(code + 6);
    numClassVar = Endian::unpack<hyu16>(code + 8);
    numMembVar = Endian::unpack<hyu16>(code + 10);
    numConstVar = Endian::unpack<hyu16>(code + 12);
    numDefaultVal = Endian::unpack<hyu16>(code + 14);
    numClosures = Endian::unpack<hyu16>(code + 16);
    int numBlk = numMethods + numClosures;

    methodSymTable = (SymbolID_t*)(code + 20);
    int i;
    for (i = 0; i < numMethods; i++)
        Endian::rewrite<SymbolID_t>(&methodSymTable[i]); // endianèëä∑Ç¶

    classSymTable = methodSymTable + numMethods;
    for (i = 0; i < numInnerClasses; i++)
        Endian::rewrite<SymbolID_t>(&classSymTable[i]);

    SymbolID_t* p = classSymTable + numInnerClasses;
    for (i = 0; i < numClassVar; ++i)
        Endian::rewrite<SymbolID_t>(&p[i]);
    classVarSymArr.initialize(p, numClassVar);

    p += numClassVar;
    for (i = 0; i < numMembVar; ++i)
        Endian::rewrite<SymbolID_t>(&p[i]);
    membVarSymArr.initialize(p, numMembVar);

    p += numMembVar;
    for (i = 0; i < numConstVar; ++i)
        Endian::rewrite<SymbolID_t>(&p[i]);
    constVarSymArr.initialize(p, numConstVar);

    p += numConstVar;
    methodTable = (hyu32*)alignInt<4>((int)p);
    innerClassTable = (hyu32*)(methodTable + numBlk);
    superTable = (hyu32*)(innerClassTable + numInnerClasses + 1);

    usingPaths = (SymbolID_t*)(superTable + numSuper);
    hyu16 usingInfoSize = Endian::unpack<hyu16>((const hyu8*)usingPaths);

    hyu8* pc = ((hyu8*)usingPaths) + usingInfoSize;
    codes = pc;

    innerClasses.initialize(numInnerClasses);
    hyu8* ip = pc + Endian::unpack<hyu32>((const hyu8*)&innerClassTable[0]);
    for (i = 0; i < numInnerClasses; ++i) {
        hyu8* btm = pc + Endian::unpack<hyu32>((const hyu8*)&innerClassTable[i+1]);
        DClass* dc;
        if (btm > bottomAddr)
            dc = NULL;
        else {
            dc = new DClass(ip, btm, pBytecode);
            dc->classSymbol = classSymTable[i];
        }
        innerClasses.subst(i, dc, NULL);
        ip = btm;
    }
}

DClass::~DClass()
{
    for (int i = numInnerClasses - 1; i >= 0; --i) {
        delete innerClasses[i];
    }
}

void* DClass::operator new(size_t size)
{
    return (DClass*) gMemPool->alloc(size, "CLAS");
}
    
void DClass::operator delete(void* p)
{
    gMemPool->free(p);
}

bool DClass::validate(void)
{
    for (int i = 0; i < numInnerClasses; ++i) {
        if (innerClasses[i] == NULL)
            return false;
        if (! innerClasses[i]->validate())
            return false;
    }
    return true;
}

void DClass::dump(TArray<SymbolID_t>& parents)
{
    hyu8 sbuf[256];

    if (bPrimitive)
        HMD_PRINTF("flag: primitive\n");
    if (numDefaultVal > 0)
        HMD_PRINTF("number of default values: %d\n", numDefaultVal);
    if (numSuper > 0) {
        HMD_PRINTF("%d super classes: ", numSuper);
        for (int i = 0; i < numSuper; ++i) {
            SymbolID_t supsym = (SymbolID_t)Endian::unpack<hyu32>((const hyu8*)&superTable[i]);
            gSymbolTable.id2str_n(sbuf, 256, supsym);
            HMD_PRINTF((char*)sbuf);
            if (i < numSuper - 1)
                HMD_PRINTF(", ");
        }
        HMD_PRINTF("\n");
    }

    bool bPrintUsingPath = false;
    hyu16 usingInfoSize = Endian::unpack<hyu16>((const hyu8*)usingPaths);
    const hyu8* ptr = ((const hyu8*)usingPaths) + 4;
    for (;;) {
        SymbolID_t sym = Endian::unpackP<SymbolID_t>(&ptr);
        if (sym == SYMBOL_ID_ERROR)
            break;
        if (!bPrintUsingPath) {
            bPrintUsingPath = true;
            HMD_PRINTF("using paths:\n");
        }
        if (sym != (SymbolID_t)0) { // 0 == HSym_nil
            gSymbolTable.id2str_n(sbuf, 256, sym);
            HMD_PRINTF((char*)sbuf);
        }
        for (;;) {
            sym = Endian::unpackP<SymbolID_t>(&ptr);
            if (sym == SYMBOL_ID_ERROR)
                break;
            gSymbolTable.id2str_n(sbuf, 256, sym);
            HMD_PRINTF("::%s", sbuf);
        }
    }
    HMD_DEBUG_ASSERT(ptr <= ((const hyu8*)usingPaths) + usingInfoSize);

    if (numClassVar > 0) {
        HMD_PRINTF("class variables: ");
        printSymbols(classVarSymArr.top(), numClassVar, "@@");
    }
    if (numMembVar > 0) {
        HMD_PRINTF("member variables: ");
        printSymbols(membVarSymArr.top(), numMembVar, "@");
    }
    if (numConstVar > 0) {
        HMD_PRINTF("constants: ");
        printSymbols(constVarSymArr.top(), numConstVar, "");
    }
        
    int numBlk = numMethods + numClosures;
    TArray<const hyu8*> adrs(numBlk + 2);
    adrs.add(codes);
    for (int i = 0; i < numBlk; ++i)
        adrs.add(codes + methodTable[i]);
    if (numInnerClasses > 0)
        adrs.add(codes + innerClassTable[0]);
    else
        adrs.add(bottomAddr);
    
    for (int i = 0; i <= numBlk; ++i) {
        if (i == 0)
            HMD_PRINTF("***** classInitializer *MAIN *****\n");
        else if (i <= numMethods) {
            gSymbolTable.id2str_n(sbuf, 256, methodSymTable[i-1]);
            HMD_PRINTF("***** method %s *****\n", sbuf);
        }
        else
            HMD_PRINTF("***** closure #%d *****\n", i - numMethods - 1);
        dumpMethod(adrs[i], adrs[i+1]);
    }


    for (int i = 0; i < numInnerClasses; ++i) {
        TArray<SymbolID_t> abs(parents.size() + 1);
        abs.add(parents);
        abs.add(innerClasses[i]->classSymbol);

        SymbolID_t sym;
        HMD_PRINTF("======== class ");
        for (hyu32 j = 0; j < abs.size(); ++j) {
            sym = abs[j];
            gSymbolTable.id2str_n(sbuf, 256, sym);
            HMD_PRINTF("::%s", sbuf);
        }
        HMD_PRINTF(" ========\n");
        innerClasses[i]->dump(abs);
    }
}

void DClass::dumpMethod(const hyu8* start, const hyu8* end)
{
    hyu8 sbuf[256];
    const hyu8* code = start;
    JumpControlTbl jct(&code);
    hyu16 sigID = Endian::unpackP<hyu16>(&code);
    hyu16 defValOffs = Endian::unpackP<hyu16>(&code);
    hyu8 numLocal = Endian::unpackP<hyu8>(&code);
    code += 3;                  // padding

    Stack<hyu8> jcNumStack;
    Stack<hyu8> jcIdxStack;
    Stack<const hyu8*> jcStartPosStack;
    hyu8 jcNum = 0;
    hyu8 jcIdx = 0;
    const hyu8* jcStartPos = code;
    
    HMD_PRINTF("signature : ");
    SignatureBase::debugPrintSignature(bytecode->getSignatureBytes(sigID));
    HMD_PRINTF("\ndefault value offset : %d\n", defValOffs);
    HMD_PRINTF("num local variables : %d\n", numLocal);

    while (code < end) {
        if (jcNum < jct.size()) {
            BMap<hys32, SymbolID_t>* addrLabels = jct.get(jcNum);
            while (jcIdx < addrLabels->size()) {
                if (jcStartPos + addrLabels->keys()[jcIdx] != code)
                    break;
                SymbolID_t label = addrLabels->values()[jcIdx];
                gSymbolTable.id2str_n(sbuf, 256, label);
                HMD_PRINTF("label :%s\n", sbuf);
                ++jcIdx;
            }
        }
        
        HMD_PRINTF("%6x  ", code - bytecode->topAddr);
        const hyu8* ocode = code;

        code = printOpcode(code, bytecode);
        
        if (*ocode == OPCODE_JUMPCONTROLSTART) {
            jcNumStack.push(jcNum);
            jcIdxStack.push(jcIdx);
            jcStartPosStack.push(jcStartPos);
            jcStartPos = code;
            jcNum = ocode[1];
            jcIdx = 0;
        } else if (*ocode == OPCODE_JUMPCONTROLEND) {
            jcNum = jcNumStack.pop();
            jcIdx = jcIdxStack.pop();
            jcStartPos = jcStartPosStack.pop();
        }
    }

}


DBytecode::DBytecode(hyu8* bytecode, hyu32 bytecodeSize)
{
    topAddr = bytecode;
    bottomAddr = bytecode + bytecodeSize;

    numLinkBytecodes = Endian::unpack<hyu16>(bytecode);
    mySymbol = Endian::unpack<SymbolID_t>(bytecode + 4);
    linkBytecodes = (const SymbolID_t*)(bytecode + 4 + sizeof(SymbolID_t));
    for (int i = 0; i < numLinkBytecodes; i++) {
        // endianèëä∑Ç¶
        Endian::rewrite<SymbolID_t>(const_cast<SymbolID_t*>(&linkBytecodes[i]));
    }
    const hyu8* p = (const hyu8*)(linkBytecodes + numLinkBytecodes);
    alignPtr<4>(&p);
    hyu32 linkBytecodeNameTableSize = Endian::unpackP<hyu32>(&p);
    linkBytecodeName = p;
    p += alignInt<4>(linkBytecodeNameTableSize);

    stringTableSize = Endian::unpackP<hys32>(&p);
    stringTable = p;
    p += alignInt<4>(stringTableSize);

    signatureTableNum = Endian::unpackP<hyu32>(&p);
    signatureTableOffs = (const hyu32*)p;
    p += sizeof(hyu32) * signatureTableNum;
    hyu32 stblsize = Endian::unpackP<hyu32>(&p);
    signatureTable = (const hyu8*)p;
    p += alignInt<4>(stblsize);
    signatureArityTable = (const hyu8*)p;
    p += alignInt<4>(signatureTableNum);
    
    mainClass = new DClass(const_cast<hyu8*>(p), bottomAddr, this);
    mainClass->classSymbol = mySymbol;
}

bool DBytecode::validate(void)
{
    if (mainClass == NULL)
        return false;
    return mainClass->validate();
}

DBytecode::~DBytecode()
{
    delete mainClass;
}

void* DBytecode::operator new(size_t size)
{
    return (DBytecode*) gMemPool->alloc(size, "BYTC");
}
    
void DBytecode::operator delete(void* p)
{
    gMemPool->free(p);
}

void DBytecode::dump(void)
{
    hyu32 i;
    hyu8 sbuf[256];

    gSymbolTable.id2str_n(sbuf, 256, mySymbol);
    HMD_PRINTF("$$$$$ package: %s $$$$$\n", sbuf);
    HMD_PRINTF("==== link bytecodes ====\n");
    if (numLinkBytecodes == 0) {
        HMD_PRINTF("--- none ---\n");
    } else {
        const char* p = (const char*)linkBytecodeName;
        for (i = 0; i < numLinkBytecodes; ++i) {
            HMD_PRINTF("require \"%s{%d}\"\n", p, linkBytecodes[i]);
            p += HMD_STRLEN(p) + 1;
        }
    }

    HMD_PRINTF("==== string table ====\n");
    if (stringTableSize == 0) {
        HMD_PRINTF("--- none ---\n");
    } else {
        i = 0;
        while (i < stringTableSize) {
            const char* p = (const char*)(stringTable + i);
            HMD_PRINTF("%4d: '%s'\n", i, p);
            i += HMD_STRLEN(p) + 1;
        }
    }

    HMD_PRINTF("==== signature table ====\n");
    if (signatureTableNum == 0 && !bPrintPredefinedSignatures) {
        HMD_PRINTF("--- none ---\n");
    } else {
        HMD_PRINTF("#:arity:signature\n");
        if (bPrintPredefinedSignatures)
            SignatureBase::printPredefinedSignatures();
        for (i = 0; i < signatureTableNum; ++i) {
            HMD_PRINTF("%d:%d:", i + SignatureBase::NUM_PREDEFINED_PARAMS, signatureArityTable[i]);
            SignatureBase::debugPrintSignature(signatureTable + signatureTableOffs[i]);
            HMD_PRINTF("\n");
        }
    }

    gSymbolTable.id2str_n(sbuf, 256, mainClass->classSymbol);
    HMD_PRINTF("======== package class %s ========\n", sbuf);
    TArray<SymbolID_t> parents(0);
    mainClass->dump(parents);
}

void DBytecode::debugGetInfo(char* buf, hyu32 bufSize, const hyu8* ptr) const
{
    if (! debugInfos.wasInfoRead()) {
        *buf = '\0';
        return;
    }

    hyu32 pos;
    if (topAddr <= ptr && ptr < bottomAddr)
        pos = ptr - topAddr;
    else
        pos = 0;
    debugInfos.debugGetInfo(buf, bufSize, pos);
}

const hyu8* DBytecode::getString(hyu32 offs) const
{
    HMD_ASSERTMSG(offs < stringTableSize, "offs=%d stringTableSize=%d",offs,stringTableSize);
    return stringTable + offs;
}

const SignatureBase::Sig_t* DBytecode::getSignatureBytes(hyu16 id) const
{
    HMD_ASSERTMSG(id < signatureTableNum + SignatureBase::NUM_PREDEFINED_PARAMS, "id=%d signatureTableNum=%d",id,signatureTableNum);
    if (id < SignatureBase::NUM_PREDEFINED_PARAMS)
        return SignatureBase::PREDEFINED_PARAMS[id];
    id -= SignatureBase::NUM_PREDEFINED_PARAMS;
    return (const SignatureBase::Sig_t*) (signatureTable + signatureTableOffs[id]);
}



void usage(bool detail = false)
{
    HMD_PRINTF("hydas :- Hayat disassembler\n");
    HMD_PRINTF("usage: disasm [options...] [-p] filename.hyb\n");
    if (detail) {
        HMD_PRINTF("  options:\n");
        HMD_PRINTF("  -I path : add path for searching debug info files\n");
        HMD_PRINTF("  -M memsize : set memory size   (ex.) -M8M , -M500K\n");
        HMD_PRINTF("  -f symbols.sym : specify symbols.sym file\n");
        HMD_PRINTF("  -p : print predefined signatures\n");
        HMD_PRINTF("  -s : print source position (need *.hdb)\n");
        HMD_PRINTF("  -h : print this\n");
    }
}

int main(int argc, const char* argv[])
{
    hyu32 memSize = DEFAULT_MEMSIZE;
    const char* param;
    const char* bytecodeFile = NULL;
    const char* symbolFile = NULL;
    const char* incPath[32];
    int numIncPath = 0;
    bool bDebugSource;
    void* hayatMemory = NULL;

    numLoadPath = 0;
    HMD_LOADPATH = NULL;


    //============================================================
    // parse command line

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
            case 'I':
                // includeÉpÉX
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
                    
            case 'p':
                bPrintPredefinedSignatures = true;
                break;
                
            case 's':
                bDebugSource = true;
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
            if (bytecodeFile != NULL) {
                HMD_PRINTF("only 1 file can be disassembled\n");
                return 1;
            }
            bytecodeFile = argv[i];
        }
    }

    if (bytecodeFile == NULL) {
        HMD_PRINTF("bytecode file was not specified\n");
        return 1;
    }


    //============================================================
    // initialize

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

    //============================================================
    // main routine

    int exitcode = 0;

    hyu8* fileBuf;
    hyu32 len = hmd_loadFileAll(bytecodeFile, &fileBuf);
    if (len == 0) {
        HMD_PRINTF("could not load %s\n", bytecodeFile);
        exitcode = 1;
        goto exit_hydas;
    }

    {
        DBytecode db(fileBuf, len);
        if (! db.validate()) {
            HMD_PRINTF("file maybe broken or not bytecode\n");
            exitcode = 1;
            goto exit_hydas;
        }

        if (bDebugSource) {
            hyu32 dl = HMD_STRLEN(bytecodeFile);
            char* debfile = (char*)HMD_ALLOCA(dl + 8);
            HMD_STRNCPY(debfile, bytecodeFile, dl - 4);
            HMD_STRNCPY(debfile + dl - 4, DEBUGINFO_EXT, 5);

            db.readDebugInfo(debfile);
        }

        db.dump();
    }

    //============================================================
    // normal exit
 exit_hydas:
    gSymbolTable.finalize();
    HMD_FREE(hayatMemory);
    return exitcode;
}
