
#include "hpSourceInfo.h"
#include "hyMemPool.h"


using namespace Hayat::Common;
using namespace Hayat::Parser;


void* SourceInfo::operator new(size_t size)
{
    return gMemPool->alloc(size, "sinf");
}

void SourceInfo::operator delete(void* p)
{
    if (p != NULL)
        gMemPool->free(p);
}

SourceInfo::SourceInfo(void)
    : line(0), col(0), fname(NULL)
{
}

void SourceInfo::printns(char* dest, hyu32 destSize)
{
    if (fname == NULL)
        HMD_SNPRINTF(dest, destSize, "line=%d,col=%d", line, col);
    else
        HMD_SNPRINTF(dest, destSize, "%s:%d:%d", fname, line, col);
#if 0
    if (fname == NULL)
        HMD_SNPRINTF(dest, destSize, "line=%d,col=%d", line, col);
    else
        HMD_SNPRINTF(dest, destSize, "file=%s,line=%d,col=%d", fname, line, col);
#endif
}
