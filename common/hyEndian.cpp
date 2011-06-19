/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyEndian.h"

using namespace Hayat::Common;


namespace Hayat {
    namespace Common {
        template<> int alignInt<2>(int x) {
            return (x + 1) & ~1;
        }
        template<> int alignInt<4>(int x) {
            return (x + 3) & ~3;
        }
        template<> int alignInt<8>(int x) {
            return (x + 7) & ~7;
        }
    }
}



typedef union {
    hyu32 u;
    hyf32 f;
} v_t;


template<> hyu32 Endian::unpackE<Endian::LITTLE, hyu32>(const hyu8* buf)
{
    hyu32 v;
    v = *buf++;
    v |= (*buf++) << 8;
    v |= (*buf++) << 16;
    v |= (*buf) << 24;
    return v;
}
template<> hyu32 Endian::unpackE<Endian::BIG, hyu32>(const hyu8* buf)
{
    hyu32 v;
    v = (*buf++) << 24;
    v |= (*buf++) << 16;
    v |= (*buf++) << 8;
    v |= *buf;
    return v;
}

template<> hyu16 Endian::unpackE<Endian::LITTLE, hyu16>(const hyu8* buf)
{
    hyu16 v;
    v = *buf++;
    v |= (*buf) << 8;
    return v;
} 
template<> hyu16 Endian::unpackE<Endian::BIG, hyu16>(const hyu8* buf)
{
    hyu16 v;
    v = (*buf++) << 8;
    v |= *buf;
    return v;
}

template<> hyf32 Endian::unpackE<Endian::LITTLE, hyf32>(const hyu8* buf)
{
    v_t v;
    v.u = unpackE<LITTLE, hyu32>(buf);
    return v.f;
}
template<> hyf32 Endian::unpackE<Endian::BIG, hyf32>(const hyu8* buf)
{
    v_t v;
    v.u = unpackE<BIG, hyu32>(buf);
    return v.f;
}




template<> void Endian::packE<Endian::LITTLE, hyu32>(hyu8* buf, hyu32 v)
{
    *buf = (hyu8)v;
    *++buf = (hyu8)(v >> 8);
    *++buf = (hyu8)(v >> 16);
    *++buf = (hyu8)(v >> 24);
}
template<> void Endian::packE<Endian::BIG, hyu32>(hyu8* buf, hyu32 v)
{
    *buf = (hyu8)(v >> 24);
    *++buf = (hyu8)(v >> 16);
    *++buf = (hyu8)(v >> 8);
    *++buf = (hyu8)v;
}

template<> void Endian::packE<Endian::LITTLE, hyu16>(hyu8* buf, hyu16 v)
{
    *buf = (hyu8)v;
    *++buf = (hyu8)(v >> 8);
}
template<> void Endian::packE<Endian::BIG, hyu16>(hyu8* buf, hyu16 v)
{
    *buf = (hyu8)(v >> 8);
    *++buf = (hyu8)v;
}

template<> void Endian::packE<Endian::LITTLE, hyf32>(hyu8* buf, hyf32 v)
{
    v_t vf;
    vf.f = v;
    packE<LITTLE, hyu32>(buf, vf.u);
}
template<> void Endian::packE<Endian::BIG, hyf32>(hyu8* buf, hyf32 v)
{
    v_t vf;
    vf.f = v;
    packE<BIG, hyu32>(buf, vf.u);
}
