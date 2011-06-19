/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYSTRINGBUFFER_H_
#define m_HYSTRINGBUFFER_H_

#include "hyValue.h"
#include "hyObject.h"
#include <stdarg.h>

/*
  ������o�b�t�@�N���X

  ������̏I�[�ɂ͕K�� '\0' ������B
  �o�b�t�@�T�C�Y�� '\0' ���܂ރT�C�Y�B

  this
  +--------------+
  | hyu16 m_bufSize
  +--------------+
  | hyu16 m_length
  +--------------+
  | char* m_buffer
  +--------------+

  m_buffer
  +------------+
  |            |
  |  buffer    |
  |            |
  |          \0|
  +------------+
 */

namespace Hayat {
    namespace Engine {
        class Thread;

        // �^��StringBuffer�ł���Object�𑀍삷��N���X
        class StringBuffer {
        protected:
            hyu16     m_bufSize;
            hyu16     m_length;
            char*   m_buffer;


        public:
            static const hyu16 DEFAULT_BUFSIZE = 64;

            static void* operator new(size_t);
            static void operator delete(void*);

            StringBuffer(hyu16 bufSize = DEFAULT_BUFSIZE) { initialize(bufSize); }
            ~StringBuffer() { finalize(); }

            void    initialize(hyu16 bufSize = DEFAULT_BUFSIZE);
            void    finalize(void);


            /// �Ή�Object��Ԃ�
            Object* getObj(void) { return Object::fromCppObj(this); }

            /// �o�b�t�@���N���A
            void    clear(void);
            /// ������擪�A�h���X
            char*   top(void) { return m_buffer; }
            /// �o�b�t�@�T�C�Y ('\0'���܂ރT�C�Y)
            hyu16     bufSize(void) { return m_bufSize; }
            /// ������
            hyu16     length(void) { return m_length; }

            /// �o�b�t�@�g�������Œǉ��\�ȕ�����
            hyu16     remain(void) { return m_bufSize - m_length - 1; }
            /// len�o�C�g�̕������ǉ��ł���悤�ɁA�K�v�Ȃ�o�b�t�@���g��
            void    reserve(hyu16 len);
            /// �������ǉ��F�o�b�t�@���s�����Ă�����g������
            void    concat(const char* str, int len = -1);
            /// �l��concatToStringBuffer���\�b�h�ŕ����񉻂��āA�����ɒǉ�����B
            void    concat(Context* context, Value d, int mode = 1);
            /// �o�C�i���f�[�^��ǉ��F���� \0 ���܂܂�Ă��Ă��ǂ�
            /// �o�b�t�@���s�����Ă�����g������
            void    addBinary(const void* bin, hyu32 size);
            /// printf�̌��ʂ𖖔��ɒǉ�����B
            void    sprintf(const char* fmt, ...);
            void    vsprintf(const char* fmt, va_list);

            /// ������I�[�A�h���X ('\0'�̈ʒu)
            char*   bottom(void) { return m_buffer + m_length; }
            /// ���ɕ������ǉ�������A������Ă�ŕ����񒷂��Čv�Z����
            /// �ǉ��O�̏I�[�ʒu����A \0 �̈ʒu���T�[�`����
            void    concatenated(void);
            /// �o�b�t�@�����삳��āA�����񒷂��Z�����Ȃ����\��������ꍇ�ɂ́A
            /// ������Ă�ŕ����񒷂��Čv�Z����
            /// �o�b�t�@�擪�ʒu����A \0 �̈ʒu���T�[�`����
            void    calcLength(void);
            /// �o�C�i���f�[�^�����̃A�h���X�܂œǂ܂ꂽ�F�I�[�ʒu�������ɃZ�b�g
            void    binaryAdded(const void* endAddr);

            /// d�𕶎��񉻂���func�ɓn���A�����񉻂����o�b�t�@�������ďI��
            /// d��nil��������󕶎����func�ɓn���B
            static void sendStr(Context* context, Value d, void(*func)(const char*));
            static void sendStr(Context* context, Value d, void(*func)(const char*,int));
            /// d1��d2�𕶎����r����B
            /// d1��d2�̌^��String��StringBuffer�̂ݎ󂯕t���Astrcmp�Ɠ����l��
            /// �Ԃ��B�����łȂ���΁A StringBuffer::COMPARE_ERROR ��Ԃ��B
            static hys32 compare(Value d1, Value d2);
            static const hys32 COMPARE_ERROR = -99999999;
            // �����񂩂�n�b�V���R�[�h���v�Z
            static hyu32 calcHashCode(const char*);
        };

    }
}
#endif /* m_HYSTRINGBUFFER_H_ */
