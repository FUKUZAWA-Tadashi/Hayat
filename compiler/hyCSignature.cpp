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
            return true;        // ���ӂ��E�ӂ�������

        if (m_isNestnum(l)) {
            //########## ���ӂ̓l�X�e�B���O ##########
            if (r == 0)
                return false;
            else if (m_isNestnum(r)) {
                // �E�ӂ��l�X�e�B���O
                if (! canSubst(left, right))
                    return false;
                l = r = 0;
            }
            else {
                if (m_isNormal(r))
                    // �E�ӂ͐��l
                    --r;
                // else �E�ӂ� *   // r�ɕω�����

                // �E�ӂ� 1 �́A(*) �Ɠ���
                static const Sig_t parenZeroStar[2] = { 0x81, MULTI }; //m_nestnum2sig(1), MULTI
                const Sig_t* pPZS = parenZeroStar;
                if (!canSubst(left, pPZS))
                    return false;
                l = 0;
            }
        }

        else if (m_isMulti(l)) {
            //########## ���ӂ�*�ł��� ##########

            if (m_isMulti(r)) {
                // ���ӂ��E�ӂ�*�ł���
                r = l = 0;
            }

            else if (m_isNestnum(r)) {
                // �E�ӂ̓l�X�e�B���O
                m_skip(right);
                r = 0;
            }

            else { // if (m_isNormal(r))
                // �E�ӂ͐��l
                HMD_DEBUG_ASSERT(m_isNormal(r));
                if (r == 0) {
                    l = 0;      // �E�ӂ������ʂȂ獶�ӂ�*�͏I��
                } else {
                    r = 0;      // �S�Ă� r �͑���ł������Ƃ���
                }
            }
            
        }


        else if (m_isDefaultVal(l)) {
            //########## ���ӂ̓f�t�H���g�l ##########
            if (m_isNestnum(r)) {
                // �E�ӂ̓l�X�e�B���O
                m_skip(right);
                r = 0;
            }
            else if (m_isNormal(r)) {
                // �E�ӂ͐��l
                if (r > 0)
                    --r;
            }
            else {
                HMD_DEBUG_ASSERT(m_isMulti(r));
                // �E�ӂ�*
            }
            l = 0;
        }


        else { // if (m_isNormal(l))
            //########## ���ӂ͐��l ##########
            if (m_isNestnum(r)) {
                // �E�ӂ̓l�X�e�B���O
                if (l == 0)
                    return false;
                --l;
                m_skip(right);
                r = 0;
            }
            else if (m_isMulti(r)) {
                // �E�ӂ�*�ł���
                if (l == 0) {
                    r = 0;      // ���ӂ������ʂȂ�E�ӂ�*�͏I��
                } else {
                    l = 0;      // �S�Ă� l �ɑ���ł������Ƃ���
                }
            }
            else {      //if (m_isNormal(r))
                // �E�ӂ͐��l
                HMD_DEBUG_ASSERT(m_isNormal(r));
                // r == 0 && l == 0 �͖`���Ń`�F�b�N�ς�
                if (r == 0 || l == 0)
                    return false; // �Е��������ʂȂ̂ɐ�������Ȃ�
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
