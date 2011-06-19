/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


#include "htPrintOpcode.h"


#include <stdarg.h>
#include "opcode_def.h"
#include "hyBytecodeIf.h"
#include "hyEndian.h"
#include "hySymbolTable.h"


using namespace Hayat::Common;
using namespace Hayat::Engine;
using namespace Hayat::Tool;


extern struct m_op_d_st { const char* name; const char* type; } m_HAYAT_OPCODE_DEBUG_TABLE_[];


static void addPrintf(char* buf, size_t bufSize, const char* format, ...)
{
    static const size_t SIZE_OPR = 64;
    char opr[SIZE_OPR];
    va_list ap;
    va_start(ap, format);
    HMD_VSNPRINTF(opr, SIZE_OPR, format, ap);
    va_end(ap);
    HMD_STRSCAT(buf, opr, bufSize);
}

static void addDump(char* dump, size_t dumpSize, const hyu8** addr, size_t num)
{
    char xx[16];
    while (num-- > 0) {
        HMD_SNPRINTF(xx,16, "%02x", *((*addr)++));
        HMD_STRSCAT(dump, xx, dumpSize);
    }
}

static void addPrintSymbol(char* buf, size_t bufSize, SymbolID_t sym)
{
    hyu32 len = HMD_STRLEN(buf);
    gSymbolTable.id2str_n((hyu8*)(buf+len), bufSize-len, sym);
}

const hyu8* Hayat::Tool::printOpcode(const hyu8* addr, const BytecodeIf* bif)
{
    static const size_t SIZE_BUF = 256;
    static const size_t SIZE_DUMP = 64;
    char buf[SIZE_BUF];
    char dump[SIZE_DUMP];
    const char* tp;

    bif->debugGetInfo(buf, SIZE_BUF, addr);
    hyu8 opcode = *addr++;
    if (*buf != '\0')
        HMD_PRINTF("%-30s %02x", buf, opcode);
    else
        HMD_PRINTF("%02x", opcode);
    if (opcode < NUM_OPCODES) {
        HMD_SNPRINTF(buf, SIZE_BUF, "%s", m_HAYAT_OPCODE_DEBUG_TABLE_[opcode].name);
        tp = m_HAYAT_OPCODE_DEBUG_TABLE_[opcode].type;
    } else {
        HMD_STRNCPY(buf, "UNKNOWN_OPCODE", SIZE_BUF);
        tp = "";
    }


#define DUMP_OPR(format, type)                  \
    addPrintf(buf,SIZE_BUF,format,Endian::unpack<type>(addr));  \
    addDump(dump,SIZE_DUMP, &addr, sizeof(type));

#define DUMP_SYM()                              \
    addPrintSymbol(buf,SIZE_BUF,Endian::unpack<SymbolID_t>(addr));      \
    addDump(dump,SIZE_DUMP, &addr, sizeof(SymbolID_t));


    dump[0] = '\0';

    bool firstOperand = true;
    while (*tp) {
        if (firstOperand) {
            firstOperand = false;
            HMD_STRSCAT(buf, " ", SIZE_BUF);
        } else {
            HMD_STRSCAT(buf, ",", SIZE_BUF);
        }
        switch (*tp++) {
        case 'I':           // INT
            DUMP_OPR("%d", hys32);
            break;
        case 'i':           // INT8
            DUMP_OPR("%d", hys8);
            break;
        case 'F':           // FLOAT
            DUMP_OPR("%f", hyf32);
            break;
        case '1':           // UINT8
            DUMP_OPR("%d", hyu8);
            break;
        case '2':           // UNIT16
            DUMP_OPR("%d", hyu16);
            break;
        case '4':           // UNIT32
            DUMP_OPR("%d", hyu32);
            break;
        case 'S':           // STRCONST
            addPrintf(buf,SIZE_BUF,"\"%s\"",bif->getString(Endian::unpack<hys32>(addr)));
            addDump(dump,SIZE_DUMP, &addr, sizeof(hys32));
            break;
        case 'l':           // LOCALVAR
            DUMP_OPR("%d", hys16);
            break;
        case 'm':           // MAYLOCALVAR
            {
                hyu16 v = Endian::unpack<hyu16>(addr);
                addDump(dump,SIZE_DUMP, &addr, sizeof(hyu16));
                if (v != 0x8000) {
                    addPrintf(buf,SIZE_BUF,"%d",v);
                } else {
                    HMD_STRSCAT(buf,"nil",SIZE_BUF);
                }
            }
            break;
        case 'T':           // TYPE
            addPrintf(buf,SIZE_BUF,"%s",gSymbolTable.id2str(Endian::unpack<hys32>(addr)));
            addDump(dump,SIZE_DUMP, &addr, sizeof(hys32));
            break;
        case 'C':           // CLASS
        case 'M':           // METHOD
        case 'f':           // MEMBVAR
        case 'c':           // CLASSVAR
        case 'g':           // GLOBALVAR
        case 'A':           // CONST
        case 'Y':           // SYMBOL
            DUMP_SYM();
            break;
        case 'R':           // RELATIVE
            {
                hys32 rel = Endian::unpack<hys32>(addr);
                addDump(dump,SIZE_DUMP, &addr, sizeof(hys32));
                int jumpPos = addr + rel - bif->codeStartAddr();
                if (rel >= 0)
                    addPrintf(buf,SIZE_BUF,"%x(+%x)", jumpPos, rel);
                else
                    addPrintf(buf,SIZE_BUF,"%x(-%x)", jumpPos, -rel);
            }
            break;
        case 'y':           // JUMPSYMBOL
            {
                hys32 sym = Endian::unpack<hys32>(addr);
                addDump(dump,SIZE_DUMP, &addr, sizeof(hys32));
                addPrintSymbol(buf,SIZE_BUF,sym);
            }
            break;
        case 's':           // SCOPE
            {
                int n = *addr;
                addPrintf(buf,SIZE_BUF,"%d, ", n);
                addDump(dump,SIZE_DUMP, &addr, sizeof(hyu8));
                for (int i = 0; i < n; i++) {
                    DUMP_SYM();
                    if (i < n-1) HMD_STRSCAT(buf,"::",SIZE_BUF);
                }
            }
            break;
        case 'p':       // SIGNATURE
            {
                hyu16 sigID = Endian::unpack<hyu16>(addr);
                addPrintf(buf,SIZE_BUF,"sig(%d):", sigID);
                addDump(dump,SIZE_DUMP, &addr, sizeof(hyu16));
                const SignatureBase::Sig_t* sigp = bif->getSignatureBytes(sigID);
                SignatureBase::snAddPrintSig(buf,SIZE_BUF,sigp);
            }
            break;
        default:
            HMD_FATAL_ERROR("internal error: bad debug info %c\n", tp[-1]);
        }
    }
    HMD_PRINTF("%-12s %s\n", dump, buf);
    fflush(stdout);

    return addr;
}
