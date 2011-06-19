/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "HSca_Random.h"

static hyu32 seed = 0x23456789;

namespace Hayat {
    void genRand(void)
    {
        /* taps: 32 31 29 1; �A�ґ�����: x^32 + x^31 + x^29 + x + 1 */
        for (int i = 32; i > 0; --i)
            seed = (seed >> 1) ^ (-((hys32)(seed & 1)) & 0xd0000001u);
    }
}


FFI_DEFINITION_START {

// M�n�񗐐�: �A���S���Y���� �K���ALFSR
// http://ja.wikipedia.org/wiki/���`�A�҃V�t�g���W�X�^
// ������ 2^32-1 �Aseed�� 0 �ɂ͂Ȃ�Ȃ�

// rand(x), rand(n,m) �́A�P�ɏ�]������Ă��邾���Ȃ̂ŁA
// ���������l�̕��������ɕp�x�������B�����x��m-n��32bit�߂���
// �e�����傫���Ȃ�͂��B
// �܂��Aseed��0�ɂȂ�Ȃ��̂ŁA���̕������΂��Ă���ƌ����Ȃ����Ȃ��B
// �ʏ�Q�[���Ŏg�p���鐔�l�͈̔͂ł���΁A�قږ��ƂȂ�Ȃ��͂��B


hys32 FFI_FUNC(srand) (Value selfVal, hys32 x)
{
    FFI_DEBUG_ASSERT_CLASSMETHOD(selfVal);
    HMD_ASSERTMSG(x != 0, M_M("srand(0) is not allowed")); // todo: exception�ɂ��ׂ�
    seed = (hyu32) x;
    return x;
}

hys32 FFI_FUNC(seed) (Value selfVal)
{
    FFI_DEBUG_ASSERT_CLASSMETHOD(selfVal);
    return (hys32)seed;
}

DEEP_FFI_FUNC(rand)
{
    Value selfVal = context->pop();
    FFI_DEBUG_ASSERT_CLASSMETHOD(selfVal);
    Hayat::genRand();
    if (numArgs == 0) {
        context->pushInt((hys32)seed);
        return;
    } else if (numArgs == 1) {
        hys32 x = context->popInt();
        HMD_ASSERT(x > 0);      // todo: exception �ɂ��ׂ�
        context->pushInt((hys32)(seed % (hyu32)x));
        return;
    } else if (numArgs == 2) {
        hys32 y = context->popInt();
        hys32 x = context->popInt();
        HMD_ASSERT(y >= x);      // todo: exception �ɂ��ׂ�
        hys32 d = y - x + 1;
        context->pushInt((hys32)(seed % (hyu32)d) + x);
        return;
    }
    HMD_FATAL_ERROR("Random::rand(...) bad arguments error: numArgs = %d\n", numArgs);
}


} FFI_DEFINITION_END
