/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


#include "hayat.h"
#include "hyException.h"
#include "hyGC.h"
#include <setjmp.h>

using namespace Hayat::Common;
using namespace Hayat::Engine;


#define HAYAT_MEM_SIZE  (1024 * 1024)



int main(int argc, char* argv[])
{
    void* hayatMem = HMD_ALLOC(HAYAT_MEM_SIZE);
    int retval = 1;

    jmp_buf jmpBuf;
    if (setjmp(jmpBuf) == 0) {
        hmd_halt_jmpbuf = &jmpBuf;

        // ������
        initMemory(hayatMem, HAYAT_MEM_SIZE);
        initializeDebug();      // �f�o�b�O�p��������hayatMem�Ƌ��p

        // �f�o�b�O���ǂݍ���
        gSymbolTable.readFile("symbols.sym");
        Bytecode::setFlagReadDebugInfo(true);
        MMes::initialize();
        //MMes::readTable("mm_sjis.mm");

        // �W�����C�u�������[�h
        initStdlib();

        // �e�X�g�X�C�[�g�o�C�g�R�[�h ���[�h&������
        extern void hySuiteFfiBinder(void); // defined in testConf.cpp
        hySuiteFfiBinder();
        Bytecode* suiteBytecode = gCodeManager.readBytecode("hy_suite.hyb");
        if (suiteBytecode == NULL)
            HMD_FATAL_ERROR("cannot load hy_suite.hyb");

        // ���s
        /*ThreadID_t tid =*/ startThread(suiteBytecode);

        while (gThreadManager.isThreadRunning())
            gThreadManager.exec1tick();

        retval = 0;

    } else {
        HMD_PRINTF("hayat halt\n");
    }


    // ��n��
    finalizeAll();
    finalizeDebug();

    HMD_FREE(hayatMem);

    return retval;
}
