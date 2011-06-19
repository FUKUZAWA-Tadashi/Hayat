/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


/*
�N���X���\�b�h�̏ꍇ�́ASTATIC_ ���O�ɂ������̂��g�p�B


  �֐���        �}�N��
  -------------------------------
  func          FFI_FUNC(func)
  func?         FFI_FUNC_Q(func)
  func!         FFI_FUNC_E(func)
  func=	        FFI_FUNC_S(func)
  ��ascii�Ȃ�   FFI_FUNC_X(16�i)

�֐������L���̎��́A�������A�X�L�[�R�[�h16�i��2���ɕύX�������т��g�p����B
�Ⴆ�΁A"+@" �Ȃ�� 2b40 �A"<=>" �Ȃ�� 3c3d3e �ƂȂ�B
16�i�����̃A���t�@�x�b�g�͏������B

 */


#ifndef m_HYFFIDECL_H_
#define m_HYFFIDECL_H_

#include "hayat.h"

// m_CLASS__FIELD_SIZE_ ��  HSca_*.h �t�@�C�����Œ�`�����

#define FFI_FUNC(func)   HSfa_ ## func
#define FFI_FUNC_Q(func) HSfq_ ## func
#define FFI_FUNC_E(func) HSfe_ ## func
#define FFI_FUNC_S(func) HSfs_ ## func
#define FFI_FUNC_X(func) HSfx_ ## func

#define DEEP_FFI_FUNC(func)   void m_HSfa_ ## func(Context* context, int numArgs)
#define DEEP_FFI_FUNC_Q(func) void m_HSfq_ ## func(Context* context, int numArgs)
#define DEEP_FFI_FUNC_E(func) void m_HSfe_ ## func(Context* context, int numArgs)
#define DEEP_FFI_FUNC_S(func) void m_HSfs_ ## func(Context* context, int numArgs)
#define DEEP_FFI_FUNC_X(func) void m_HSfx_ ## func(Context* context, int numArgs)

#define FFI_GCMARK_FUNC() void HSfg_GCMarkFunc(Object* object)


// �ȉ���ASSERT��Class�N���X�ȊO�Ŏg�p�\ //
#define FFI_DEBUG_ASSERT_CLASSMETHOD(selfVal) HMD_DEBUG_ASSERTMSG(selfVal.type->symCheck(HSym_Class), M_M("instance method call for a class method"))
#define FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal) HMD_DEBUG_ASSERTMSG(! selfVal.type->symCheck(HSym_Class), M_M("class method call for instance method"))
#define FFI_ASSERT_CLASSMETHOD(selfVal) HMD_ASSERTMSG(selfVal.type->symCheck(HSym_Class), M_M("instance method call for a class method"))
#define FFI_ASSERT_INSTANCEMETHOD(selfVal) HMD_ASSERTMSG(! selfVal.type->symCheck(HSym_Class), M_M("class method call for instance method"))



// �����o�ϐ��ւ̃A�N�Z�X���\�b�h��`
// getter
#define FFI_MEMBER_GETTER(MEMB,MEMBTYPE,CPPCLASS,CPPMEMB)      \
    MEMBTYPE FFI_FUNC(MEMB) (Value selfVal)                    \
    {                                                              \
        FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);              \
        CPPCLASS * cpp_ins_p = selfVal.toCppObj<CPPCLASS>();       \
        return (MEMBTYPE) (cpp_ins_p->CPPMEMB);                    \
    }

//setter
#define FFI_MEMBER_SETTER(MEMB,MEMBTYPE,CPPCLASS,CPPTYPE,CPPMEMB)  \
    MEMBTYPE FFI_FUNC_S(MEMB) (Value selfVal, MEMBTYPE right_val)  \
    {                                                                  \
        FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal);                  \
        CPPCLASS * cpp_ins_p = selfVal.toCppObj<CPPCLASS>();           \
        cpp_ins_p->CPPMEMB = (CPPTYPE)right_val;                       \
        return right_val;                                              \
    }


// �A�N�Z�X���\�b�h����c++�����o�ϐ����������ꍇ�̃A�N�Z�X���\�b�h��`
// getter
#define FFI_S_MEMBER_GETTER(MEMB,MEMBTYPE,CPPCLASS)  FFI_MEMBER_GETTER(MEMB,MEMBTYPE,CPPCLASS,MEMB)
// setter
#define FFI_S_MEMBER_SETTER(MEMB,MEMBTYPE,CPPCLASS,CPPTYPE) FFI_MEMBER_SETTER(MEMB,MEMBTYPE,CPPCLASS,CPPTYPE,MEMB)



using namespace Hayat::Common;
using namespace Hayat::Engine;

#endif /* m_HYFFIDECL_H_ */
