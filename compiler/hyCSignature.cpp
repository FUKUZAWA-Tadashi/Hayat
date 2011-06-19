/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCSignature.h"


using namespace Hayat::Common;
using namespace Hayat::Compiler;



void* Signature::operator new(size_t size)
{
    return (void*) gMemPool->alloc(size);
}

void Signature::operator delete(void* p)
{
    gMemPool->free(p);
}

Signature::Signature(void)
    : m_lastSigIdx(1), m_sigs(4), m_parent(NULL)
{
    m_sigs.add(m_nestnum2sig(1));
    m_sigs.add(0);
}

void Signature::m_clear(void)
{
    m_sigs.clear();
    m_sigs.add(m_nestnum2sig(1));
    m_sigs.add(0);
    m_lastSigIdx = 1;
}

Signature* Signature::openSub(void)
{
    hyu32 n = m_sigs.size();
    HMD_DEBUG_ASSERT(n > m_lastSigIdx);
    Sig_t lastSig = m_sigs[m_lastSigIdx];
    if (m_isDefaultVal(lastSig) || m_isMulti(lastSig))
        IllegalArgException::throwEx(M_M("nesting argument cannot be placed after default value or '*' argument"));

    Signature* sub = new Signature();
    sub->m_parent = this;
    return sub;
}

Signature* Signature::close(void)
{
    if (m_parent != NULL) {
        Signature* p = m_parent;
        p->m_addSubSignature(m_sigs);
        delete this;
        return p;
    }
    return NULL;
}

void Signature::addSig(Sig_t sig)
{
    HMD_DEBUG_ASSERT(! m_isNestnum(sig));
    hyu32 n = m_sigs.size();
    HMD_DEBUG_ASSERT(n > m_lastSigIdx);
    Sig_t lastSig = m_sigs[m_lastSigIdx];
    if (lastSig == 0) {
        m_sigs[m_lastSigIdx] = sig;
    } else if (m_isNormal(lastSig) && m_isNormal(sig)) {
        HMD_DEBUG_ASSERT(m_lastSigIdx == n - 1);
        m_sigs[m_lastSigIdx] = lastSig + sig;
    } else {
        if (m_isNormal(sig) && m_isDefaultVal(lastSig))
            IllegalArgException::throwEx(M_M("argument that have no default value cannot be placed after default value"));
        else if (lastSig == MULTI2 || (sig == MULTI2 && lastSig != 0))
            IllegalArgException::throwEx(M_M("no argument can be placed around '...' argument"));
        else if (m_isMulti(lastSig))
            IllegalArgException::throwEx(M_M("no argument can be placed after '*' argument"));

        m_sigs.add(sig);
        m_addNestnum();              // ++nestnum
        m_lastSigIdx = n;
    }
}

void Signature::m_addSubSignature(const TArray<Sig_t>& subSig)
{
    hyu32 n = m_sigs.size();
    Sig_t lastSig = m_sigs[m_lastSigIdx];
    if (lastSig == 0) {
        HMD_DEBUG_ASSERT(n == m_lastSigIdx + 1);
        m_sigs.remove(m_lastSigIdx);
    } else {
        m_lastSigIdx = n;
        m_addNestnum();              // ++nestnum
    }
    m_sigs.add(subSig);
}

bool Signature::canSubst(const Sig_t*& left, const Sig_t*& right)
{
    int ln = m_sig2nestnum(*left);
    int rn = m_sig2nestnum(*right);

    Sig_t l = 0;
    Sig_t r = 0;

    for (int i = 10000; i > 0; --i) {

        if (l == 0) {
            if (0 < ln) {
                --ln;
                l = *++left;
            }
        }
        if (r == 0) {
            if (0 < rn) {
                --rn;
                r = *++right;
            }
        }
        if (l == 0 && r == 0)
            return true;        // 左辺も右辺も閉じ括弧

        if (m_isNestnum(l)) {
            //########## 左辺はネスティング ##########
            if (r == 0)
                return false;
            else if (m_isNestnum(r)) {
                // 右辺もネスティング
                if (! canSubst(left, right))
                    return false;
                l = r = 0;
            }
            else {
                if (m_isNormal(r))
                    // 右辺は数値
                    --r;
                // else 右辺は *   // rに変化無し

                // 右辺の 1 は、(*) と同じ
                static const Sig_t parenZeroStar[2] = { 0x81, MULTI }; //m_nestnum2sig(1), MULTI
                const Sig_t* pPZS = parenZeroStar;
                if (!canSubst(left, pPZS))
                    return false;
                l = 0;
            }
        }

        else if (m_isMulti(l)) {
            //########## 左辺は*である ##########

            if (m_isMulti(r)) {
                // 左辺も右辺も*である
                r = l = 0;
            }

            else if (m_isNestnum(r)) {
                // 右辺はネスティング
                m_skip(right);
                r = 0;
            }

            else { // if (m_isNormal(r))
                // 右辺は数値
                HMD_DEBUG_ASSERT(m_isNormal(r));
                if (r == 0) {
                    l = 0;      // 右辺が閉じ括弧なら左辺の*は終了
                } else {
                    r = 0;      // 全ての r は代入できた事とする
                }
            }
            
        }


        else if (m_isDefaultVal(l)) {
            //########## 左辺はデフォルト値 ##########
            if (m_isNestnum(r)) {
                // 右辺はネスティング
                m_skip(right);
                r = 0;
            }
            else if (m_isNormal(r)) {
                // 右辺は数値
                if (r > 0)
                    --r;
            }
            else {
                HMD_DEBUG_ASSERT(m_isMulti(r));
                // 右辺は*
            }
            l = 0;
        }


        else { // if (m_isNormal(l))
            //########## 左辺は数値 ##########
            if (m_isNestnum(r)) {
                // 右辺はネスティング
                if (l == 0)
                    return false;
                --l;
                m_skip(right);
                r = 0;
            }
            else if (m_isMulti(r)) {
                // 右辺は*である
                if (l == 0) {
                    r = 0;      // 左辺が閉じ括弧なら右辺の*は終了
                } else {
                    l = 0;      // 全ての l に代入できた事とする
                }
            }
            else {      //if (m_isNormal(r))
                // 右辺は数値
                HMD_DEBUG_ASSERT(m_isNormal(r));
                // r == 0 && l == 0 は冒頭でチェック済み
                if (r == 0 || l == 0)
                    return false; // 片方が閉じ括弧なのに数が合わない
                if (l <= r) {
                    r -= l;
                    l = 0;
                } else { // l > r
                    l -= r;
                    r = 0;
                }
            }
        }
    }

    HMD_FATAL_ERROR("signature data maybe broken");
    return false;
}


bool Signature::canSubst(const Signature& left, const Signature& right)
{
    const Sig_t* lp = left.m_sigs.top();
    const Sig_t* rp = right.m_sigs.top();
    return canSubst(lp, rp);
}


hyu8 Signature::arity(const Sig_t* p)
{
    int a = m_arity(p);
    HMD_ASSERT(a >= 0 && a < 128);
    return (hyu8)a;
}


#ifdef HMD_DEBUG
void Signature::debugPrintSignature(void)
{
    const Sig_t* p = m_sigs.top();
    SignatureBase::debugPrintSignature(p);
}
#endif
