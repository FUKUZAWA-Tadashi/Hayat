/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


// �G���W�����ł̗�O���o���Ɏg����V���{����`
//
// hyCCompiler.cpp �� initializeCompiler() �̒��ŁA�V���{���ɕϊ������B

#include "hyStaticAssert.h"
using namespace Hayat::Common;


namespace Hayat {
    namespace Compiler {
        extern const char* exceptionSymbols[] = {
            "fatal_error",      // �v���I�G���[
            "type_mismatch",    // �^�����҂��Ă������̂ƈႤ
            "fiber_return",     // Fiber��yield�����҂��Ă����̂�return�����s���ꂽ
            "fiber_error",      // Fiber�̎��s�Ɏ��s
            "method_not_found", // ���\�b�h��������Ȃ�����
            "array_index_out_of_bounds", // �z��Y�����͈͊O���w���Ă���
            "invalid_argument",          // �����ُ�
            "use_member_var_out_of_instance", // �C���X�^���X�łȂ��̂Ƀ����o�ϐ��ɃA�N�Z�X���悤�Ƃ���
            "division_by_zero",               // 0�Ŋ���Z
            "empty_list",                     // �󃊃X�g������
            "closure_return",                 // closure��return�悪����
            "signature_mismatch",             // �����V�O�l�`�����}�b�`���Ȃ�
            "type_error",                     // �����̌^������Ȃ�
            "cannot_access_member",           // �����o�ɃA�N�Z�X�ł��Ȃ�
            "cannot_access_constant",         // �萔�ɃA�N�Z�X�ł��Ȃ�
            "cannot_access_scoped_val",       // �X�R�[�v�t���̒萔�܂��̓N���X�ɃA�N�Z�X�ł��Ȃ�
            "file_reload_failed",             // �����[�h���s
            "bad_constructor",                // �R���X�g���N�^���������C���X�^���X��Ԃ��Ȃ�����
            "unknown_identifier",             // �s���Ȏ��ʎq
            "bad_ffi_call",                   // FFI�ďo�����������Ȃ�
            "ffi_failed",                     // FFI�̎��s���ɃG���[��������

            (const char*)0      // end mark
        };

        STATIC_ASSERT(sizeof(exceptionSymbols)/sizeof(exceptionSymbols[0]) < 100, exceptionSymbols_size_limited_in_initializeCompiler);
    }
}
