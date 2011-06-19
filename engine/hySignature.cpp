/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hySignature.h"
#include "hyContext.h"
#include "hyMemPool.h"
#include "hyFiber.h"
#include "hyValueArray.h"

using namespace Hayat::Common;
using namespace Hayat::Engine;




void* Signature::operator new(size_t size)
{
#ifndef HMD_DEBUG
    (void)size;
#endif
    HMD_DEBUG_ASSERT(size == sizeof(Signature));
    return (void*) gMemPool->alloc(sizeof(Signature), "SIGN");
}

void Signature::operator delete(void* p)
{
    gMemPool->free((MemCell*)p);
}

Signature::Signature(hyu8 numOnStack, const Sig_t* sigBytes, hyu32 sigBytesLen, Context* context)
    : m_sigs(sigBytes, sigBytesLen), m_context(context)
{
    m_sp = m_context->stack.size() - numOnStack;
}

Signature::Signature(hyu8 numArgs, Context* context)
    : m_sigs(0), m_context(context)
{
    static const Sig_t NESTNUM_1 = 0x81; // == m_nestnum2sig(1)
    static const Sig_t basicSigs[9][2] = {
        { NESTNUM_1, 0 },  // (0)
        { NESTNUM_1, 1 },  // (1)
        { NESTNUM_1, 2 },  // (2)
        { NESTNUM_1, 3 },  // (3)
        { NESTNUM_1, 4 },  // (4)
        { NESTNUM_1, 5 },  // (5)
        { NESTNUM_1, 6 },  // (6)
        { NESTNUM_1, 7 },  // (7)
        { NESTNUM_1, 8 },  // (8)
    };

    HMD_ASSERT(m_isNormal((Sig_t)numArgs));

    if (numArgs <= 8) {
        m_sigs.initialize(basicSigs[numArgs], 2);
    } else {
        m_sigs.initialize(2);
        Sig_t* p = m_sigs.addSpaces(2);
        *p = NESTNUM_1;
        *++p = (Sig_t)numArgs;
        m_sigs.setContentsMemID("Sigs");
    }
    if (context == NULL)
        m_sp = 0;
    else
        m_sp = m_context->stack.size() - numArgs;
}


Signature::~Signature()
{
}

void Signature::setNumOnStack(hyu32 n)
{
    m_sp = m_context->stack.size() - n;
}


void Signature::clearRight(void)
{
    m_context->stack.dropTo(m_sp);
}


// ���̉E��Signature���w�荶��Signature�ɑ���\���ǂ������ׂ�
// �K�v�Ȃ璆�g��W�J���Ē��ׂ� (Signature�͕ω�����)
// ����\�Ȃ�X�^�b�N������Signature�ɉ����Đ������āAtrue��Ԃ�
bool Signature::canSubstTo(const Sig_t* sigBytes, const HClass* defValClass, hyu16 defValOffs)
{
    Sig_t* right = m_sigs.top();
    const Sig_t* left = sigBytes;
    hyu32 sp = m_sp;
#if 0
    {
        HMD__PRINTF_FK("left: ");
        const Sig_t* p = sigBytes;
        printSig(p);
        HMD__PRINTF_FK("\nright: ");
        printSigs();
        //m_context->debugPrintStack();
    }
#endif
    if (m_canSubst(left, right, sp, defValClass, defValOffs)) {
#if 0
        HMD__PRINTF_FK("can. right -> ");
        printSigs();
        //m_context->debugPrintStack();
#endif
        HMD_DEBUG_ASSERT(right == m_sigs.nthAddr(m_sigs.size() - 1));
        // ����\.
        m_adjustTo(sigBytes, defValClass, defValOffs);
#if 0
        HMD__PRINTF_FK("adjust -> ");
        printSigs();
        //m_context->debugPrintStack();
#endif
        return true;
    }
#if 0
    else {
        HMD__PRINTF_FK("can not ! right -> ");
        printSigs();
    }
#endif
    return false;
}

