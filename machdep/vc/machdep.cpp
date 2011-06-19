/*  -*- coding: sjis-dos; -*-  */

/*
 * 機種依存コード
 * Visual C++ 用
 */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "machdep.h"
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>
#include <direct.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>

#include <tchar.h>
#include <windows.h>


#include "hyMemPool.h"


// hmd_loadFile_inPathで使うロードパス宣言
//HMD_DLL_EXPORT const char** HMD_LOADPATH;


#ifndef TEST__CPPUNIT
#ifndef HY_WORK_WITHOUT_VM
#include "hyVM.h"
#endif
#endif


static int hmd_debug_out_mode = HMD_DEBUG_OUT_STD;
HMD_DLL_EXPORT void hmd_set_debug_out_mode(int mode)
{
    hmd_debug_out_mode = mode;
}


HMD_DLL_EXPORT void hmd_vprintf(const char* msg, va_list ap)
{
    char tmp[4096];
    vsprintf_s(tmp, 4096, msg, ap);
    if (hmd_debug_out_mode == HMD_DEBUG_OUT_STD) {
        fflush(stderr);
        fputs(tmp, stdout);
        fflush(stdout);
    } else {
        OutputDebugStringA(tmp);
    }
}

void hmd_vprintf_stderr(const char* msg, va_list ap)
{
    char tmp[4096];
    vsprintf_s(tmp, 4096, msg, ap);
    if (hmd_debug_out_mode == HMD_DEBUG_OUT_STD) {
        fflush(stdout);
        fputs(tmp, stderr);
        fflush(stderr);
    } else {
        OutputDebugStringA(tmp);
    }
}

HMD_DLL_EXPORT void hmd_printf(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    hmd_vprintf(msg, ap);
    va_end(ap);
}

void hmd_printf_stderr(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    hmd_vprintf_stderr(msg, ap);
    va_end(ap);
}

HMD_DLL_EXPORT void hmd_errprintf(const char* file, int line, const char* msg, ...)
{
    hmd_printf_stderr("%s:%d: ", file, line);
    va_list ap;
    va_start(ap, msg);
    hmd_vprintf_stderr(msg, ap);
    va_end(ap);
    hmd_printf_stderr("\n");
}

HMD_DLL_EXPORT void hmd_fatalError(const char* file, int line, const char* msg, ...)
{
#ifndef TEST__CPPUNIT
#  ifndef HY_WORK_WITHOUT_VM
    Hayat::Engine::VM::debugStackTrace();
#  endif
    hmd_printf("%s:%d: ", file, line);
    va_list ap;
    va_start(ap, msg);
    hmd_vprintf(msg, ap);
    va_end(ap);
    hmd_printf("\n");
#endif

    hmd_halt();
}


// hmd_halt() 時に飛ぶ場所
HMD_DLL_EXPORT jmp_buf* hmd_halt_jmpbuf = NULL;

HMD_DLL_EXPORT void hmd_halt(void)
{
    fflush(stdout);
    fflush(stderr);
    if (hmd_halt_jmpbuf != NULL) {
        jmp_buf* tmp = hmd_halt_jmpbuf;
        hmd_halt_jmpbuf = NULL;
        longjmp(*tmp, -1);
    }

    hmd_printf_stderr("Hayat HALT\n");
#ifdef HY_WORK_WITHOUT_VM
    throw "halt";
#endif
#ifdef _DEBUG
    *((char*)0) = *((char*)1);  // segmentation fault
    volatile int aa = 0;
    aa = 1 / aa;                // division by zero
    for(;;);                    // infinity loop
#else
    exit(99);
#endif
}


HMD_DLL_EXPORT bool hmd_isFileExist(const char* fpath)
{
    struct stat statBuf;
    return (stat(fpath, &statBuf) == 0);
}

HMD_DLL_EXPORT hyu32 hmd_fileSize(const char* fpath)
{
    struct stat statBuf;
    if (stat(fpath, &statBuf) != 0) {
        // HMD_ERRPRINTF("can't stat file (%s)\n", fpath);
        return 0;
    }
    return (hyu32) statBuf.st_size;
}

