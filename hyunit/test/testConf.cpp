/*  -*- coding: sjis-dos; -*-  */


// hmd_loadFile_inPathで使うロードパス定義
const char* m_HMD_LOADPATH[] = {
#ifndef WIN32
    "out",
    "../out",
    "../../stdlib/out",
    "../../engine",             //  for mm_sjis.mm
    (const char*)0
#else
    "..\\..\\hyunit\\test\\out",
    "..\\..\\hyunit\\out",
    "..\\..\\stdlib\\out",
    "..\\..\\engine",           //  for mm_sjis.mm
    (const char*)0
#endif
};
const char** HMD_LOADPATH = m_HMD_LOADPATH;



#include "hyClass.h"
EXTERN_BYTECODE_FFI(hyTest_ffi);
EXTERN_BYTECODE_FFI(hyTest_inherit);

void hySuiteFfiBinder(void)
{
    LINK_BYTECODE_FFI(hyTest_ffi);
    LINK_BYTECODE_FFI(hyTest_inherit);
}
