/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "stdafx.h"
#include "EngineControl.h"
#include "hayat.h"


// DLL���[�h���Ƀ��b�Z�[�W�o��
#define VERBOSE_LOAD_DLL

#ifdef MS_DLL_EXPORT_DECL
# define DLL_EXPORT     _declspec(dllexport)
#else
# define DLL_EXPORT
#endif


using namespace Hayat::Common;
using namespace Hayat::Engine;

// ���[�h�p�X�̃A�h���X
DLL_EXPORT const char** HMD_LOADPATH = NULL;


namespace hydll {

    // ���s�t���[���J�E���g
    hyu32   g_nFrame = 0;

    // ���C��������
    size_t  mainMemSize = 4 * 1024 * 1024;
    void*   mainMem = NULL;

    // �f�o�b�O������
    size_t  debugMemSize = 0;
    void*   debugMem = NULL;
    // �f�o�b�O����ǂނ��ǂ���
    bool    bReadDebugInfos = false;

    // ���[�h�p�X���i�[���Ă����z��
    TArray<const char*> loadPathArr(0);

    // �A�b�v�f�[�g�֐���o�^���Ă����z��
    TArray<RFunc_t>     updateFuncArr(0);

    // �`��֐���o�^���Ă����z��
    TArray<RFunc_t>     drawFuncArr(0);

    // �I���������֐���o�^���Ă����z��
    TArray<RFunc_t>     termFuncArr(0);

    // DLL�̃n���h����o�^���Ă����z��
    TArray<HMODULE>     dllHandleArr(0);



