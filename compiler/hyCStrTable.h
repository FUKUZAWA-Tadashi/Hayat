/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCSTRTABLE_H_
#define m_HYCSTRTABLE_H_

#include "machdep.h"
#include <stdio.h>
#include "hpSubstr.h"
#include "hyArray.h"

using namespace Hayat::Common;
using namespace Hayat::Parser;

class Test_hyCStrTable;

namespace Hayat {
    namespace Compiler {


        // ������e�[�u�� or �o�C�g��e�[�u��
        //
        // �d���`�F�b�N�����A��������i�[����B
        // �܂��̓o�C�g����i�[����B
        // ������ƃo�C�g��͓����ł��Ȃ��B
        // ������̏I�[�� '\0' �Ń^�[�~�l�[�g����Ă���B
        // �o�C�g��͒��g�� '\0' ���܂܂�Ă��Ă��ǂ��A�I�[�̓T�C�Y�Ŏ����B
        class StrTable {
            friend class ::Test_hyCStrTable;    // unittest 

        protected:
            static const hyu32 m_INIT_STR_OFFS_SIZE = 256;
            static const hyu32 m_INIT_STR_BUF_SIZE = 5120;

        public:
            StrTable(void);
            ~StrTable();

            static const hyu32 NOT_FOUND = (hyu32)-1;
            static const hyu16 NOT_FOUND_ID = (hyu16)-1;


            void        initialize(hyu32 initStrOffsSize = m_INIT_STR_OFFS_SIZE, hyu32 initStrBufSize = m_INIT_STR_BUF_SIZE);
            void        finalize(void);

            // �d���`�F�b�N����(�����K�v�Ȃ當������i�[��)���̃I�t�Z�b�g��Ԃ�
            hyu32         getOffs(const char* str, hyu32 len = 0);

            // �d���`�F�b�N�����ɕ�������i�[���A���̃I�t�Z�b�g��Ԃ�
            hyu32         addStr(const char* str, hyu32 len = 0);

            // str�Ɠ��������񂪊��Ɋi�[����Ă�����A���̃I�t�Z�b�g��Ԃ�
            // �i�[����Ă��Ȃ�������ANOT_FOUND ��Ԃ�
            hyu32         checkOffs(const char* str, hyu32 len = 0);

            // �I�t�Z�b�g�ɑΉ����镶�����Ԃ�
            const char* getStr(hyu32 offset);
            

            // �d���`�F�b�N����(�����K�v�Ȃ�o�C�g����i�[��)���̔ԍ���Ԃ�
            hyu16         getIdBytes(const hyu8* bp, hyu32 len);
            // �d���`�F�b�N�����Ƀo�C�g����i�[���A���̔ԍ���Ԃ�
            hyu16         addBytes(const hyu8* bp, hyu32 len);
            // bp�Ɠ����o�C�g�񂪊��Ɋi�[����Ă�����A���̔ԍ���Ԃ�
            // �i�[����Ă��Ȃ�������ANOT_FOUND_ID ��Ԃ�
            hyu16         checkIdBytes(const hyu8* bp, hyu32 len);
            // �ԍ��ɑΉ�����o�C�g���Ԃ�
            const hyu8*   getBytes(hyu16 id, hyu32* pLen = NULL);

            // ������A�o�C�g��̌���Ԃ�
            hyu32         numStr(void) { return m_numStr; }


            // FILE* ���當����e�[�u����ǂݍ���
            void        readFile(FILE* fp);
            // ������e�[�u���� FILE* �ɏo��
            // UTF16�̕�����e�[�u�����o�͂����ꍇ�AreadFile�ł͓ǂ߂Ȃ�
            void        writeFile(FILE* fp);

            // �o�C�g��e�[�u�����o��
            void        writeBytes(TArray<hyu8>* out);
            // �o�C�g��e�[�u����ǂݍ���
            void        readBytes(const hyu8** pinp);

#ifdef HMD_DEBUG
            void        dumpByteTable(void);
#endif

        protected:

            hyu32         m_numStr;
            hyu32*        m_strOffs;
            hyu32         m_strOffsSize;
            char*       m_strBuf;
            hyu32         m_strBufSize;

            //  �������ǉ����鏀��: m_strBuf�̃T�C�Y���s������Ȃ�realloc
            void        m_reserve(hyu32 len);
            // m_strOffs��1�ǉ����鏀��: m_strOffs�̃T�C�Y���s������Ȃ�realloc
            void        m_expandStrOffs(void);

        };

    }
}
#endif /* m_HYCSTRTABLE_H_ */
