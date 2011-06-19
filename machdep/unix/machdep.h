/*  -*- coding: sjis-dos; -*-  */

/*
 * �@��ˑ��R�[�h
 * UNIX / cygwin �p
 */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_MACHDEP_UNIX_H_
#define m_MACHDEP_UNIX_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <setjmp.h>


#define HMD_EXTERN_DECL   extern
#define HMD_DLL_EXPORT


namespace Hayat {
    namespace Common {
        class MemPool;
    }
}




#define PATH_DELIM      '/'
#define PATH_DELIM_S    "/"


// HMD = Hayat Machine Dependent

#define HMD_DEBUG       1


typedef char            hys8;
typedef short           hys16;
typedef int             hys32;
typedef unsigned char   hyu8;
typedef unsigned short  hyu16;
typedef unsigned int    hyu32;
typedef float           hyf32;
typedef double          hyf64;

#define SYMBOL_ID_IS_HYU16
#undef  SYMBOL_ID_IS_HYU32

extern void* malloc(size_t);
extern void free(void*);


#define HMD_ALLOC(size) malloc(size)
#define HMD_FREE(m_pointer) \
{ \
    if (m_pointer) { \
        free((void*)(m_pointer));             \
        m_pointer = NULL; \
    } \
}
#define HMD_ALLOCA(size) alloca(size)

#define HMD_STRLEN(x) strlen(x)
#define HMD_STRCMP(x,y) strcmp(x,y)
#define HMD_STRCASECMP(x,y) strcasecmp(x,y)
#define HMD_STRNCMP(x,y,n) strncmp(x,y,n)
#define HMD_MEMCPY(x,y,n) memcpy(x,y,n)
#define HMD_MEMCMP(x,y,n) memcmp(x,y,n)
#define HMD_STRNCPY(x,y,n) hmd_strncpy(x,y,n)
#define HMD_STRSCAT(x,y,n) strncat(x,y,(n)-strlen(x))

#define HMD_SNPRINTF(...) snprintf(__VA_ARGS__)
#define HMD_VSNPRINTF(s,n,f,ap) vsnprintf(s,n,f,ap)



