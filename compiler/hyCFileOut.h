/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYFILEOUT_H_
#define m_HYFILEOUT_H_

#include <stdio.h>

namespace Hayat {
    namespace Compiler {

        void writeGenHead(FILE* fp);

        // �t�@�C���ɏ��������e���A���t�@�C���Ɠ������e�������ꍇ�ɂ�
        // �X�V�����Ƀ^�C���X�^���v��ύX���Ȃ����[�h�t���̃t�@�C�����C�^�B
        class DiffWrite {
        public:
            typedef enum {
                DW_DEFAULT,     // �N���X�̃f�t�H���g�ݒ���g��
                DW_OVERWRITE,   // ���e���r�����ɕK���㏑���X�V����
                DW_DIFFWRITE    // ���e�������Ȃ�X�V���Ȃ�
            } DwMode_e;

            // �N���X�̃f�t�H���g�ݒ���Z�b�g
            static void setDefaultMode(DwMode_e);

        protected:
            static DwMode_e m_defaultMode;

        public:
            DiffWrite(void);
            DiffWrite(const char* path, DwMode_e mode = DW_DEFAULT);
            ~DiffWrite();

            FILE* open(const char* path, DwMode_e mode = DW_DEFAULT);
            void close(void);
            FILE* fp(void) { return m_fp; }

        protected:
            FILE*       m_fp;
            DwMode_e    m_mode;
            const char* m_orgPath;
            char*       m_tmpPath;
        };
    }
}

#endif /* m_HYFILEOUT_H_ */
