/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_MBCSCONV_H_
#define m_MBCSCONV_H_

//
// �����R�[�h�ϊ����[�`�� for Visual C++
//


#include "machdep.h"


namespace Hayat {
    namespace Parser {

        class MBCSConv {
        public:
            static bool initialize(const char* fromcode, const char* tocode = NULL);
            static void finalize(void);

            // *pInStr�ɂ���}���`�o�C�g������1�����ǂށB�ǂ񂾃o�C�g����
            // *pLen��(NULL�łȂ����)�i�[����B *pInStr ��ǂ񂾃o�C�g����
            // �i�܂���B�ǂ߂Ȃ������ꍇ�� *pLen �� 0 ���i�[����B
            static wchar_t getCharAt(const char** pInStr, hyu32* pLen = NULL);

            // inStr�̃}���`�o�C�g������(�����R�[�hfromcode)��outBuf��
            // �����R�[�htocode�ɕϊ����Ċi�[����B�ϊ���̕����񒷂�Ԃ��B
            // �G���[�̏ꍇ��(hyu32)-1��Ԃ��B
            static hyu32 conv(const char* inStr, hyu32 inStrLen, char* outBuf, hyu32 outBufSize);

            // inStr�̃G�X�P�[�v���� \n \r \x?? \? ��W�J�����������outBuf��
            // �i�[����B���������R�[�h��fromcode�B
            // bEolCare��true�Ȃ疖����CR�R�[�h�͍폜���A������LF�R�[�h��t������B
            // �ŏI�I�ɖ����� \0 ��t������B(�߂�l�ł�\0���J�E���g���Ȃ�)
            // �߂�l: �W�J�㕶���񒷁B -1=�ǂ߂Ȃ������A-2=outBuf�I�[�o�[�t���[
            static hys32 unescape(const char* inStr, hyu32 inStrLen, char* outBuf, hyu32 outBufSize, bool bEolCare = false);

            // fromcode��UTF8���ǂ���
            static bool is_utf8_in(void);

        protected:
            static hyu16  m_inCodePage;
            static hyu16  m_outCodePage;
        };
        
    }
}



#endif /* m_MBCSCONV_H_ */
