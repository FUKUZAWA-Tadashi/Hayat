/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCIDENT_H_
#define m_HYCIDENT_H_

#include "machdep.h"

// Hayat�Ŏg�����ʎq���AC++�Ŏg���鎯�ʎq�ɕϊ�����
//
// name �� [a-zA-Z0-9_] �ȊO�̕������܂܂�Ă��Ȃ��ꍇ
//   �� a_name
//
// name �� [a-bzA-Z0-9_] �ȊO�̕������܂܂�Ă���ꍇ
//   ������ ? �ŁA���� [a-bzA-Z0-9_]
//   �� q_name
//
//   ������ ! �ŁA���� [a-bzA-Z0-9_]
//   �� e_name
//
//   ������ = �ŁA���� [a-bzA-Z0-9_]
//   �� s_name
//
//   ������ .hyb �ŁA���� [a-bzA-Z0-9_]
//   �� b_name
//
//   ����ȊO
//   �� x_16�i
//

namespace Hayat {
    namespace Compiler {

        class Ident {
        public:
            // name���G���R�[�h����buf�ɓ����B���肫��Ȃ�������false��Ԃ�
            static bool encode(const char* name, char* buf, hyu32 bufSize);

            // name���G���R�[�h���ĕԂ��B�擪�� head ������B
            // �������ăG���R�[�h���s���ɂ�NULL��Ԃ��B
            // �߂�l�� gMemPool ����alloc�����o�b�t�@�ɓ���̂ŁA
            // �s�v�ƂȂ�����free���Ȃ���΂Ȃ�Ȃ�
            static char* encode(const char* name, const char* head = "", hyu32 extraMem = 0);

        };

    }
}

#endif /* m_HYCIDENT_H_ */
