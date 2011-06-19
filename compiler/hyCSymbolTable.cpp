/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCSymbolTable.h"
#include "hyEndian.h"
#include "hyMemPool.h"
#include "hyCFileOut.h"
#include <stdio.h>

using namespace Hayat::Common;
using namespace Hayat::Compiler;


#ifndef WIN32
const char* const SymbolTable::SYMBOL_FILENAME = "/symbols.sym";
const char* const SymbolTable::SYMBOL_H_NAME = "/HSymbol.h";
#else
const char* const SymbolTable::SYMBOL_FILENAME = "\\symbols.sym";
const char* const SymbolTable::SYMBOL_H_NAME = "\\HSymbol.h";
#endif


SymbolTable::SymbolTable(void)
    : m_numSymbols(0), m_symOffsSize(0), m_symbolBufSize(0)
{
}

SymbolTable::~SymbolTable()
{
    finalize();
}

void SymbolTable::initialize(const char* const* initSymTable, hyu32 initSymOffsSize, hyu32 initSymbolBufSize)
{
    m_numSymbols = 0;
    m_symOffs = gMemPool->allocT<int>(initSymOffsSize, "symo");
    HMD_ASSERT(m_symOffs != NULL);
    m_symOffsSize = initSymOffsSize;
    m_symOffs[0] = 0;
    m_symbolBuf = gMemPool->allocT<char>(initSymbolBufSize, "symb");
    HMD_ASSERT(m_symbolBuf != NULL);
    m_symbolBufSize = initSymbolBufSize;
    if (initSymTable != NULL) {
        while (*initSymTable != NULL)
            symbolID(*initSymTable++);
    }
}

void SymbolTable::finalize(void)
{
    if (m_symbolBufSize > 0) {
        gMemPool->free(m_symbolBuf);
        m_symbolBufSize = 0;
    }
    if (m_symOffsSize > 0) {
        gMemPool->free(m_symOffs);
        m_symOffsSize = 0;
    }
}

// strに対応するSymbol IDを返す。無ければ SYMBOL_ID_ERROR を返す
SymbolID_t  SymbolTable::check(const char* str, hyu32 len)
{
    for (hyu32 i = 0; i < m_numSymbols; i++) {
        const char* p = m_symbolBuf + m_symOffs[i];
        if (p[len] == '\0')
            if (!strncmp(str, p, len))
                return (SymbolID_t)i;
    }
    return SYMBOL_ID_ERROR;
}

SymbolID_t SymbolTable::symbolID(const char* strStart, const char* strEnd)
{
    if (m_symbolBufSize == 0) {
        m_symOffsSize = 8;
        m_symOffs = gMemPool->allocT<int>(m_symOffsSize, "symo");
        HMD_ASSERT(m_symOffs != NULL);
        m_symOffs[0] = 0;
        m_symbolBufSize = 64;
        m_symbolBuf = gMemPool->allocT<char>(m_symbolBufSize, "symb");
        HMD_ASSERT(m_symbolBuf != NULL);
    }
    size_t len = strEnd - strStart;
    HMD_ASSERT(len < 256);
    SymbolID_t symID = check(strStart, len);
    if (symID != SYMBOL_ID_ERROR)
        return symID;
    if (m_symOffs[m_numSymbols] + len + 1 >= m_symbolBufSize) {
        HMD__PRINTF_MEM("SymbolTable: realloc m_symbolBuf(%x) size(%x)\n", m_symbolBuf, m_symbolBufSize*sizeof(char));
        if (m_symbolBufSize == 0) m_symbolBufSize = m_INIT_SYMBOLBUF_SIZE;
        while (m_symOffs[m_numSymbols] + len + 1 >= m_symbolBufSize)
            m_symbolBufSize *= 2;
        m_symbolBuf = gMemPool->reallocT<char>(m_symbolBuf, m_symbolBufSize);
        HMD__PRINTF_MEM("             -->     m_symbolBuf(%x) size(%x)\n", m_symbolBuf, m_symbolBufSize*sizeof(char));
        HMD_ASSERT(m_symbolBuf != NULL);
    }
    char* p = m_symbolBuf + m_symOffs[m_numSymbols];
    HMD_STRNCPY(p, strStart, len+1);
    if (m_numSymbols+1 >= m_symOffsSize) {
        HMD__PRINTF_MEM("SymbolTable: realloc m_symOffs(%x) size(%x)\n", m_symOffs, m_symOffsSize*sizeof(int));
        if (m_symOffsSize == 0) m_symOffsSize = m_INIT_SYMOFFS_SIZE;
        while (m_numSymbols+1 >= m_symOffsSize)
            m_symOffsSize *= 2;
        m_symOffs = gMemPool->reallocT<int>(m_symOffs, m_symOffsSize);
        HMD__PRINTF_MEM("             -->     m_symOffs(%x) size(%x)\n", m_symOffs, m_symOffsSize*sizeof(int));
        HMD_ASSERT(m_symOffs != NULL);
    }
    m_symOffs[m_numSymbols+1] = m_symOffs[m_numSymbols] + len + 1;
    return (SymbolID_t) m_numSymbols++;
}

