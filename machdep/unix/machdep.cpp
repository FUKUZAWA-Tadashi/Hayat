/*  -*- coding: sjis-dos; -*-  */

/*
 * 機種依存コード
 * UNIX / cygwin 用
 */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "machdep.h"
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


#include "hyMemPool.h"


// hmd_loadFile_inPathで使うロードパス宣言
extern const char** HMD_LOADPATH;


#ifndef TEST__CPPUNIT
#ifndef HY_WORK_WITHOUT_VM
#include "hyVM.h"
#endif
#endif




char* hmd_strncpy(char* dst, const char* src, size_t len)
{
    strncpy(dst, src, len);
    dst[len-1] = '\0';
    return dst;
}

void hmd_printf(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    fflush(stderr);
    vfprintf(stdout, msg, ap);
    va_end(ap);
    fflush(stdout);
}

void hmd_vprintf(const char* msg, va_list ap)
{
    fflush(stderr);
    vfprintf(stdout, msg, ap);
    fflush(stdout);
}


void hmd_errprintf(const char* file, int line, const char* msg, ...)
{
    char tmp[1024];
    va_list ap;
    va_start(ap, msg);
    HMD_SNPRINTF(tmp, 1024, "%s:%d: %s\n", file, line, msg);
    fflush(stdout);
    vfprintf(stderr, tmp, ap);
    va_end(ap);
    fflush(stderr);
}

void hmd_fatalError(const char* file, int line, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
#ifndef TEST__CPPUNIT
    char tmp[1024];
#ifndef HY_WORK_WITHOUT_VM
    Hayat::Engine::VM::debugStackTrace();
#endif
    HMD_SNPRINTF(tmp, 1024, "%s:%d: %s\n", file, line, msg);
    fflush(stdout);
    vfprintf(stderr, tmp, ap);
#endif
    va_end(ap);
    fflush(stderr);
    hmd_halt();
}



// hmd_halt() 時に飛ぶ場所
jmp_buf* hmd_halt_jmpbuf = NULL;

void hmd_halt(void)
{
    fflush(stdout);
    fflush(stderr);
    if (hmd_halt_jmpbuf != NULL) {
        jmp_buf* tmp = hmd_halt_jmpbuf;
        hmd_halt_jmpbuf = NULL;
        longjmp(*tmp, -1);
    }

    fprintf(stderr, "Hayat HALT\n");
    fflush(stderr);
#ifdef HY_WORK_WITHOUT_VM
    throw "halt";
#endif
    *((char*)0) = *((char*)1);  // segmentation fault
    volatile int aa = 0;
    aa = 1 / aa;                // division by zero
    for(;;);                    // infinity loop
}



bool hmd_isFileExist(const char* fpath)
{
    struct stat statBuf;
    return (stat(fpath, &statBuf) == 0);
}

hyu32 hmd_fileSize(const char* fpath)
{
    struct stat statBuf;
    if (stat(fpath, &statBuf) != 0) {
        // HMD_ERRPRINTF("can't stat file (%s)\n", fpath);
        return 0;
    }
    return (hyu32) statBuf.st_size;
}

hyu32 hmd_loadFile(hyu8* buf, const char* fpath, hyu32 size)
{
    FILE* fp = fopen(fpath, "rb");
    if (fp == NULL) {
        HMD_ERRPRINTF("cannot open file (%s)\n", fpath);
        return 0;
    } else {
        size_t nread = fread(buf, 1, size, fp);
        fclose(fp);
        return (hyu32) nread;
    }
}


// ファイルを全部読み込めるサイズのメモリを pool に確保し、
// そこにファイルを読み込む。
// poolがNULLなら、gMemPoolを使う。

hyu32 hmd_loadFileAll(const char* fpath, hyu8** pFileBuf, Hayat::Common::MemPool* pool)
{
    if (pool == NULL)
        pool = Hayat::Common::gMemPool;
    *pFileBuf = NULL;
    if (fpath == NULL) {
        return 0;
    }
    hyu32 length = hmd_fileSize(fpath);
    if (length == 0)
        return 0;

    *pFileBuf = hmd_allocFileBuf(length, pool);
    if (*pFileBuf == NULL) {
        HMD_ERRPRINTF("can't allocate memory for file: %s\n", fpath);
        return 0;
    }
    hyu32 nread = hmd_loadFile(*pFileBuf, fpath, length);
    if (nread != length) {
        HMD_ERRPRINTF("file load failed: %s\n", fpath);
        hmd_freeFileBuf(*pFileBuf, pool);
        *pFileBuf = NULL;
        return 0;
    }
    return nread;
}


