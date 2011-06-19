/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCCommandLine.h"
#include "hyMemPool.h"

using namespace Hayat::Compiler;
using namespace Hayat::Common;


void Hayat::Compiler::addLoadPath(const char* path, const char* subdir)
{
    hyu32 pathLen = HMD_STRLEN(path);
    char* p = gMemPool->allocT<char>(pathLen+1, "fnlp");
    HMD_STRNCPY(p, path, pathLen+1);
    if (subdir == NULL) {
        HMD_LOADPATH = gMemPool->reallocT<const char*>(HMD_LOADPATH, numLoadPath + 2);
        HMD_LOADPATH[numLoadPath++] = p;
    } else {
        HMD_LOADPATH = gMemPool->reallocT<const char*>(HMD_LOADPATH, numLoadPath + 3);
        HMD_LOADPATH[numLoadPath++] = p;
        hyu32 sdLen = HMD_STRLEN(subdir);
        p = gMemPool->allocT<char>(pathLen + sdLen + 2, "fnlp");
        HMD_STRNCPY(p, path, pathLen+1);
        HMD_STRSCAT(p, PATH_DELIM_S, pathLen + sdLen + 2);
        HMD_STRSCAT(p, subdir, pathLen + sdLen + 2);
        HMD_LOADPATH[numLoadPath++] = p;
    }
    HMD_LOADPATH[numLoadPath] = NULL;
}

void Hayat::Compiler::finalizeLoadPath(void)
{
    for (hyu32 i = 0; i < numLoadPath; ++i)
        gMemPool->free((void*)HMD_LOADPATH[i]);
    gMemPool->free(HMD_LOADPATH);
    HMD_LOADPATH = NULL;
    numLoadPath = 0;
}


const char* Hayat::Compiler::getParam(int& i, int argc, const char* argv[], const char* pname)
{
    int n = HMD_STRLEN(pname);
    if (argv[i][n] != '\0') {
        return &argv[i][n];
    } else if (++i < argc) {
        return &argv[i][0];
    }
    printf("warning: no parameter of %s\n", pname);
    return NULL;
}

int Hayat::Compiler::getWarnLevel(const char* opt, const char* arg)
{
    hyu32 optLen = HMD_STRLEN(opt);
    if (0 != strncmp(opt, arg, optLen))
        return -1;
    const char* p = &arg[optLen];
    char c = *p;
    if (c == '\0')
        return 0;
    if (c != '=')
        return -2;
    c = *++p;
    if (c == '\0')
        return 0;
    int v = 0;
    while (c >= '0' && c <= '9') {
        v *= 10;
        v += c - '0';
        c = *++p;
    }
    v *= 2;
    if (c == '!') {
        v += 1;
        c = *++p;
    }
    if (c != '\0')
        return -3;
    return v;
}
