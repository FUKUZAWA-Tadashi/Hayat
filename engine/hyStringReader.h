/*  -*- coding: sjis-dos -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef h_HYSTRINGREADER_H_
#define h_HYSTRINGREADER_H_

#include "hyObject.h"


namespace Hayat {
    namespace Engine {
        
        class StringBuffer;

        class StringReader {

        protected:
            // m_sbObj �������� m_str �̂ǂ��炩���g�p
            Object* m_sbObj;  // StringBuffer��Object
            const char* m_str;  // String
            hys32 m_strlen; // m_str�̕�����

            hys32 m_pos;    // �ǂݍ��݈ʒu

            const char* m_addr(hys32 pos);

        public:
            StringReader(void);
            void initialize(Object* sbObj);
            void initialize(const char* str);

            // ���݈ʒu��Ԃ��B
            hys32   getPos(void) { return m_pos; }
            // ���݈ʒu��ύX����Bpos���L���Ȃ�pos��Ԃ��B�����Ȃ�-1
            hys32   setPos(hys32 pos);
            // �S�̃T�C�Y��Ԃ��B�I�[�ʒu�Ɠ����B
            hys32   length(void);
            // 1�o�C�g�擾�B�����񂪂����������-1��Ԃ��B
            hys32   getc(void);
            // delim�Ɋ܂܂�镶�����I�[�Ƃ��镶������擾�B
            // �����񂪂����������NULL��Ԃ��B
            StringBuffer*   gets(const char* delim = "\n");
            // searchChars�Ɋ܂܂�镶���𔭌�������A������ pos ���ړ�����
            // true ��Ԃ��B�����ł��������񂪏I�������� false ��Ԃ��B
            bool    search(const char* searchChars);
            // skipChars�Ɋ܂܂�镶���ł���ԁApos��i�߂�B
            hys32   skip(const char* skipChars);
            // ch�ł���ԁApos��i�߂�B
            hys32   skip(hys32 ch) { return skip(ch, ch); }
            // c1 <= c <= c2 �ł���ԁApos��i�߂�B
            hys32   skip(hys32 c1, hys32 c2);
            // start <= p < end �͈̔͂̕�������擾
            StringBuffer*   substr(hys32 start, hys32 end);
            // ���݈ʒu����len�o�C�g���̕�������擾
            StringBuffer*   read(hys32 len);

            // ������ǂށB�ǂ߂Ȃ�������pos�͈ړ����Ȃ�
            hys32   getInt(void);
            // ����������ǂށB�ǂ߂Ȃ�������pos�͈ړ����Ȃ�
            hyf32   getFloat(void);

            void m_GC_mark(void);
        };

    }
}

#endif /* h_HYSTRINGREADER_H_ */