hyu8* hmd_allocFileBuf(hyu32 size, Hayat::Common::MemPool* pool)
{
    if (pool == NULL)
        pool = Hayat::Common::gMemPool;
    return pool->allocT<hyu8>(size, "FBUF");
}

void hmd_freeFileBuf(hyu8* fileBuf, Hayat::Common::MemPool* pool)
{
    if (pool == NULL)
        pool = Hayat::Common::gMemPool;
    pool->free(fileBuf);
}



static void expandPath(const char* fname, const char* dir, char* buf, size_t bufSize)
{
    hyu32 fnlen = HMD_STRLEN(fname);
    hyu32 dirlen = HMD_STRLEN(dir);
    HMD_ASSERT(fnlen + dirlen + 1 < bufSize);
    if (*fname == '/') {
        // 絶対パス
        HMD_STRNCPY(buf, fname, fnlen + 1);
        return;
    }
    HMD_STRNCPY(buf, dir, dirlen + 1);
    HMD_STRSCAT(buf, "/", bufSize);
    HMD_STRSCAT(buf, fname, bufSize);
}

static bool bFileExist(const char* fpath)
{
    struct stat statBuf;
    return stat(fpath, &statBuf) == 0;
}

hyu32 hmd_loadFile_inPath(const char* fname, hyu8** pFileBuf, char** pPathBuf, Hayat::Common::MemPool* pool)
{
    hyu32 length;
    if (pool == NULL)
        pool = Hayat::Common::gMemPool;
    if (bFileExist(fname)) {
        length = hmd_loadFileAll(fname, pFileBuf, pool);
        if (length > 0) {
            if (pPathBuf != NULL) {
                int pblen = HMD_STRLEN(fname);
                *pPathBuf = pool->allocT<char>(pblen+1, "FNam");
                HMD_STRNCPY(*pPathBuf, fname, pblen+1);
            }
            return length;
        }
    }

    if (HMD_LOADPATH == NULL)
        return 0;

    char buf[256];
    for (int i = 0; HMD_LOADPATH[i] != NULL; ++i) {
        expandPath(fname, HMD_LOADPATH[i], buf, 256);
        if (bFileExist(buf)) {
            length = hmd_loadFileAll(buf, pFileBuf, pool);
            if (length > 0) {
                if (pPathBuf != NULL) {
                    int pblen = HMD_STRLEN(buf);
                    *pPathBuf = pool->allocT<char>(pblen+1, "FNam");
                    HMD_STRNCPY(*pPathBuf, buf, pblen+1);
                }
                return length;
            }
        }
    }
    return 0;
}



bool hmd_isNoWait(void)
{
    return false;
}

void hmd_rm_file(const char* fpath)
{
    unlink(fpath);
}

void hmd_mv_file(const char* srcPath, const char* dstPath)
{
    // cygwinだと、なぜか permission denied で rename できない事がある
    // close直後だと起こりやすい？？
    // sleepを挟んでリトライしてみる 
    int retryCount = 3;
    while (--retryCount >= 0) {
        if (rename(srcPath, dstPath) == 0)
            return;     // OK
        sleep(1);
    }
    fprintf(stderr,"cannot rename '%s' to '%s'\n", srcPath, dstPath);
    perror("");
    hmd_halt();
}

void hmd_mkdir(const char* path, int mode)
{
    mkdir(path, mode);
}

void hmd_chdir(const char* path)
{
    chdir(path);
}

size_t hmd_wcrtomb(char* mb, wchar_t wc, mbstate_t* s)
{
    return wcrtomb(mb, wc, s);
}


void hmd_sleep(hyu32 sec)
{
    sleep(sec);
}


// 同時に2つ以上のpathをロックできない
static int lock_fd = -1;
static const char* lock_path = NULL;
bool hmd_lockFile(const char* path)
{
    if (lock_fd != -1)
        return false;
    int fd = open(path, O_CREAT | O_EXCL);
    if (fd == -1)
        return false;
    lock_fd = fd;
    lock_path = path;
    return true;
}

void hmd_unlockFile(void)
{
    if (lock_fd != -1) {
        close(lock_fd);
        lock_fd = -1;
    }
    if (lock_path != NULL) {
        unlink(lock_path);
        lock_path = NULL;
    }
}
