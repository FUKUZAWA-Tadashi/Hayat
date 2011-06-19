/*  -*- coding: sjis-dos; -*-  */

#ifndef m_HYSTATICASSERT_H_
#define m_HYSTATICASSERT_H_

// STATIC_ASSERT(íËêîèåèéÆ, àÍà”Ç»éØï éq)
//
// égópó·:
// STATIC_ASSERT(sizeof(x)/sizeof(x[0]) < 10, size_must_less_than_10);


namespace Hayat {
    namespace Common {
        template <bool> struct STATIC_ASSERT_ST;
        template <> struct STATIC_ASSERT_ST<true> { enum { x__dummy__x = 1 }; };
        template <int x> struct STATIC_ASSERT_TEST {};
#define STATIC_ASSERT(x,y) typedef STATIC_ASSERT_TEST< sizeof( STATIC_ASSERT_ST<x> ) >  x_x_ ## y
    }
}

#endif /* m_HYSTATICASSERT_H_ */
