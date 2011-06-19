/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HAYAT_H_
#define m_HAYAT_H_

#include "hyThreadManager.h"
#include "hyVM.h"
#include "hyBytecode.h"
#include "hyCodeManager.h"
#include "hyVarTable.h"
#include "hyStringBuffer.h"
#include "hyValueArray.h"
#include "hyException.h"
#include "hyHash.h"
#include "hyStringBuffer.h"
#include "hySymbolTable.h"
#include "hyDebug.h"

namespace Hayat {
    namespace Engine {

        // �������AGC��������
        void    initMemory(void* pMemory, size_t memorySize);

        // stdlib������
        // �o�C�g�R�[�h�͓����ŊǗ������̂ŁA�Ō��finalize�������ōs�Ȃ���
        void    initStdlib(const char* stdlibPath = "stdlib.hyb");

        // ���C�u�����ރo�C�g�R�[�h�ǂݍ��݁A������
        // �߂�l�̓o�C�g�R�[�h�V���{��
        // �o�C�g�R�[�h�͓����ŊǗ������̂ŁA�Ō��finalize�������ōs�Ȃ���
        SymbolID_t      readLibrary(const char* filename);

        // �o�C�g�R�[�h�̐擪����A�X���b�h�ŊJ�n
        ThreadID_t      startThread(Bytecode* bytecode);

        // �X���b�h�����I��
        void    terminateThread(ThreadID_t tid);

        // ��n��
        void    finalizeAll(void);

        // �f�o�b�O������������
        void    initializeDebug(void* debugMemory = NULL, size_t debugMemorySize = 0);
        // �f�o�b�O��n��
        void    finalizeDebug(void);
    }
}


#endif /* m_HAYAT_H_ */


/********** �g�p�� *******
   
    // �������m��
    void* pMem = OS_ALLOCATE_MEMORY(memSize); 
   
    // ������
    Hayat::Engine::initMemory(pMem, memSize);
   
    // ===={ �f�o�b�O�p�ݒ�  (���̕����͌Ă΂Ȃ��Ă��ǂ�)
    // �f�o�b�O���p�������m�� (���Ȃ���΃��C��������gMemPool�ɓǂ�)
    void* pDebMem = OS_ALLOCATE_MEMORY(debMemSize);
    initializeDebug(pDebMem, debMemSize);
    // ���o�C�g�R�[�h�̃f�o�b�O���(*.hdb)�ǂݍ��݂��I�t�ɂ���ꍇ�͈ȉ����g��
    // Hayat::Engine::Bytecode::setFlagReadDebugInfo(false);

    // �f�o�b�O�p�V���{��(symbols.sym)�ǂݍ��� (�����Ă��ǂ�)
    Hayat::Engine::gSymbolTable.readFile("symbols.sym");
    // �G���[���b�Z�[�W��Shift-JIS�o�[�W������ǂݍ��� (�����Ă��ǂ�)
    MMes::readTable("mm_sjis.mm");
    // ====} �f�o�b�O�p�ݒ�I���

   
    // stdlib������
    Hayat::Engine::initStdlib();
   
    // ���ꂼ��̃v���W�F�N�g�ŕK�v�ȃo�C�g�R�[�h�ǂݍ���
    // �匳��ǂ߂�require��͈𖠎��Ɏ����œǂݍ���
    Hayat::Engine::readLibrary("project_library_bytecode.hyb");
   
    // ���s�������o�C�g�R�[�h��ǂݍ���
    Hayat::Engine::Bytecode* pBytecode = Hayat::Engine::gCodeManager.readBytecode("target.hyb");
    // ������X���b�h�Ŏ��s�J�n
    Hayat::Engine::ThreadID_t tid;
    tid = Hayat::Engine::startThread(pBytecode);
   
    // 1�t���[������exec1tick()���Ă�
    // while (Hayat::Engine::gThreadManager.isThreadRunning())
    //     Hayat::Engine::gThreadManager.exec1tick();
   
    // ��n��
    Hayat::Engine::finalizeAll();
 
    // �f�o�b�O��n��  (�f�o�b�O�p�ݒ�������Ȃ�ΌĂ�)
    Hayat::Engine::finalizeDebug();

    // �f�o�b�O�p��������� (�m�ۂ����Ȃ��)
    OS_FREE_MEMORY(pDebMem);


    // ���������
    OS_FREE_MEMORY(pMem);

********** **********/