bool Signature::m_canSubst(const Sig_t*& left, Sig_t*& right, hyu32& sp, const HClass* defValClass, hyu16 defValOffs)
{
    Sig_t* rnp = right;
    int ln = m_sig2nestnum(*left);
    int rn = m_sig2nestnum(*right);
    Sig_t l = 0;
    Sig_t r = 0;

    for (int i = 10000; i > 0; --i) {

        if (l == 0) {
            if (ln > 0) {
                --ln;
                l = *++left;
                // ���ӂɃZ�p���[�^�͓����Ă��Ȃ�
                HMD_DEBUG_ASSERT(! m_isSep(l));
            }
        }
        if (r == 0) {
            if (rn > 0) {
                --rn;
                r = *++right;
                // �E�ӂ�0�������Ă���\������
            }
        }

        if (l == 0) {
            //########## �������Ӗ��� ##########
            if (r == 0) {
                if (rn <= 0)        // �E�ӂ������̂�OK
                    return true;
            }
            else if (r == SEP) {    // ��؂�Ȃ̂Ŏ����`�F�b�N
                r = 0;          
            }
            else if (m_isMulti(r)) {     // *
                m_onwrite(right, rnp); // �������݉ɂ���
                switch (m_expand(sp)) {      // *��W�J
                case 1:
                    // 1,* �ɂȂ���
                    ++sp;
                    if (m_isNormal(right[-1])) {
                        ++(right[-1]);
                    } else {
                        m_insert(right, 1, rnp);
                        ++*rnp;
                        *right = 1;
                        *++right = MULTI;
                    }
                    return false;
                case 0:
                    // 0 �ɂȂ���
                    if (m_isNormal(right[-1])) {
                        *right = SEP;
                    } else {
                        ++*rnp;
                        m_insert(right, 1, rnp);
                        *right = 0;
                        *++right = SEP;
                    }
                    r = 0;
                    break;
                default:  // case -1
                    // �W�J�s�\�@
                    return false;
                }
            }
            else {            // r>0,�l�X�e�B���O
                return false;
            }


        } else if (l == MULTI2) {
            //########## ffi�̉ϒ����� ##########
            if (r == 0)
                return true;
            if (m_isNormal(r)) {
                sp += r;
            }
            else if (m_isNestnum(r)) {
                m_skip(right, sp);
            }
            else if (m_isMulti(r)) {
                m_onwrite(right, rnp); // m_sigs�������݉ɂ���
                int exresult;
                while (1 == (exresult = m_expand(sp))) {
                    // �W�J����
                    ++sp;
                    if (m_isNormal(right[-1])) {
                        ++(right[-1]);
                    } else {
                        ++*rnp;
                        m_insert(right, 1, rnp);
                        *right = 1;
                        *++right = MULTI;
                    }
                }
                if (exresult == 0) {
                    // �W�J�f�[�^���s����
                    *right = SEP;
                }
            }
            r = 0;

        } else if (m_isMulti(l)) {
            //########## ���ӂ�*�ł��� ##########
            if (m_isSep(r)) {    // ��؂�Ȃ̂ł���ȏ㍶�ӂ� * �ɋ����͖���
                r = l = 0;
            } else if (m_isMulti(r)) {
                // �E�ӂ�*�ł���
                ++sp;
                r = l = 0;
            } else if (m_isNormal(r)) {
                // �E�ӂ͐��l
                if (r == 0) {
                    l = 0;      // ���ӂւ̋����I��
                } else {
                    sp += r;
                    r = 0;      // �E�ӂ�S�����ӂɋ������Ď�
                }
            } else {
                HMD_ASSERT(m_isNestnum(r));
                // �E�ӂ̓l�X�e�B���O
                m_skip(right, sp);
                r = 0;          // ���ӂɋ������Ď�
            }


        } else if (m_isNormal(l)) {
            //########## ���ӂ͐��l ##########
            if (m_isSep(r)) {    // ��؂�Ȃ̂Ŏ����`�F�b�N
                r = 0;          
            } else if (m_isMulti(r)) {
                // �E�ӂ�*�ł���
                m_onwrite(right, rnp); // m_sigs�������݉ɂ���
                while (l > 0) {
                    switch (m_expand(sp)) {
                    case 1:
                        // �W�J�ł���
                        ++sp;
                        if (m_isNormal(right[-1])) {
                            ++(right[-1]);
                        } else {
                            ++*rnp;
                            m_insert(right, 1, rnp);
                            *right = 1;
                            *++right = MULTI;
                        }
                        --l;
                        break;
                    case 0:
                        // �W�J�f�[�^���s����
                        *right = SEP;
                        return false;
                    default:
                        // �W�J�s�\�@
                        return false;
                    }
                }
            } else if (m_isNormal(r)) {
                // �E�ӂ͐��l
                if (r == 0)
                    return false;
                else if (l <= r) {
                    sp += l;
                    r -= l;
                    l = 0;
                }
                else { // l > r
                    sp += r;
                    l -= r;
                    r = 0;
                }
            } else {
                HMD_ASSERT(m_isNestnum(r));
                // �E�ӂ̓l�X�e�B���O
                --l;
                m_skip(right, sp);
                r = 0;
            }


        } else if (m_isDefaultVal(l)) {
            //########## ���ӂ̓f�t�H���g�l ##########
            if (m_isSep(r)) {
                // ��؂�
                // �f�t�H���g�l�g�p
            } else if (m_isNestnum(r)) {
                // �E�ӂ̓l�X�e�B���O
                m_skip(right, sp);
                r = 0;
            } else if (m_isNormal(r)) {
                // �E�ӂ͐��l
                if (r > 0)
                    --r;
                // else �f�t�H���g�l�g�p
            } else {
                HMD_DEBUG_ASSERT(m_isMulti(r));
                // �E�ӂ�*
                m_onwrite(right, rnp); // m_sigs�������݉ɂ���
                switch (m_expand(sp)) {
                case 1:
                    // �W�J�ł���
                    ++sp;
                    if (m_isNormal(right[-1])) {
                        ++(right[-1]);
                    } else {
                        ++*rnp;
                        m_insert(right, 1, rnp);
                        *right = 1;
                        *++right = MULTI;
                    }
                    break;
                case 0:
                    // �W�J�f�[�^���s����
                    // �f�t�H���g�l�g�p
                    *right = SEP;
                    r = 0;
                    break;
                default:
                    // �W�J�s�\�@
                    return false;
                }
            }
                
            l = 0;

        } else {
            HMD_ASSERT(m_isNestnum(l));
            //########## ���ӂ̓l�X�e�B���O ##########
            if (m_isSep(r)) {    // ��؂�Ȃ̂Ŏ����`�F�b�N
                r = 0;          
                continue;
            }
            else if (m_isNestnum(r)) {
                HMD_ASSERT(m_isNestnum(r));
                // �E�ӂ��l�X�e�B���O
                if (! m_canSubst(left, right, sp, defValClass, defValOffs))
                    return false;
                l = r = 0;
                continue;
            }
            else if (m_isMulti(r)) {
                // �E�ӂ� *
                m_onwrite(right, rnp); // m_sigs�������݉ɂ���
                switch (m_expand(sp)) {
                case 1:
                    // �W�J�ł���
                    if (m_isNormal(right[-1])) {
                        ++rn;
                        ++(*--right);
                    } else {
                        ++rn; ++*rnp;
                        m_insert(right, 1, rnp);
                        *right = 1;
                        right[1] = MULTI;
                    }
                    r = 1;
                    break;
                case 0:
                    // �W�J�f�[�^���s����
                    *right = SEP;
                    return false;
                default:
                    // �W�J�s�\�@
                    return false;
                }
            }

            // �E�ӂ͐��l
            HMD_DEBUG_ASSERT(m_isNormal(r));
            if (r == 0) // �E�ӂ� 0 �� ���s
                return false;

            if (! m_isExpandable(m_context->stack.getAt(sp)))
                return false;   // 1 �� (*) �ɓW�J�ł��Ȃ�

            m_onwrite(right, rnp); // m_sigs�������݉ɂ���
            if (r == 1) {
                Sig_t rr = *right;
                if (rr > 1) {
                    // n �� n-1,(*)
                    m_insert(right, 2, rnp);
                    right[0] = rr - 1;
                    right[1] = m_nestnum2sig(1);
                    right[2] = MULTI;
                    ++right; ++*rnp;
                } else {    // rr == 1
                    // 1 �� (*)
                    m_insert(right, 1, rnp);
                    right[0] = m_nestnum2sig(1);
                    right[1] = MULTI;
                }
            }
            else {  // r > 1
                Sig_t d = *right - r;
                if (d == 0) {
                    // n �� (*),n-1
                    ++rn; ++*rnp;
                    m_insert(right, 2, rnp);
                } else {
                    // n �� d,(*),r-1    where d + r == n
                    ++rn; *rnp += 2;
                    m_insert(right, 3, rnp);
                    *right++ = d;
                }
                right[0] = m_nestnum2sig(1);
                right[1] = MULTI;
                right[2] = r - 1;
            }
            r = *right;
        }

    }

    HMD_FATAL_ERROR("signature data maybe broken");
    return false;
}


