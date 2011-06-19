/*  -*- coding: sjis-dos; -*-  */

/*
 * �@��ˑ��R�[�h
 * Visual C++ �p
 */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_MACHDEP_VCPP_H_
#define m_MACHDEP_VCPP_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <wchar.h>
#include <setjmp.h>


#ifdef MS_DLL_EXPORT_DECL
# define HMD_EXTERN_DECL    extern _declspec(dllexport)
# define HMD_DLL_EXPORT     _declspec(dllexport)
#else
# define HMD_DLL_EXPORT
# ifdef MS_DLL_IMPORT_DECL
#  define HMD_EXTERN_DECL   extern _declspec(dllimport)
# else
#  define HMD_EXTERN_DECL   extern
# endif
#endif


namespace Hayat {
    namespace Common {
        class MemPool;
    }
}


#define PATH_DELIM      '\\'
#define PATH_DELIM_S    "\\"


// HMD = Hayat Machine Dependent

#ifdef _DEBUG
#define HMD_DEBUG       1
#else
#undef HMD_DEBUG
#endif


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



#define HMD_ALLOC(size) malloc(size)
#define HMD_FREE(m_pointer) \
{ \
    if (m_pointer) { \
        free((void*)(m_pointer));             \
        m_pointer = NULL; \
    } \
}
#define HMD_ALLOCA(size) _alloca(size)

#define HMD_STRLEN(x) ((hyu32)strlen(x))
#define HMD_STRCMP(x,y) strcmp(x,y)
#define HMD_STRCASECMP(x,y) _stricmp(x,y)
#define HMD_STRNCMP(x,y,n) strncmp(x,y,n)
#define HMD_MEMCPY(x,y,n) memcpy(x,y,n)
#define HMD_MEMCMP(x,y,n) memcmp(x,y,n)
#define HMD_STRNCPY(x,y,n) strncpy_s(x,(size_t)(n),y,_TRUNCATE)
#define HMD_STRSCAT(x,y,n) strcat_s(x,(size_t)(n),y)

#define HMD_SNPRINTF(buf,size,...) _snprintf_s(buf,size,_TRUNCATE,__VA_ARGS__)
#define HMD_VSNPRINTF(s,n,f,ap) vsnprintf_s(s,n,_TRUNCATE,f,ap)



#ifdef __cplusplus
extern "C" {
#else
    typedef int bool;
#endif
// �f�o�b�O�o�͂� stdout,stderr ���g��
#define HMD_DEBUG_OUT_STD   0
// �f�o�b�O�o�͂� OutputDebugString ���g��
#define HMD_DEBUG_OUT_ODS   1
HMD_EXTERN_DECL void hmd_set_debug_out_mode(int mode);
HMD_EXTERN_DECL bool hmd_isFileExist(const char* fpath);
HMD_EXTERN_DECL hyu32 hmd_fileSize(const char* fpath);
HMD_EXTERN_DECL hyu32 hmd_loadFile(hyu8* buf, const char* fpath, hyu32 size);
HMD_EXTERN_DECL hyu32 hmd_loadFileAll(const char* fpath, hyu8** pFileBuf, Hayat::Common::MemPool* pool = NULL);
HMD_EXTERN_DECL hyu32 hmd_loadFile_inPath(const char* fname, hyu8** pFileBuf, char** pPathBuf = NULL, Hayat::Common::MemPool* pool = NULL);
HMD_EXTERN_DECL hyu8* hmd_allocFileBuf(hyu32 size, Hayat::Common::MemPool* pool = NULL);
HMD_EXTERN_DECL void hmd_freeFileBuf(hyu8* fileBuf, Hayat::Common::MemPool* pool = NULL);
HMD_EXTERN_DECL bool hmd_isNoWait(void);
// �ȉ��R���p�C���p�֐�
HMD_EXTERN_DECL FILE* hmd_fopen(const char* path, const char* mode);
HMD_EXTERN_DECL void hmd_rm_file(const char* fpath);
HMD_EXTERN_DECL void hmd_mv_file(const char* srcPath, const char* dstPath);
HMD_EXTERN_DECL void hmd_mkdir(const char* path, int mode);
HMD_EXTERN_DECL void hmd_chdir(const char* path);
HMD_EXTERN_DECL size_t hmd_wcrtomb(char* mb, wchar_t wc, mbstate_t* s);
HMD_EXTERN_DECL void hmd_sleep(hyu32 sec);
HMD_EXTERN_DECL bool hmd_lockFile(const char* path); // ������2�ȏ��path�����b�N�ł��Ȃ�
HMD_EXTERN_DECL void hmd_unlockFile(void);
// �ȏ�R���p�C���p�֐�
HMD_EXTERN_DECL void hmd_halt(void);
HMD_EXTERN_DECL void hmd_printf(const char*, ...);
HMD_EXTERN_DECL void hmd_vprintf(const char*, va_list);
HMD_EXTERN_DECL void hmd_errprintf(const char*,int,const char*, ...);
HMD_EXTERN_DECL void hmd_fatalError(const char*,int,const char*, ...);
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


// hmd_loadFile_inPath�Ŏg�����[�h�p�X�錾
HMD_EXTERN_DECL const char** HMD_LOADPATH;
// hmd_halt() ���ɔ�ԏꏊ
HMD_EXTERN_DECL jmp_buf* hmd_halt_jmpbuf;


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


#include <float.h>
static inline int isinf(double n) {
    return (!_finite(n) && !_isnan(n));
}


#endif /* m_MACHDEP_VCPP_H_ */
