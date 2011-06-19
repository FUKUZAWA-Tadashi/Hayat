/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "machdep.h"
#include "hyDebugSourceInfo.h"
#include "hyMemPool.h"

using namespace Hayat::Common;
using namespace Hayat::Engine;


static const int MEM_SIZE = 16 * 1024 * 1024;

const char** HMD_LOADPATH = NULL;


int main(int argc, const char* argv[])
{
    if (argc < 2) {
        HMD_PRINTF("Hayat debug info file dumper\n");
        HMD_PRINTF("Usage: hydebdump filename.deb\n");
        return 1;
    }

    void* hayatMemory = HMD_ALLOC(MEM_SIZE);
    MemPool::initGMemPool(hayatMemory, MEM_SIZE);

    DebugInfos* dinf = new DebugInfos(argv[1]);
    
    hyu32 num = dinf->size();
    char buf[1024];

    HMD_PRINTF("addr:line:co:file\n");
    for (hyu32 i = 0; i < num; ++i) {
        dinf->getNthInfo(buf, 1024, i);
        HMD_PRINTF("%s\n", buf);
    }

    HMD_FREE(hayatMemory);
    return 0 ;
}