// �X�^�b�N������Signature�ɉ����Đ�������
void Signature::m_adjustTo(const Sig_t* sigBytes, const HClass* defValClass, hyu16 defValOffs)
{
    const Sig_t* right = m_sigs.top();
    const Sig_t* left = sigBytes;
    hyu32 sp = m_sp;
    m_adjust(left, right, sp, defValClass, defValOffs);
    HMD_DEBUG_ASSERT(right == m_sigs.nthAddr(m_sigs.size() - 1));
}

void Signature::m_adjust(const Sig_t*& left, const Sig_t*& right, hyu32& sp, const HClass* defValClass, hyu16 defValOffs)
{
    ValueStack& stack = m_context->stack;

    int ln = m_sig2nestnum(*left);
    int rn = m_sig2nestnum(*right);
    Sig_t l = 0;
    Sig_t r = 0;

    for (int i = 10000; i > 0; --i) {

        if (l == 0) {
            if (ln > 0) {
                --ln;
                l = *++left;
            }
        }
        if (r == 0) {
            if (rn > 0) {
                --rn;
                r = *++right;
            }
        }

        if (l == MULTI2) {
            //========== ffi�̉ϒ����� ==========
            if (r == 0)
                return;
            if (m_isNormal(r)) {
                sp += r;
            } else if (m_isNestnum(r)) {
                m_bundle(right, sp);
            } else {
                HMD_DEBUG_ASSERT(! m_isMulti(r));
            }
            r = 0;

        } else if (l == 0) {
            //========== �������Ӗ��� ==========
            HMD_DEBUG_ASSERT(r == 0 || m_isSep(r));
            while (--rn >= 0) {
                ++right;
                HMD_DEBUG_ASSERT(*right == 0 || m_isSep(*right));
            }
            return;

        } else if (m_isNormal(l)) {
            //========== ���ӂ͐��l ==========
            if (m_isNormal(r)) {
                if (l >= r) {
                    l -= r; sp += r; r = 0;
                } else {
                    r -= l; sp += l; l = 0;
                }
            }
            else if (m_isNestnum(r)) {
                --l;
                m_bundle(right, sp);
                r = 0;
            }
            else {
                HMD_DEBUG_ASSERT(! m_isMulti(r));
                r = 0;
            }


        } else if (m_isMulti(l)) {
            //========== ���ӂ� * ==========

            if (m_isMultiDefaultVal(l)) {
                // ���ӂ̓f�t�H���g�l�t�� *
                if (m_isMulti(r)) {
                    // �E�ӂ� *
                    switch (m_expand(sp)) {
                    case 1:
                        // �l������̂Ō㔼�̏����ɑ���
                        r = 1;
                        ++rn;
                        --right;
                        break;
                    case 0:
                        // �󂾂����̂Ńf�t�H���g�l�����
                        r = SEP;
                        break;
                    }
                }

                if (r == 0 || m_isSep(r)) {
                    // �f�t�H���g�l���
                    stack.insertAt(sp, 1);
                    Value* pDefVal = defValClass->getDefaultValAddr(defValOffs + m_sig2DefaultValIdx(l));
                    HMD_ASSERTMSG(pDefVal != NULL, M_M("class %s not initialized for default value"), defValClass->name());
                    stack.setAt(sp, *pDefVal);
                    ++sp;
                    r = l = 0;
                    continue;
                }
            }
            
            if (m_isMulti(r)) {
                l = r = 0;      // �E�ӂ� *
                ++sp;
            } else {
                HMD_DEBUG_ASSERT(! m_isSep(r));
                ValueList* root = NULL;
                ValueList* vp = NULL;
                    
                int j;
                for (j = 10000; j > 0; --j) {
                    if (r == 0 || m_isSep(r)) {
                        stack.insertAt(sp, 1);
                        stack.setAt(sp, Value(HC_List, root));
                        l = r = 0;
                        ++sp;
                        break;
                    } else if (m_isMulti(r)) {
                        m_context->pushList(root);
                        m_context->push(stack.getAt(sp));
                        bool b = m_context->execMethod(Value(HC_Class, (void*)HC_Fiber),
                                                      HSym_joint,
                                                      2);
                        HMD_DEBUG_ASSERT(b);
                        Value jointed = m_context->pop();
                        stack.setAt(sp, jointed);
                        r = l = 0;
                        ++sp;
                        break;
                    } else if (m_isNestnum(r)) {
                        m_bundle(right, sp);
                        r = 1;
                    }

                    HMD_DEBUG_ASSERT(m_isNormal(r));
                    while (r-- > 0) {
                        Object* obj = HC_List->createInstance_noInit(sizeof(ValueList));
                        ValueList* p = obj->cppObj<ValueList>();
                        p->setContent(stack.getAt(sp));
                        if (vp == NULL)
                            root = p;
                        else
                            vp->link(p);
                        vp = p;
                        stack.removeAt(sp);
                    }

                    if (rn > 0) {
                        --rn;
                        r = *++right;
                    } else
                        r = 0;
                }
                if (j <= 0)
                    HMD_FATAL_ERROR("bug");
            }

        } else if (m_isDefaultVal(l)) {
            //========== ���ӂ̓f�t�H���g�l ==========
            if (r == 0 || m_isSep(r)) {
                HMD_ASSERT(defValClass != NULL);
                stack.insertAt(sp, 1);
                Value* pDefVal = defValClass->getDefaultValAddr(defValOffs + m_sig2DefaultValIdx(l));
                HMD_ASSERTMSG(pDefVal != NULL, M_M("class %s not initialized for default value"), defValClass->name());
                stack.setAt(sp, *pDefVal);
                r = 0;
                ++sp;
            } else if (m_isNormal(r)) {
                --r;
                ++sp;
            } else if (m_isNestnum(r)) {
                m_bundle(right, sp);
                r = 0;
            } else {
                HMD_DEBUG_ASSERT(!m_isMulti(r));
            }
            l = 0;

        } else {
            //========== ���ӂ̓l�X�g ==========
            HMD_DEBUG_ASSERT(m_isNestnum(l));
            HMD_DEBUG_ASSERT(m_isNestnum(r));
            m_adjust(left, right, sp, defValClass, defValOffs);
            l = r = 0;
        }
    }
    HMD_FATAL_ERROR("bug");
}