    void    printLastError(void)
    {
        LPVOID lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      GetLastError(),
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // ����̌���
                      (LPTSTR) &lpMsgBuf,
                      0,
                      NULL
                      );
        HMD_PRINTF((const char*)lpMsgBuf);
        LocalFree(lpMsgBuf);
    }


    // Hayat�Ŏg�p���郁�����T�C�Y���w�肷��
    // �w�肵�Ȃ���΁A4MiB�̃��������m�ۂ���
    void    setMemSize(size_t memSize)
    {
        mainMemSize = memSize;
    }

    // Hayat�Ŏg�p����f�o�b�O�p�������T�C�Y���w�肷��
    // �w�肵�Ȃ���΁A�f�o�b�O�p�������͊m�ۂ��Ȃ�
    void    setDebugMemSize(size_t memSize)
    {
        debugMemSize = memSize;
    }

    void    setFlagReadDebugInfos(bool flag)
    {
        bReadDebugInfos = flag;
    }

    // ���[�h�p�X��ǉ�����
    void    addLoadPath(const char* path)
    {
        //printf("addLoadPath(%s)\n", path);
        hyu32 n = loadPathArr.size();
        if (n == 0) {
            loadPathArr.add(NULL);
            n = 1;
        }
        hyu32 len = HMD_STRLEN(path);
        char* apath = gMemPool->allocT<char>(len+1);
        HMD_STRNCPY(apath, path, len+1);
        loadPathArr[n-1] = apath;  // end mark �ɏ㏑��
        loadPathArr.add(NULL);     // �V���� end mark
        HMD_LOADPATH = loadPathArr.top();
        // n=0;for(const char** p = HMD_LOADPATH; *p != NULL; ++p)HMD_PRINTF("path %d:'%s'\n",n++,*p);
    }

    // 1�t���[����1��Ă΂��A�b�v�f�[�g�֐���o�^����
    void    addUpdateFunc(RFunc_t func)
    {
        updateFuncArr.add(func);
    }
    // �A�b�v�f�[�g�֐���擪�ɓo�^����
    void    insertUpdateFunc(RFunc_t func)
    {
        updateFuncArr.insert(0, 1) = func;
    }

    // 1�t���[����1��Ă΂��`��֐���o�^����
    void    addDrawFunc(RFunc_t func)
    {
        drawFuncArr.add(func);
    }

    // �I���������֐���o�^����
    void    addTermFunc(RFunc_t func)
    {
        termFuncArr.add(func);
    }



    static HMODULE LoadLibraryMB(const char* path)
    {
#ifdef _UNICODE
        int len = _mbslen((const unsigned char*)path);
        int siz = MultiByteToWideChar(0,0, path,len, NULL,0);
        TCHAR* apath = (TCHAR*)HMD_ALLOCA((siz+1) * sizeof(TCHAR));
        MultiByteToWideChar(0,0, path,len, apath,siz);
        return AfxLoadLibrary(apath);
#else
        return AfxLoadLibrary(path);
#endif
    }

    // �o�C�g�R�[�h���������Ƀ��[�h���ꂽ����̃R�[���o�b�N�B
    // �Ή�����DLL������΃��[�h����
    static void cb4dll(const char* hybPath)
    {
        int len = HMD_STRLEN(hybPath);
        char* dllPath = (char*)HMD_ALLOCA(len + sizeof(dll_prefix)+sizeof(dll_ext));
        char* q = dllPath;
        char* q2 = q;

        const unsigned char* p = _mbsrchr((const unsigned char*)hybPath, PATH_DELIM);
        if (p == NULL) {
            p = (const unsigned char*)hybPath;
        } else {
            p += 1;
            size_t lp = p - (const unsigned char*)hybPath;
            memcpy(q, hybPath, lp);
            q += lp;
            q2 = q;
        }
        // p��hybPath�̃t�@�C���������̐擪�A�h���X
        const unsigned char* p2 = _mbsstr((const unsigned char*)p, (const unsigned char*)".hyb");
        if ((p2 <= p) || (p2 >= (const unsigned char*)hybPath + len)) {
            p2 = (const unsigned char*)hybPath + len;
        }
        // p2�͊g���q�𔲂����t�@�C���������̖����A�h���X

        memcpy(q, dll_prefix, sizeof(dll_prefix));
        q += sizeof(dll_prefix) - 1;
        size_t lp2 = p2 - p;
        memcpy(q, p, lp2);
        q += lp2;
        memcpy(q, dll_ext, sizeof(dll_ext));
        q += sizeof(dll_ext) - 1;
        *q = '\0';

#ifdef VERBOSE_LOAD_DLL
        HMD_PRINTF("try load dll: %s\n", dllPath);
#endif
		HMODULE hmod = LoadLibraryMB(dllPath);    // dll���[�h
#ifdef VERBOSE_LOAD_DLL
        if (hmod == NULL) printLastError();
#endif        
        if (hmod == NULL && q2 != dllPath) {
            // �p�X�����̃t�@�C���������Ń��[�h���Ă݂�
#ifdef VERBOSE_LOAD_DLL
            HMD_PRINTF("try load dll: %s\n", q2);
#endif        
            hmod = LoadLibraryMB(q2);
#ifdef VERBOSE_LOAD_DLL
            if (hmod == NULL) printLastError();
#endif        
        }

        if (hmod != NULL) {
            dllHandleArr.add(hmod);
#ifdef VERBOSE_LOAD_DLL
            HMD_PRINTF("DLL '%s' loaded.\n", q2);
#endif
        }
    }



    void init_memory(void)
    {
        // �f�o�b�O�o�͂́AOutputDebugString ���g��
        //hmd_set_debug_out_mode(HMD_DEBUG_OUT_ODS);
        // �f�o�b�O�o�͂́A�W���o�͂��g��
        hmd_set_debug_out_mode(HMD_DEBUG_OUT_STD);

        // ���C��������������
        mainMem = HMD_ALLOC(mainMemSize);
        HMD_ASSERT(mainMem != NULL);
        initMemory(mainMem, mainMemSize);

        // �f�o�b�O������������
        if (bReadDebugInfos) {
            if (debugMemSize == 0)
                debugMemSize = 4 * 1024 * 1024;
        }
        if (debugMemSize > 0) {
            debugMem = HMD_ALLOC(debugMemSize);
            if (debugMem == NULL)
                debugMemSize = 0;
            else
                initializeDebug(debugMem, debugMemSize);
        }
    }


    void init_main(void)
    {
        jmp_buf hayatJmpBuf;
        if (setjmp(hayatJmpBuf) == 0) {
            hmd_halt_jmpbuf = & hayatJmpBuf;

            if (bReadDebugInfos) {
                // �f�o�b�O���ǂݍ���
                gSymbolTable.readFile("symbols.sym");
                Bytecode::setFlagReadDebugInfo(true);
                MMes::initialize();
                MMes::readTable("mm_sjis.mm");
            }

            // DLL�ǂݍ��ݗp�R�[���o�b�N�o�^
            gCodeManager.setBytecodeLoadedCallback(cb4dll);

            // �W�����C�u�������[�h&������
            initStdlib();
        } else {
#ifdef HMD_ALLOW_CPP_EXCEPTION
            throw "initialization failed";
#else
            HMD_FATAL_ERROR(M_M("initialization failed"));
#endif
        }
    }

    
    Bytecode* load_bytecode(const char* hybFilename)
    {
        Bytecode* pBytecode = gCodeManager.readBytecode(hybFilename);
        if (pBytecode == NULL) {
            HMD_FATAL_ERROR(M_M("cannot load %s"), hybFilename);
        }
		return pBytecode;
	}

    bool reload_bytecode(const char* hybFilename, bool bInit)
    {
        Bytecode* pBytecode = gCodeManager.reloadBytecode(hybFilename, SYMBOL_ID_ERROR, bInit);
        if (pBytecode == NULL)
            return false;
        return true;
	}

    void start_bytecode(Bytecode* pBytecode)
    {
        jmp_buf hayatJmpBuf;
        if (setjmp(hayatJmpBuf) == 0) {
            hmd_halt_jmpbuf = & hayatJmpBuf;

            // ���s�J�n
            /*ThreadID_t tid =*/ startThread(pBytecode);

            // �ŏ���1tick�����s
            gThreadManager.exec1tick();

        } else {
#ifdef HMD_ALLOW_CPP_EXCEPTION
            throw "error on hayat script initialization";
#else
            HMD_FATAL_ERROR(M_M("error on hayat script initialization"));
#endif
        }
    }

    bool start_main(const char* hybFilename)
    {
        Bytecode* pBytecode = load_bytecode(hybFilename);
        if (pBytecode != NULL) {
            start_bytecode(pBytecode);
            return true;
        }
        return false;
    }

    bool tick_main(void)
    {
        if (! gThreadManager.isThreadRunning()) {
            return false;
        }

        jmp_buf hayatJmpBuf;
        if (setjmp(hayatJmpBuf) == 0) {

            hmd_halt_jmpbuf = & hayatJmpBuf;
        
            gThreadManager.exec1tick();

            hyu32 n = updateFuncArr.size();
            for (hyu32 i = 0; i < n; ++i)
                updateFuncArr[i]();

            n = drawFuncArr.size();
            for (hyu32 i = 0; i < n; ++i)
                drawFuncArr[i]();

            GC::incremental();

        } else {
#ifdef HMD_ALLOW_CPP_EXCEPTION
            throw "error on hayat script execution";
#else
            HMD_FATAL_ERROR(M_M("error on hayat script execution"));
#endif
        }

        return true;
    }

    void term_main(void)
    {
        hyu32 n = termFuncArr.size();
        for (hyu32 i = 0; i < n; ++i)
            termFuncArr[i]();

        finalizeAll();
        finalizeDebug();

        loadPathArr.finalize();
        updateFuncArr.finalize();
        drawFuncArr.finalize();
        termFuncArr.finalize();
        for (hyu32 i = dllHandleArr.size(); i > 0; ) {
            AfxFreeLibrary(dllHandleArr[--i]);
        }
        dllHandleArr.finalize();

        HMD_FREE(mainMem);
        HMD_FREE(debugMem);
    }

}
