/*  -*- coding: sjis-dos -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

// n	big endian unsigned 16bit
// N	big endian 32bit
// v	little endian unsigned 16bit
// V	little endian 32bit
// c 	signed char 8bit
// C	unsigned char 8bit
// s	signed short 16bit
// S	unsigned short 16bit
// i	signed int 32bit
// f	float 32bit
// a	�i���o�C�g��؂蕶����
// A	�o�C�g����؂蕶���� : �ő�65535�o�C�g�܂�
// x	pack:�i���o�C�g / unpack:1byte�ǂݔ�΂�





#ifndef h_HYPACK_H_
#define h_HYPACK_H_

namespace Hayat {
    namespace Engine {

        class ValueArray;
        class StringBuffer;

        class Pack {
        public:
            static void pack(const char* templ, ValueArray* va, StringBuffer* sb);
            static void unpack(const char* templ, ValueArray* va, StringBuffer* sb);
        };
        
    }
}

#endif /* h_HYPACK_H_ */
