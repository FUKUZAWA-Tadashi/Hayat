/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


#ifndef m_HYTUNING_H_
#define m_HYTUNING_H_


// �o�C�g�R�[�h�����[�h�@�\���g�p���邩
#define HY_ENABLE_BYTECODE_RELOAD

// Object�̈ړ��������邩�ǂ���
// �ړ���������ƁAGC�̃I�[�o�[�w�b�h�������Ȃ�A�X�s�[�h�ቺ�ɂȂ���
#define HY_ENABLE_RELOCATE_OBJECT

// �o�C�g�R�[�h�����[�h�@�\���L���Ȃ�AObject�̈ړ��������Ȃ���΂Ȃ�Ȃ�
// ���ȉ�4�s�͕ύX���Ă͂����Ȃ�
#ifdef HY_ENABLE_BYTECODE_RELOAD
#undef HY_ENABLE_RELOCATE_OBJECT
#define HY_ENABLE_RELOCATE_OBJECT
#endif


// Hash initial capacity 
#define HASH_DEFAULT_INIT_CAPACITY	31
// Hash load factor 75%
#define HASH_DEFAULT_LOAD_FACTOR	75


// �z��e�ʂ����ꖢ���̏ꍇ�́A���₷���Ɍ��e�ʂ�2�{�Ƃ���
#define ARRAY_CAPACITY_DOUBLE_LIMIT	    0x20
// �z��e�ʂ����ꖢ���̏ꍇ�́A���₷���Ɍ��e�ʂ�1.5�{�Ƃ���
#define ARRAY_CAPACITY_ONE_HALF_LIMIT	0x80
// �z��e�ʂ�����ȏ�̏ꍇ�̑����l
#define ARRAY_CAPACITY_INCREMENTS	    0x40


// finally ���l�X�e�B���O�ł���ő吔
#define MAX_FINALLY_NEST                16


// �R���p�C�����ɃV���{����W���o�͂֏o��
//#define EXPERIMENT_SYMBOL_DUMP
#undef EXPERIMENT_SYMBOL_DUMP


// �g�p�\�c�������v�Z�������ɂ��邩�ǂ���
#ifdef TEST__CPPUNIT
#define EXACT_FREEMEMSIZE
#else
#undef EXACT_FREEMEMSIZE
//#define EXACT_FREEMEMSIZE
#endif

// �o�C�g�R�[�h���s�������L�^���邩�ǂ���
// VM��halt�������Ɏ��s�������L�^����Ă���Ε\�������
#ifdef HMD_DEBUG
#define HY_ENABLE_BYTECODE_HISTORY
#else
//#undef HY_ENABLE_BYTECODE_HISTORY
#define HY_ENABLE_BYTECODE_HISTORY
#endif


// FFI�̎��������t�@�C�� *.h *.cpp �̃t�@�C�����𐶐����鎞�A
// �N���X���ƃC���i�[�N���X���̊Ԃɋ��ޕ�����
#define HY_GEN_FNAME_DELIM  "__"


#endif /* m_HYTUNING_H_ */