HMD_DLL_EXPORT hyu32 hmd_loadFile(hyu8* buf, const char* fpath, hyu32 size)
{
    FILE* fp;
    errno_t eno = fopen_s(&fp, fpath, "rb");
    if (eno != 0) {
        HMD_ERRPRINTF("cannot open file (%s)\n", fpath);
        perror("");
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

HMD_DLL_EXPORT hyu32 hmd_loadFileAll(const char* fpath, hyu8** pFileBuf, Hayat::Common::MemPool* pool)
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


HMD_DLL_EXPORT hyu8* hmd_allocFileBuf(hyu32 size, Hayat::Common::MemPool* pool)
{
    if (pool == NULL)
        pool = Hayat::Common::gMemPool;
    return (hyu8*) pool->alloc(size, "FBUF");
}

HMD_DLL_EXPORT void hmd_freeFileBuf(hyu8* fileBuf, Hayat::Common::MemPool* pool)
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
    if (*fname == '\\') {
        // 絶対パス
        HMD_STRNCPY(buf, fname, fnlen + 1);
        return;
    }
    HMD_STRNCPY(buf, dir, dirlen + 1);
    HMD_STRSCAT(buf, "\\", bufSize);
    HMD_STRSCAT(buf, fname, bufSize);
}

static bool bFileExist(const char* fpath)
{
    struct stat statBuf;
    return stat(fpath, &statBuf) == 0;
}

HMD_DLL_EXPORT hyu32 hmd_loadFile_inPath(const char* fname, hyu8** pFileBuf, char** pPathBuf, Hayat::Common::MemPool* pool)
{
    char buf[256];
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



HMD_DLL_EXPORT bool hmd_isNoWait(void)
{
    return false;
}

void hmd_rm_file(const char* fpath)
{
    _unlink(fpath);
}

void hmd_mv_file(const char* srcPath, const char* dstPath)
{
    _unlink(dstPath);
    if (rename(srcPath, dstPath) == 0)
        return;     // OK
    perror("cannot rename file");
    fprintf(stderr,"cannot rename '%s' to '%s'\n", srcPath, dstPath);
    HMD_FATAL_ERROR("cannot rename file");
}

void hmd_mkdir(const char* path, int)
{
    _mkdir(path);
}

void hmd_chdir(const char* path)
{
    if (_chdir(path) != 0) {
        perror("cannot chdir");
        fprintf(stderr,"cannot chdir to '%s'\n", path);
        HMD_FATAL_ERROR("cannot chdir");
    }
}


FILE* hmd_fopen(const char* path, const char* mode)
{
    FILE* fp;
    errno_t eno = fopen_s(&fp, path, mode);
    if (eno != 0)
        return NULL;
    return fp;
}

size_t hmd_wcrtomb(char* mb, wchar_t wc, mbstate_t* s)
{
    size_t retval;
    errno_t eno;
    eno = wcrtomb_s(&retval, mb, 4, wc, s);
    if (eno != 0) {
        perror("");
        HMD_FATAL_ERROR("wcrtomb failed");
    }
    return retval;
}

void hmd_sleep(hyu32 sec)
{
    Sleep(sec * 1000);
}


// 同時に2つ以上のpathをロックできない
static int lock_fh = -1;
static const char* lock_path = NULL;
bool hmd_lockFile(const char* path)
{
#if 0
    if (lock_fh != -1)
        return false;
#endif
    int fh;
    errno_t eno = _sopen_s(&fh, path, _O_CREAT | _O_TEMPORARY | _O_EXCL, _SH_DENYNO, _S_IWRITE);
    if (eno != 0)
        return false;
    lock_fh = fh;
    lock_path = path;
    return true;
}

void hmd_unlockFile(void)
{
    if (lock_fh != -1) {
        _close(lock_fh);
        lock_fh = -1;
    }
    if (lock_path != NULL) {
        _unlink(lock_path);
        lock_path = NULL;
    }
}
