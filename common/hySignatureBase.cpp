/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hySignatureBase.h"

using namespace Hayat::Common;


#ifndef WIN32
// gccではこれが必要…
const SignatureBase::Sig_t SignatureBase::MULTI; // = 0xf0;
const SignatureBase::Sig_t SignatureBase::MULTI2; // = 0xf1;
#endif

const SignatureBase::Sig_t SignatureBase::PREDEFINED_PARAMS[SignatureBase::NUM_PREDEFINED_PARAMS][3] = {
    { SignatureBase::NESTNUM_1, 0 },  // (0)
    { SignatureBase::NESTNUM_1, 1 },  // (1)
    { SignatureBase::NESTNUM_1, 2 },  // (2)
    { SignatureBase::NESTNUM_1, 3 },  // (3)
    { SignatureBase::NESTNUM_1, 4 },  // (4)
    { SignatureBase::NESTNUM_1, 5 },  // (5)
    { SignatureBase::NESTNUM_1, 6 },  // (6)
    { SignatureBase::NESTNUM_1, 7 },  // (7)
    { SignatureBase::NESTNUM_1, 8 },  // (8)
    { SignatureBase::NESTNUM_1, SignatureBase::MULTI }, // (*)
    { SignatureBase::NESTNUM_1, SignatureBase::MULTI2 }, // (...)
    { SignatureBase::NESTNUM_1, SignatureBase::DEFVAL }, // ($0)
    { SignatureBase::NESTNUM_2, 1, SignatureBase::DEFVAL }, // (1,$0)
    { SignatureBase::NESTNUM_2, 2, SignatureBase::DEFVAL }, // (2,$0)
    { SignatureBase::NESTNUM_2, 3, SignatureBase::DEFVAL }, // (3,$0)
    { SignatureBase::NESTNUM_2, 1, SignatureBase::MULTI }, // (1,*)
    { SignatureBase::NESTNUM_2, 2, SignatureBase::MULTI }, // (2,*)
    { SignatureBase::NESTNUM_2, 3, SignatureBase::MULTI }, // (3,*)
    { SignatureBase::NESTNUM_2, SignatureBase::DEFVAL, SignatureBase::DEFVAL+1 }, // ($0,$1)
};
// PREDEFINED_PARAMS[][0] は NESTNUM_1 もしくは NESTNUM_2 でなければならない
// → Compiler::Package::initialize() に依存コードあり


// ネスティングの中身を丸ごとスキップ
void SignatureBase::m_skip(const Sig_t*& p)
{
    HMD_DEBUG_ASSERT(m_isNestnum(*p));
    int n = m_sig2nestnum(*p++);
    while (n-- > 0) {
        if (m_isNestnum(*p))
            m_skip(p);
    }
}

// 引数の個数
int SignatureBase::m_arity(const Sig_t*& p)
{
    int a = 0;
    int n = m_sig2nestnum(*p++);
    while (n-- > 0) {
        Sig_t s = *p;
        if (m_isNestnum(s)) {
            int x = m_arity(p);
            if (x == 0)
                ++a;
            else
                a += x;
        }
        else if (m_isMulti(s)) {
            ++a;
            ++p;
        }
        else if (m_isNormal(s)) {
            a += s; ++p;
        }
        else if (m_isDefaultVal(s)) {
            ++a;
        }
        else {
            HMD_FATAL_ERROR("unknown signature byte %x", s);
        }
    }
    return a;
}

// デフォルト値インデックスの調整
void SignatureBase::m_adjustDefaultValIdx(Sig_t*& p, int& idx)
{
    int n = m_sig2nestnum(*p++);
    while (n-- > 0) {
        Sig_t s = *p;
        if (m_isNestnum(s)) {
            m_adjustDefaultValIdx(p, idx);
        }
        else if (m_isMulti(s)) {
            if (s < MULTI) {
                // デフォルト値付きマルチ
                *p = defaultValIdx2multisig(idx++);
            }
            ++p;
        }
        else if (m_isNormal(s)) {
            ++p;
        }
        else if (m_isDefaultVal(s)) {
            *p++ = defaultValIdx2sig(idx++);
        }
        else {
            HMD_FATAL_ERROR("unknown signature byte %x", s);
        }
    }
}

void SignatureBase::debugPrintSignature(const Sig_t* p)
{
    char buf[256];
    buf[0] = '\0';
    snAddPrintSig(buf, 256, p);
    HMD_PRINTF(buf);
}

void SignatureBase::snAddPrintSig(char* buf, size_t bufSize, const Sig_t*& p)
{
    HMD_STRSCAT(buf,"(",bufSize);
    int n = m_sig2nestnum(*p);
    while (n-- > 0) {
        Sig_t s = *++p;
        if (m_isNormal(s)) {
            char xx[16];
            HMD_SNPRINTF(xx,16,"%d",s);
            HMD_STRSCAT(buf,xx,bufSize);
        }
        else if (s == MULTI2)
            HMD_STRSCAT(buf,"...",bufSize);
        else if (m_isMultiDefaultVal(*p))
            HMD_STRSCAT(buf,"*$",bufSize);
        else if (m_isMulti(s))
            HMD_STRSCAT(buf,"*",bufSize);
        else if (m_isDefaultVal(*p))
            HMD_STRSCAT(buf,"$",bufSize);
        else if (m_isSep(s))
            HMD_STRSCAT(buf,"/",bufSize);
        else
            snAddPrintSig(buf,bufSize,p);
        if (n>0)
            HMD_STRSCAT(buf,",",bufSize);
    }
    HMD_STRSCAT(buf,")",bufSize);
}

void SignatureBase::printPredefinedSignatures()
{
    char buf[256];
    for (hyu32 i = 0; i < NUM_PREDEFINED_PARAMS; ++i) {
        const Sig_t* p = PREDEFINED_PARAMS[i];
        HMD_SNPRINTF(buf, 256, "%d:%d:", i, m_arity(p));
        p = PREDEFINED_PARAMS[i];
        snAddPrintSig(buf, 256, p);
        HMD_PRINTF(buf);
        HMD_PRINTF("\n");
    }
}