void Signature::m_bundle(const Sig_t*& right, hyu32& sp)
{
    ValueStack& stack = m_context->stack;
    ValueList* root = NULL;
    ValueList* vp = NULL;
    ValueArray genArr(2);
    
    for (int rn = m_sig2nestnum(*right); rn > 0; --rn) {
        Sig_t r = *++right;
        if (m_isSep(r)) {
            continue;
        }
        else if (m_isNestnum(r)) {
            m_bundle(right, sp);
            r = 1;
        }

        if (m_isMulti(r)) {
            genArr.add(stack.getAt(sp));
            stack.removeAt(sp);
            root = vp = NULL;
        } else {
            HMD_DEBUG_ASSERT(m_isNormal(r));
            while (r-- > 0) {
                Object* obj = HC_List->createInstance_noInit(sizeof(ValueList));
                ValueList* p = obj->cppObj<ValueList>();
                p->setContent(stack.getAt(sp));
                if (vp == NULL) {
                    root = p;
                    genArr.add(Value::fromList(root));
                } else
                    vp->link(p);
                vp = p;
                stack.removeAt(sp);
            }
        }
    }

    stack.insertAt(sp, 1);
    hyu32 n = genArr.size();
    if (n == 0) {
        stack.setAt(sp, EMPTY_LIST_VALUE);
    } else if (n == 1) {
        stack.setAt(sp, genArr[0]);
    } else {
        Object* obj = HC_Array->createInstance_noInit(sizeof(ValueArray));
        ValueArray* objArr = obj->cppObj<ValueArray>();
        objArr->initialize(n);
        for (hyu32 i = 0; i < n; ++i)
            objArr->add(genArr[i]);
        bool b = m_context->execMethod(Value::fromObj(obj), HSym_eacheach, 0);
        HMD_DEBUG_ASSERT(b);
        Value eacheachFiber = m_context->pop();
        stack.setAt(sp, eacheachFiber);
    }

}


