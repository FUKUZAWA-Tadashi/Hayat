/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCIdent.h"
#include "hyMemPool.h"

using namespace Hayat::Common;
using namespace Hayat::Compiler;




bool Ident::encode(const char* name, char* buf, hyu32 bufSize)
{
    hyu32 n = 0;
    bool asconly = true;
    const char* p = name;
    char c = *p;
    const char* pNonAsc;
    *buf = '\0'; // false‚ð•Ô‚·ê‡‚àŠmŽÀ‚Éterminate‚³‚ê‚½•¶Žš—ñ‚Æ‚È‚é‚æ‚¤
    while (c != '\0') {
        if (asconly) {
            if (! ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_')) ) {
                asconly = false;
                pNonAsc = p;
            }
        }
        c = *++p;
        ++n;
    }
    if (asconly) {
        if (n + 2 >= bufSize)
            return false;
        *buf++ = 'a';
        *buf++ = '_';
        HMD_STRNCPY(buf, name, n+1);
        return true;
    }

    if (p == pNonAsc + 1 && p > name + 1) {
        c = *pNonAsc;
        if (c == '?' || c == '!' || c == '=') {
            if (n + 1 >= bufSize)
                return false;
            if (c == '?')
                *buf++ = 'q';
            else if (c == '!')
                *buf++ = 'e';
            else
                *buf++ = 's';
            *buf++ = '_';
            HMD_STRNCPY(buf, name, n);
            return true;
        }
    }

    if (HMD_STRNCMP(pNonAsc, ".hyb", 5) == 0) {
        *buf++ = 'b';
        *buf++ = '_';
        hyu32 len = pNonAsc-name;
        if (len + 2 >= bufSize)
            return false;
        HMD_STRNCPY(buf, name, len+1);
        buf[len] = '\0';
        return true;
    }

    if (n * 2 + 2 >= bufSize)
        return false;
    *buf++ = 'x';
    *buf++ = '_';
    for ( ; n > 0; --n) {
        HMD_SNPRINTF(buf, 3, "%02x", *name);
        buf += 2;
        ++name;
    }
    *buf = '\0';
    return true;
}

char* Ident::encode(const char* name, const char* head, hyu32 extraMem)
{
    char* buf = (char*)HMD_ALLOCA(256);
    if (! encode(name, buf, 256))
        return NULL;

    hyu32 headLen = HMD_STRLEN(head);
    hyu32 nameLen = HMD_STRLEN(buf);
    char* mem = gMemPool->allocT<char>(headLen + nameLen + extraMem + 1);
    HMD_STRNCPY(mem, head, headLen + 1);
    HMD_STRNCPY(mem + headLen, buf, nameLen + 1);
    return mem;
}
