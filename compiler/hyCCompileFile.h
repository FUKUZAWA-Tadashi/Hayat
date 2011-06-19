/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_COMPILEFILE_H_
#define m_COMPILEFILE_H_


namespace Hayat {
    namespace Compiler {


        struct CompileOption {
            // �p�[�X�r����ԃ��b�Z�[�W���o�����x�� 0�`2
            int         printIntermediateLevel;

            // �W�����v�惉�x�����݃`�F�b�N���x��
            // 0 = �`�F�b�N���Ȃ�
            // 1 = �t�@�C�����Ƀ��x�����������Warning
            // 2 = �u���b�N���Ƀ��x�����������Warning
            int         Wlabel;
    
            // sayCommand�ʂ��ԍ�����������ȏ�ɂȂ��Warning
            int         Wnumsay;

            // Wlabel��Warning�ł͂Ȃ�Error�Ƃ���
            bool        WlabelIsError;

            // Wnumsay��Warning�ł͂Ȃ�Error�Ƃ���
            bool        WnumsayIsError;

            // ���݂��s���ȃN���X�⃁�\�b�h���������ꍇ�̓�����w�肷��B
            // 0 = �`�F�b�N���Ȃ�
            // 1 = WARNING���o��
            // 2 = ERROR���o���Ē�~
            int         Wunknown;

            // class�̊O�̃X�R�[�v�̒萔���Ɠ����萔�����ǂ�������
            // 0 = �G���[�ɂȂ炸�g�p�\
            // 1 = WARNING���o�����g�p�\
            // 2 = ERROR���o���Ē�~
            int         Wconstscope;

            // �p�[�X�������ɍ\���؂��o��
            bool        bPrintSyntaxTree;

            // �p�[�X���Ƀ������@�\���g�����ǂ���
            // ���������g���ƍ�����������������ʂɕK�v�A�g��Ȃ���Βᑬ�Ⴡ����
            bool        bUseMemoize;

            // �璷���b�Z�[�W�t���O
            bool        bVerbose;

            // �|�󃁃b�Z�[�W�t�@�C����
            const char* mmesFileName;
        };


        extern CompileOption compileOption;

        int compileFile(const char* hyFileName);

    }
}

#endif /* m_COMPILEFILE_H_ */