void Signature::m_onwrite(Sig_t*& p, Sig_t*& q)
{
    Sig_t* x = m_sigs.top();
    size_t dp = p - x;
    HMD_DEBUG_ASSERT(dp < m_sigs.size());
    size_t dq = q - x;
    HMD_DEBUG_ASSERT(dq < m_sigs.size());
    m_sigs.reserve(0);           // �O���Q�Ə�ԂȂ�ʏ�g�p��ԂɈڍs
    p = m_sigs.nthAddr(dp);
    q = m_sigs.nthAddr(dq);
}

void Signature::m_insert(Sig_t*& p, hyu32 n, Sig_t*& q)
{
    size_t dp = p - m_sigs.top();
    HMD_DEBUG_ASSERT(dp < m_sigs.size());
    size_t dq = q - m_sigs.top();
    HMD_DEBUG_ASSERT(dq < m_sigs.size());
    Sig_t& v = m_sigs.insert(dp, n);
    p = &v;
    q = m_sigs.nthAddr(dq);
}

// �l���W�J�\���ǂ���
// List�Ȃ�W�J�\�@
// Fiber�Ȃ�W�J�\�@
// each()�����I�u�W�F�N�g�Ȃ�W�J�\�@
// �ǂ�����Ă͂܂�Ȃ���ΓW�J�s�\�@
bool Signature::m_isExpandable(Value& v)
{
    const HClass* pClass = v.getType();
    if (pClass->symCheck(HSym_List))
        return true;
    if (pClass->symCheck(HSym_Fiber))
        return true;
    Signature zeroSig(0, m_context);
    MethodPtr methodPtr(v);
    if (pClass->searchMethod(&methodPtr, HSym_each, &zeroSig))
        return true;
    return false;
}


