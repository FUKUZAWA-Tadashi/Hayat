/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYENDIAN_H_
#define m_HYENDIAN_H_

#include "machdep.h"
#include "hySymbolID.h"

namespace Hayat {
    namespace Common {

        template<int N> int alignInt(int x) {
            return (x + N - 1) - ((x + N - 1) % N);
        }
        template<int N, typename T> void alignPtr(T** p) {
            *p = (T*)alignInt<N>((int)*p);
        }


        class Endian {
        public:
            typedef enum { LITTLE, BIG } Endian_t;

            static const Endian_t MY_ENDIAN = LITTLE;

            template<Endian_t E, typename T> static T unpackE(const hyu8* buf);

            template<typename T> inline static T unpack(const hyu8* buf) {
                return unpackE<Endian::MY_ENDIAN, T>(buf);
            }

            template<typename P> inline static P unpackP(const hyu8** pbuf) {
                P tmp = unpack<P>(*pbuf);
                *pbuf += sizeof(P);
                return tmp;
            }


            template<Endian_t E, typename T> static void packE(hyu8* buf, T v);

            template<typename T> inline static void pack(hyu8* buf, T v) {
                packE<Endian::MY_ENDIAN, T>(buf, v);
            }

            template<typename P> inline static void packP(hyu8** pbuf, P v) {
                pack<P>(*pbuf, v);
                *pbuf += sizeof(P);
            }


            template<Endian_t E, typename T> static void rewriteE(T* addr) {
                *((T*)addr) = unpackE<E, T>((const hyu8*)addr);
            }
            template<typename T> inline static void rewrite(T* addr) {
                rewriteE<Endian::MY_ENDIAN, T>(addr);
            }
            template<typename T> inline static void rewriteP(T** addr) {
                rewriteE<Endian::MY_ENDIAN, T>(*addr);
                ++ *addr;
            }
        };



        template<> int alignInt<2>(int x);
        template<> int alignInt<4>(int x);
        template<> int alignInt<8>(int x);


        template<> hyu32 Endian::unpackE<Endian::LITTLE, hyu32>(const hyu8* buf);
        template<> inline hys32 Endian::unpackE<Endian::LITTLE, hys32>(const hyu8* buf) {
            return (hys32)unpackE<Endian::LITTLE, hyu32>(buf); }
        template<> hyu16 Endian::unpackE<Endian::LITTLE, hyu16>(const hyu8* buf);
        template<> inline hys16 Endian::unpackE<Endian::LITTLE, hys16>(const hyu8* buf) {
            return (hys16)unpackE<Endian::LITTLE, hyu16>(buf); }
        template<> hyf32 Endian::unpackE<Endian::LITTLE, hyf32>(const hyu8* buf);
        template<> inline hyu8 Endian::unpackE<Endian::LITTLE, hyu8>(const hyu8* buf) {
            return *buf; }
        template<> inline hys8 Endian::unpackE<Endian::LITTLE, hys8>(const hyu8* buf) {
            return (hys8)*buf; }

        template<> hyu32 Endian::unpackE<Endian::BIG, hyu32>(const hyu8* buf);
        template<> inline hys32 Endian::unpackE<Endian::BIG, hys32>(const hyu8* buf) {
            return (hys32)unpackE<Endian::BIG, hyu32>(buf); }
        template<> hyu16 Endian::unpackE<Endian::BIG, hyu16>(const hyu8* buf);
        template<> inline hys16 Endian::unpackE<Endian::BIG, hys16>(const hyu8* buf) {
            return (hys16)unpackE<Endian::BIG, hyu16>(buf); }
        template<> hyf32 Endian::unpackE<Endian::BIG, hyf32>(const hyu8* buf);
        template<> inline hyu8 Endian::unpackE<Endian::BIG, hyu8>(const hyu8* buf) {
            return *buf; }
        template<> inline hys8 Endian::unpackE<Endian::BIG, hys8>(const hyu8* buf) {
            return (hys8)*buf; }


        template<> void Endian::packE<Endian::LITTLE, hyu32>(hyu8* buf, hyu32 v);
        template<> inline void Endian::packE<Endian::LITTLE, hys32>(hyu8* buf, hys32 v) {
            packE<Endian::LITTLE, hyu32>(buf, (hyu32)v); }
        template<> void Endian::packE<Endian::LITTLE, hyu16>(hyu8* buf, hyu16 v);
        template<> inline void Endian::packE<Endian::LITTLE, hys16>(hyu8* buf, hys16 v) {
            packE<Endian::LITTLE, hyu16>(buf, (hyu16)v); }
        template<> void Endian::packE<Endian::LITTLE, hyf32>(hyu8* buf, hyf32 v);
        template<> inline void Endian::packE<Endian::LITTLE, hyu8>(hyu8* buf, hyu8 v) {
            *buf = v; }
        template<> inline void Endian::packE<Endian::LITTLE, hys8>(hyu8* buf, hys8 v) {
            *buf = (hyu8)v; }

        template<> void Endian::packE<Endian::BIG, hyu32>(hyu8* buf, hyu32 v);
        template<> inline void Endian::packE<Endian::BIG, hys32>(hyu8* buf, hys32 v) {
            packE<Endian::BIG, hyu32>(buf, (hyu32)v); }
        template<> void Endian::packE<Endian::BIG, hyu16>(hyu8* buf, hyu16 v);
        template<> inline void Endian::packE<Endian::BIG, hys16>(hyu8* buf, hys16 v) {
            packE<Endian::BIG, hyu16>(buf, (hyu16)v); }
        template<> void Endian::packE<Endian::BIG, hyf32>(hyu8* buf, hyf32 v);
        template<> inline void Endian::packE<Endian::BIG, hyu8>(hyu8* buf, hyu8 v) {
            *buf = v; }
        template<> inline void Endian::packE<Endian::BIG, hys8>(hyu8* buf, hys8 v) {
            *buf = (hyu8)v; }

    }
}
    
#endif /* m_HYENDIAN_H_ */