// strに対応するSymbol IDを返す。既存に無ければ新規割当て
SymbolID_t SymbolTable::symbolID(const char* str)
{
    return symbolID(str, str + HMD_STRLEN(str));
}

// Substrに対応するSymbol IDを返す。既存に無ければ新規割当て
SymbolID_t SymbolTable::symbolID(InputBuffer* inp, Substr& ss)
{
    return symbolID(inp->addr(ss.startPos), inp->addr(ss.endPos));
}

const char* SymbolTable::id2str(SymbolID_t id)
{
    if ((hyu32)id >= m_numSymbols)
        return NULL;
    return m_symbolBuf + m_symOffs[(hyu32)id];
}


// ファイルからテーブルを読み込む
bool SymbolTable::readFile(const char* path)
{
    FILE* fp = hmd_fopen(path, "rb");
    if (fp == NULL) {
        initialize(NULL);
        return false;
    }
    hyu8 buf[4];
    size_t nread = fread(buf, sizeof(hyu8), 4, fp);
    HMD_ASSERTMSG(nread == 4, M_M("cannot read num symbols from file %s"), path);
    m_numSymbols = Endian::unpack<hyu32>(buf);
    nread = fread(buf, sizeof(hyu8), 4, fp);
    HMD_ASSERTMSG(nread == 4, M_M("cannot read symbol table size from file %s"), path);
    m_symbolBufSize = Endian::unpack<hyu32>(buf) - 8;
    m_symOffsSize = m_numSymbols + 1;
    m_symOffs = gMemPool->allocT<int>(m_symOffsSize, "symo");
    m_symbolBuf = gMemPool->allocT<char>(m_symbolBufSize, "symb");
    nread = fread(m_symbolBuf, sizeof(char), m_symbolBufSize, fp);
    HMD_ASSERT(nread == m_symbolBufSize);
    char* p = m_symbolBuf;
    for (hyu32 i = 0; i < m_numSymbols; i++) {
        m_symOffs[i] = p - m_symbolBuf;
        while (*p++ != '\0')
            ;
    }
    m_symOffs[m_numSymbols] = p - m_symbolBuf;
    HMD_DEBUG_ASSERT(p == m_symbolBuf + m_symbolBufSize);
    fclose(fp);
    return true;
}

bool SymbolTable::mergeFile(const char* path)
{
    hyu8* buf;
    hyu32 nread = hmd_loadFileAll(path, &buf);
    if (nread == 0)
        return true;
    const hyu8* endp = mergeTable(buf, nread);
    hmd_freeFileBuf(buf);
    return (endp != NULL);
}

static bool compTable(const hyu8* smallbuf, const hyu8* bigbuf, hyu32 size)
{
    for (hyu32 i = 0; i < size; ++i) {
        if (*smallbuf++ != *bigbuf++) {
#ifndef TEST__CPPUNIT
            --smallbuf; --bigbuf;
            hyu32 j;
            for (j = 0; j < i; ++j) {
                if (*--smallbuf == '\0') {
                    ++smallbuf;
                    break;
                }
                --bigbuf;
            }
            HMD_PRINTF("SymbolTable: data mismatch at %04x\n", i+8);
            HMD_PRINTF("'%s' != '%s'\n", smallbuf, bigbuf);
#endif
            return false;
        }
    }
    return true;
}

const hyu8* SymbolTable::mergeTable(const hyu8* buf, hyu32 bufSize)
{
    hyu32 numSym = Endian::unpack<hyu32>(buf);
    hyu32 tableSize = Endian::unpack<hyu32>(buf + 4);
    if (tableSize > bufSize) {
        return NULL;
    }
    buf += 8;
    tableSize -= 8;
    hyu32 d = m_symOffs[m_numSymbols];
    if (d >= tableSize) {
        if (! compTable(buf, (const hyu8*)m_symbolBuf, tableSize)) {
            return NULL;
        }
        return buf + tableSize;
    } else {
        HMD_DEBUG_ASSERT(m_numSymbols < numSym);
        if (! compTable((const hyu8*)m_symbolBuf, buf, d)) {
            HMD_PRINTF("SymbolTable: symbol mismatch\n");
            return NULL;
        }
        m_symbolBuf = gMemPool->reallocT<char>(m_symbolBuf, tableSize);
        m_symbolBufSize = tableSize;
        memcpy(m_symbolBuf + d, buf + d, tableSize - d);
        m_symOffs = gMemPool->reallocT<int>(m_symOffs, numSym + 1);
        const char* p = m_symbolBuf + d;
        for (hyu32 i = m_numSymbols; i < numSym; ++i) {
            m_symOffs[i] = p - m_symbolBuf;
            while (*p++ != '\0')
                ;
        }
        m_symOffs[numSym] = p - m_symbolBuf;
        HMD_DEBUG_ASSERT(p == m_symbolBuf + tableSize);
        m_numSymbols = numSym;
        return buf + tableSize;
    }
}