// �X�^�b�N��sp�ʒu�̒l��W�J
// (1)List�Ȃ� head �� tail �ɓW�J����
// (2)Fiber�Ȃ�generate()����
// (3)Fiber�łȂ���΁Aeach()���Ă��Fiber�𐶐����āAgenerate()����
// �W�J�ł�����Asp��head������, sp+1��tail���}������A 1 ��Ԃ�
// �W�J���Ēl���s���Ă�����Asp�̒l���폜���A 0 ��Ԃ�
// �W�J���ł��Ȃ��l��������A�X�^�b�N�͕ω���������  -1 ��Ԃ�
int Signature::m_expand(hyu32 sp)
{
    ValueStack& stack = m_context->stack;
    Value v = stack.getAt(sp);

    if (v.type->symCheck(HSym_List)) {
        ValueList* list = (ValueList*) v.ptrData;
        if (list != NULL) {
            stack.insertAt(sp, 1);
            stack.setAt(sp, list->head());
            Value* p = stack.addrAt(sp+1);
            p->type = HC_List;
            p->ptrData = list->tail();
            return 1;
        } else {
            stack.removeAt(sp);
            return 0;
        }
    }

    if (! v.getType()->symCheck(HSym_Fiber)) {
        if (0 >= m_context->execMethod_ifExist(v, HSym_each, 0)) {
            return -1;
        }
        v = stack.pop();
        if (! v.getType()->symCheck(HSym_Fiber)) {
            return -1;
        }
    }
    Fiber* fiber = v.toObj()->cppObj<Fiber>();
    if (! fiber->isFinished()) {
        bool b = m_context->execMethod(v, HSym_generate_noThrow, 0);
        HMD_DEBUG_ASSERT(b);
        Value retval = m_context->pop();
        if (! fiber->isFinished()) {
            // generate�ł���
            stack.insertAt(sp, 1);
            stack.setAt(sp, retval);
            stack.setAt(sp+1, v);
            return 1;
        }
    }
    // generate������̂�������������
    stack.removeAt(sp);
    return 0;
}

// �l�X�e�B���O�I���܂ŃX�L�b�v
void Signature::m_skip(Sig_t*& right, hyu32& sp)
{
    HMD_DEBUG_ASSERT(m_isNestnum(*right));
    int n = m_sig2nestnum(*right);
    while (n-- > 0) {
        Sig_t r = *++right;
        if (m_isNestnum(r))
            m_skip(right, sp);
        else {
            if (m_isNormal(r))
                sp += r;
            else if (m_isMulti(r))
                ++sp;
            HMD_DEBUG_ASSERT(sp <= m_context->stack.size());
        }
    }
}

int Signature::getNumOnStack(void)
{
    return m_context->stack.size() - m_sp;
}


#ifdef HMD_DEBUG
void Signature::printSigs(void)
{
    const Sig_t* p = m_sigs.top();
    debugPrintSignature(p);
    HMD_PRINTF("\n");
}
#endif
