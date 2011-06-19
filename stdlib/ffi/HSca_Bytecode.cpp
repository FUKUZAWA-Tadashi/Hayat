/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Bytecode.h"
#include "hySymbolTable.h"
#include "hyCodeManager.h"

using namespace Hayat::Common;

FFI_DEFINITION_START {

    void FFI_FUNC(reload) (Value selfVal, const char* filename)
    {
#ifdef HY_ENABLE_BYTECODE_RELOAD
        FFI_DEBUG_ASSERT_CLASSMETHOD(selfVal);
        if (gCodeManager.reloadBytecode(filename) == NULL) {
            VM::getContext()->throwException(HSym_file_reload_failed, M_M("file %s reload failed"), filename);
            return;
        }
#else
        VM::getContext()->throwException(HSym_feature_not_available, M_M("reload feature not available"), filename);
#endif
    }

    void FFI_FUNC(reloadAs) (Value selfVal, const char* filename, SymbolID_t name)
    {
#ifdef HY_ENABLE_BYTECODE_RELOAD
        FFI_DEBUG_ASSERT_CLASSMETHOD(selfVal);
        if (gCodeManager.reloadBytecode(filename, name) == NULL) {
            VM::getContext()->throwException(HSym_file_reload_failed, M_M("file %s reload as %s failed"), filename, gSymbolTable.id2str(name));
            return;
        }
#else
        VM::getContext()->throwException(HSym_feature_not_available, M_M("reload feature not available"), filename);
#endif
    }

    void FFI_FUNC(setObsoleteVarKeptOnReload) (Value selfVal, bool flag)
    {
        FFI_DEBUG_ASSERT_CLASSMETHOD(selfVal);
        gCodeManager.setKeepObsoleteVar_whenReload(flag);
    }
    
    void FFI_FUNC(setClassVarCopiedOnReload) (Value selfVal, bool flag)
    {
        FFI_DEBUG_ASSERT_CLASSMETHOD(selfVal);
        gCodeManager.setCopyClassVar_whenReload(flag);
    }
    
    void FFI_FUNC(setConstVarCopiedOnReload) (Value selfVal, bool flag)
    {
        FFI_DEBUG_ASSERT_CLASSMETHOD(selfVal);
        gCodeManager.setCopyConstVar_whenReload(flag);
    }

    ValueArray* FFI_FUNC(remains) (Value selfVal)
    {
        FFI_DEBUG_ASSERT_CLASSMETHOD(selfVal);
        ValueArray* arr = new ValueArray(0);
        gCodeManager.takeReplacedBytecodeSyms(arr);
        return arr;
    }


} FFI_DEFINITION_END