#ifdef __cplusplus
extern "C" {
#else
    typedef int bool;
#endif
extern char* hmd_strncpy(char* dst, const char* src, size_t len);
extern bool hmd_isFileExist(const char* fpath);
extern hyu32 hmd_fileSize(const char* fpath);
extern hyu32 hmd_loadFile(hyu8* buf, const char* fpath, hyu32 size);
extern hyu32 hmd_loadFileAll(const char* fpath, hyu8** pFileBuf, Hayat::Common::MemPool* pool = NULL);
extern hyu32 hmd_loadFile_inPath(const char* fname, hyu8** pFileBuf, char** pPathBuf = NULL, Hayat::Common::MemPool* pool = NULL);
extern hyu8* hmd_allocFileBuf(hyu32 size, Hayat::Common::MemPool* pool = NULL);
extern void hmd_freeFileBuf(hyu8* fileBuf, Hayat::Common::MemPool* pool = NULL);
extern bool hmd_isNoWait(void);
// �ȉ��R���p�C���p�֐�
inline FILE* hmd_fopen(const char* path, const char* mode) { return fopen(path, mode); }
extern void hmd_rm_file(const char* fpath);
extern void hmd_mv_file(const char* srcPath, const char* dstPath);
extern void hmd_mkdir(const char* path, int mode);
extern void hmd_chdir(const char* path);
extern size_t hmd_wcrtomb(char* mb, wchar_t wc, mbstate_t* s);
extern void hmd_sleep(hyu32 sec);
extern bool hmd_lockFile(const char* path); // ������2�ȏ��path�����b�N�ł��Ȃ�
extern void hmd_unlockFile(void);
// �ȏ�R���p�C���p�֐�
extern void hmd_halt(void);
#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
extern "C" {
#endif
extern void hmd_printf(const char*, ...);
extern void hmd_vprintf(const char*, va_list);
extern void hmd_errprintf(const char*,int,const char*, ...);
extern void hmd_fatalError(const char*,int,const char*, ...);
#ifdef __cplusplus
}
#endif


#ifdef HMD_DEBUG
// �f�o�b�O���[�h



// �W���o�͂Ƀ��b�Z�[�W�o��
#define HMD_PRINTF(...)         hmd_printf(__VA_ARGS__)
#define HMD_VPRINTF(fmt,ap)     hmd_vprintf((fmt),(ap))
// �W���G���[�o�͂Ƀ��b�Z�[�W�o�́F���s�ʒu�\���t��
#define HMD_ERRPRINTF(...)      hmd_errprintf(__FILE__,__LINE__,__VA_ARGS__)
// ��������������Ȃ��ꍇ�̓G���[���b�Z�[�W�o�͂��Ē�~ �F HMD_DEBUG�L�����̂ݓ���
#define HMD_DEBUG_ASSERTMSG(cond,...) if(!(cond))hmd_fatalError(__FILE__,__LINE__,__VA_ARGS__);else((void)0)
// ��������������Ȃ��ꍇ�͒�~ �F HMD_DEBUG�L�����̂ݓ���
#define HMD_DEBUG_ASSERT(cond)           HMD_DEBUG_ASSERTMSG((cond), "assertion failed: " #cond);
// ��������������Ȃ��ꍇ�̓G���[���b�Z�[�W�o�͂��Ē�~
#define HMD_ASSERTMSG(cond,...) if(!(cond))hmd_fatalError(__FILE__,__LINE__,__VA_ARGS__);else((void)0)
// ��������������Ȃ��ꍇ�͒�~
#define HMD_ASSERT(cond)           HMD_ASSERTMSG((cond), "assertion failed: " #cond);
// �G���[���b�Z�[�W�o�͂��Ē�~
#define HMD_FATAL_ERROR(...)    hmd_fatalError(__FILE__,__LINE__,__VA_ARGS__)
// ��~
#define HMD_HALT()              hmd_halt()
#ifdef __cplusplus
template <typename T> void m_hmd_assert_equal(T a, T b, const char* mes) {
    if (a != b)
        HMD_FATAL_ERROR(mes);
}
// 2�̒l���r���ĈقȂ��Ă���ꍇ�͒�~ �F C++��HMD_DEBUG�L�����̂ݓ���
#define HMD_DEBUG_ASSERT_EQUAL(a,b) m_hmd_assert_equal(a,b, "HMD_DEBUG_ASSERT_EQUAL failed: " #a " != " #b)
#else
#define HMD_DEBUG_ASSERT_EQUAL(a,b)   ((void)0)
#endif



#else // HMD_DEBUG
// �f�o�b�O���[�h�ł͂Ȃ�

#define HMD_PRINTF(...)         hmd_printf(__VA_ARGS__)
#define HMD_VPRINTF(fmt,ap)     hmd_vprintf((fmt),(ap))
#define HMD_ERRPRINTF(...)      hmd_errprintf(__FILE__,__LINE__,__VA_ARGS__)
#define HMD_DEBUG_ASSERT(x)     ((void)0)
#define HMD_DEBUG_ASSERTMSG(...)        ((void)0)
#define HMD_FATAL_ERROR(...)    hmd_fatalError(__FILE__,__LINE__,__VA_ARGS__)
#define HMD_HALT()              hmd_halt()
#define HMD_ASSERTMSG(cond,...) if(!(cond))hmd_fatalError(__FILE__,__LINE__,__VA_ARGS__);else((void)0)
#define HMD_ASSERT(cond)        HMD_ASSERTMSG((cond), "assertion failed: " #cond);
#define HMD_DEBUG_ASSERT_EQUAL(a,b)   ((void)0)


#endif // HMD_DEBUG 


// hmd_halt() ���ɔ�ԏꏊ
extern jmp_buf* hmd_halt_jmpbuf;


#ifdef TEST__CPPUNIT

// fn �����s���� hmd_halt() ���Ă΂�Ȃ���ΐ���
#define HMD_ASSERT_NO_HALT(fn)                  \
    { jmp_buf jmpBuf;                           \
        if (setjmp(jmpBuf) == 0) {              \
            hmd_halt_jmpbuf = &jmpBuf;          \
            (fn);                               \
        } else {                                \
            CPPUNIT_FAIL( "HMD_ASSERT_NO_HALT(" #fn ")" );      \
        }                                       \
    }

// fn �����s���� hmd_halt() ���Ă΂��ΐ���
#define HMD_ASSERT_HALT(fn)                     \
    { jmp_buf jmpBuf;                           \
        if (setjmp(jmpBuf) == 0) {              \
            hmd_halt_jmpbuf = &jmpBuf;          \
            (fn);                               \
            CPPUNIT_FAIL( "HMD_ASSERT_HALT(" #fn ")" ); \
        }                                       \
    }

#endif


#endif /* m_MACHDEP_UNIX_H_ */
