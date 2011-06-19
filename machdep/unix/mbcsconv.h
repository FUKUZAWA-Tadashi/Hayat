/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_MBCSCONV_H_
#define m_MBCSCONV_H_

//
// ������̕����R�[�h�ϊ����[�`�� for unix
// iconv���g�p���ĕϊ�����
//


// in case of Japanese Shift-JIS, linux mbtowc() converts
// backslash '\' to 165, but we need 92 (L'\\') .
// define this if you want to change single byte 165 to 92.
#define AVOID_MBTOWC_SJIS_SANITIZE


#include <iconv.h>
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

        protected:
            static iconv_t  m_iconv_desc;
            static const char* m_prevLocale;
#ifdef AVOID_MBTOWC_SJIS_SANITIZE
            static bool m_sjis;
#endif
#ifdef __CYGWIN__
            // cygwin��mbtowc()���������l��Ԃ��Ȃ��̂ŁAiconv()���g�p����
            static iconv_t m_iconv_mbwc_desc;
            static size_t cygwin_mbtowc(wchar_t* pmc, const char* s, size_t n);
#endif
        };
        
    }
}


#endif /* m_MBCSCONV_H_ */