// テーブルをファイルに出力
void SymbolTable::writeFile(const char* path)
{
    DiffWrite dw;
    FILE* fp = dw.open(path);
    HMD_ASSERTMSG(fp != NULL, M_M("cannot write to file %s"), path);
    fwriteTable(fp);
    dw.close();
}

// テーブルをFILE*へ出力
void SymbolTable::fwriteTable(FILE* fp)
{
    hyu8 buf[4];
    Endian::pack<hyu32>(buf, m_numSymbols);
    fwrite(buf, sizeof(hyu8), 4, fp);
    Endian::pack<hyu32>(buf, m_symOffs[m_numSymbols] + 8);
    fwrite(buf, sizeof(hyu8), 4, fp);
    fwrite(m_symbolBuf, sizeof(char), m_symOffs[m_numSymbols], fp);
}


// シンボルラベルヘッダファイルを作成
void SymbolTable::writeSymbolH(const char* path, bool verbose)
{
    DiffWrite dw;
    FILE* fp = dw.open(path);
    if (fp == NULL) {
        if (verbose)
            printf(M_M("can't write to file %s"), path);
        return;
    }
    writeGenHead(fp);
    fprintf(fp, "#ifndef m_HSYMBOL_H_\n");
    fprintf(fp, "#define m_HSYMBOL_H_\n");
    fprintf(fp, "#include \"hySymbolID.h\"\n");
    fprintf(fp, "using namespace Hayat::Common;\n");
    fprintf(fp, "namespace Hayat {\n");
    fprintf(fp, "  namespace Engine {\n");
    for (hyu32 i = 0; i < m_numSymbols; i++) {
        char tmp[256];
        m_symLabel(tmp, 256, m_symbolBuf + m_symOffs[i]);
        HMD_ASSERT(tmp[0] != '\0');
        fprintf(fp, "    static const SymbolID_t %s\t\t= %d;\n", tmp, i);
    }
    fprintf(fp, "  }\n");
    fprintf(fp, "}\n");
    fprintf(fp, "#endif\n");
    dw.close();
}

// 0～15の数値を16進1桁の文字にする
static inline char m_hex(int c)
{
    HMD_DEBUG_ASSERT(c >= 0 && c <= 15);
    if (c > 9)
        return c - 10 + 'a';
    else
        return c + '0';
}
// シンボルラベル生成
void SymbolTable::m_symLabel(char* buf, size_t bufSize, const char* str)
{
    size_t len = HMD_STRLEN(str);
    const char* p = str;
    char c;
    bool special = false;       // 先頭が *
    bool lastQ = false;         // 末尾が ?
    bool lastE = false;         // 末尾が !
    bool lastHYB = false;       // 末尾が .hyb
    bool marks = false;         // [A-Za-z0-9_] 以外が含まれる
    if (*p == '*') {
        special = true;
        ++p;
    }
    while ((c = *p++) != '\0') {
        if (c == '?' && *p == '\0') {
            lastQ = true;
            break;
        } else if (c == '!' && *p == '\0') {
            lastE = true;
            break;
        } else if (c == '.') {
            if (HMD_STRNCMP(p, "hyb", 4) == 0) {
                lastHYB = true;
            } else {
                marks = true;
            }
            break;
        } else if (!(c >= 'a' && c <= 'z') &&
                   !(c >= 'A' && c <= 'Z') &&
                   !(c >= '0' && c <= '9') &&
                   (c != '_')) {
            marks = true;
            break;
        }
    }

    p = str;
    if (marks) {
        if (bufSize < len * 2 + 7) {
            // バッファ不足
            *buf = '\0';
            return;
        }
        HMD_STRNCPY(buf, "HSymx_", 7);
        buf += 6;
        while ((c = *p++) != '\0') {
            *buf++ = m_hex((c >> 4) & 0x0f);
            *buf++ = m_hex(c & 0x0f);
        }
        *buf = '\0';

    } else {
        if (bufSize < len + 8) {
            // バッファ不足
            *buf = '\0';
            return;
        }
        const char* ed = str + len;
        HMD_STRNCPY(buf, "HSym", 5);
        buf += 4;
        if (special) {
            ++p;
            *buf++ = 'R';
        }
        if (lastQ) {
            *buf++ = 'Q';
            *buf++ = '_';
            --ed;
        } else if (lastE) {
            *buf++ = 'E';
            *buf++ = '_';
            --ed;
        } else if (lastHYB) {
            *buf++ = 'b';
            *buf++ = '_';
            ed -= 4;
        } else {
            *buf++ = '_';
        }        
        while (p < ed) {
            *buf++ = *p++;
        }
        *buf = '\0';
    }
}
