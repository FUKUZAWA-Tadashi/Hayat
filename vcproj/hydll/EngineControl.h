/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#pragma once

#include "machdep.h"
#include "tchar.h"

namespace Hayat {
    namespace Engine {
        class Bytecode;
    }
}

namespace hydll {

    // �o�^�֐��^
    typedef void (*RFunc_t)(void);

    // foo.hy�p��DLL�̃t�@�C������ hyr_foo.dll �ł���Ƃ��Č�������B
    // DLL�t�@�C�����̑O�ɒu��������
    static const char dll_prefix[] = "hyr_";
    // DLL�t�@�C�����̊g���q
    static const char dll_ext[] = ".dll";

    // ���s�t���[���J�E���g
    extern hyu32    g_nFrame;

    // Hayat�Ŏg�p���郁�����T�C�Y���w�肷��
    // �w�肵�Ȃ���΁A4MiB�̃��������m�ۂ���
    // init_memory()�̑O�ɌĂ΂Ȃ���΂Ȃ�Ȃ�
    extern void    setMemSize(size_t memSize);

    // �f�o�b�O�p�������[�h���邩�ǂ�����ݒ�
    // false���ƁAsetDebugMemSize()���Ă�ł��f�o�b�O�p�������͊m�ۂ��Ȃ��B
    // true���ƁA�V���{�����A�f�o�b�O���ASJIS�G���[���b�Z�[�W ��ǂށB
    //     setDebugMemSize()���Ă�ł��Ȃ��ꍇ�̓��C���̃������ɓǂށB
    // init_memory()�̑O�ɌĂ΂Ȃ���΂Ȃ�Ȃ�
    extern void    setFlagReadDebugInfos(bool flag);

    // Hayat�Ŏg�p����f�o�b�O�p�������T�C�Y���w�肷��
    // �w�肵�Ȃ���΁A�f�o�b�O�p�������͊m�ۂ��Ȃ�
    // init_memory()�̑O�ɌĂ΂Ȃ���΂Ȃ�Ȃ�
    extern void    setDebugMemSize(size_t MemSize);

    // ������������
    extern void    init_memory(void);

    // ���[�h�p�X��ǉ�����
    extern void    addLoadPath(const char* path);

    // ��{������
    extern void    init_main(void);

    // �o�C�g�R�[�h��ǂݍ���
    Hayat::Engine::Bytecode*   load_bytecode(const char* hybFilename);

    // �o�C�g�R�[�h�������[�h
    extern bool    reload_bytecode(const char* hybFilename, bool bInit);

    // �w��o�C�g�R�[�h�̓�����V�K�X���b�h�Ŏ��s
    void    start_bytecode(Hayat::Engine::Bytecode* pBytecode);

    // �w��o�C�g�R�[�h�t�@�C����ǂݍ���œ�����V�K�X���b�h�Ŏ��s
    extern bool    start_main(const char* hybFilename);


    // �X�N���v�g�� 1tick ���s
    extern bool    tick_main(void);

    // �I������
    extern void    term_main(void);




    // 1�t���[����1��Ă΂��A�b�v�f�[�g�֐���o�^����
    extern void    addUpdateFunc(RFunc_t func);
    // �A�b�v�f�[�g�֐���擪�ɓo�^����
    extern void    insertUpdateFunc(RFunc_t func);

    // 1�t���[����1��Ă΂��`��֐���o�^����
    extern void    addDrawFunc(RFunc_t func);

    // �I���������֐���o�^����
    extern void    addTermFunc(RFunc_t func);

}


