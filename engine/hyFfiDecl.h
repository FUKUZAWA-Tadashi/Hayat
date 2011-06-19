/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */


/*
クラスメソッドの場合は、STATIC_ が前についたものを使用。


  関数名        マクロ
  -------------------------------
  func          FFI_FUNC(func)
  func?         FFI_FUNC_Q(func)
  func!         FFI_FUNC_E(func)
  func=	        FFI_FUNC_S(func)
  非asciiなど   FFI_FUNC_X(16進)

関数名が記号の時は、文字をアスキーコード16進数2桁に変更した並びを使用する。
例えば、"+@" ならば 2b40 、"<=>" ならば 3c3d3e となる。
16進数中のアルファベットは小文字。

 */


#ifndef m_HYFFIDECL_H_
#define m_HYFFIDECL_H_

#include "hayat.h"

// m_CLASS__FIELD_SIZE_ は  HSca_*.h ファイル内で定義される

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


// 以下のASSERTはClassクラス以外で使用可能 //
#define FFI_DEBUG_ASSERT_CLASSMETHOD(selfVal) HMD_DEBUG_ASSERTMSG(selfVal.type->symCheck(HSym_Class), M_M("instance method call for a class method"))
#define FFI_DEBUG_ASSERT_INSTANCEMETHOD(selfVal) HMD_DEBUG_ASSERTMSG(! selfVal.type->symCheck(HSym_Class), M_M("class method call for instance method"))
#define FFI_ASSERT_CLASSMETHOD(selfVal) HMD_ASSERTMSG(selfVal.type->symCheck(HSym_Class), M_M("instance method call for a class method"))
#define FFI_ASSERT_INSTANCEMETHOD(selfVal) HMD_ASSERTMSG(! selfVal.type->symCheck(HSym_Class), M_M("class method call for instance method"))



// メンバ変数へのアクセスメソッド定義
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


// アクセスメソッド名とc++メンバ変数名が同じ場合のアクセスメソッド定義
// getter
#define FFI_S_MEMBER_GETTER(MEMB,MEMBTYPE,CPPCLASS)  FFI_MEMBER_GETTER(MEMB,MEMBTYPE,CPPCLASS,MEMB)
// setter
#define FFI_S_MEMBER_SETTER(MEMB,MEMBTYPE,CPPCLASS,CPPTYPE) FFI_MEMBER_SETTER(MEMB,MEMBTYPE,CPPCLASS,CPPTYPE,MEMB)



using namespace Hayat::Common;
using namespace Hayat::Engine;

#endif /* m_HYFFIDECL_H_ */
