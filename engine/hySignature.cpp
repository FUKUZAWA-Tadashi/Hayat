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


// この右辺Signatureが指定左辺Signatureに代入可能かどうか調べる
// 必要なら中身を展開して調べる (Signatureは変化する)
// 代入可能ならスタックを左辺Signatureに沿って整理して、trueを返す
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
        // 代入可能.
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
                // 左辺にセパレータは入っていない
                HMD_DEBUG_ASSERT(! m_isSep(l));
            }
        }
        if (r == 0) {
            if (rn > 0) {
                --rn;
                r = *++right;
                // 右辺は0が入っている可能性あり
            }
        }

        if (l == 0) {
            //########## もう左辺無し ##########
            if (r == 0) {
                if (rn <= 0)        // 右辺も無いのでOK
                    return true;
            }
            else if (r == SEP) {    // 区切りなので次をチェック
                r = 0;          
            }
            else if (m_isMulti(r)) {     // *
                m_onwrite(right, rnp); // 書き込み可にする
                switch (m_expand(sp)) {      // *を展開
                case 1:
                    // 1,* になった
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
                    // 0 になった
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
                    // 展開不能　
                    return false;
                }
            }
            else {            // r>0,ネスティング
                return false;
            }


        } else if (l == MULTI2) {
            //########## ffiの可変長引数 ##########
            if (r == 0)
                return true;
            if (m_isNormal(r)) {
                sp += r;
            }
            else if (m_isNestnum(r)) {
                m_skip(right, sp);
            }
            else if (m_isMulti(r)) {
                m_onwrite(right, rnp); // m_sigs書き込み可にする
                int exresult;
                while (1 == (exresult = m_expand(sp))) {
                    // 展開した
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
                    // 展開データが尽きた
                    *right = SEP;
                }
            }
            r = 0;

        } else if (m_isMulti(l)) {
            //########## 左辺は*である ##########
            if (m_isSep(r)) {    // 区切りなのでこれ以上左辺の * に供給は無い
                r = l = 0;
            } else if (m_isMulti(r)) {
                // 右辺も*である
                ++sp;
                r = l = 0;
            } else if (m_isNormal(r)) {
                // 右辺は数値
                if (r == 0) {
                    l = 0;      // 左辺への供給終了
                } else {
                    sp += r;
                    r = 0;      // 右辺を全部左辺に供給して次
                }
            } else {
                HMD_ASSERT(m_isNestnum(r));
                // 右辺はネスティング
                m_skip(right, sp);
                r = 0;          // 左辺に供給して次
            }


        } else if (m_isNormal(l)) {
            //########## 左辺は数値 ##########
            if (m_isSep(r)) {    // 区切りなので次をチェック
                r = 0;          
            } else if (m_isMulti(r)) {
                // 右辺は*である
                m_onwrite(right, rnp); // m_sigs書き込み可にする
                while (l > 0) {
                    switch (m_expand(sp)) {
                    case 1:
                        // 展開できた
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
                        // 展開データが尽きた
                        *right = SEP;
                        return false;
                    default:
                        // 展開不能　
                        return false;
                    }
                }
            } else if (m_isNormal(r)) {
                // 右辺は数値
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
                // 右辺はネスティング
                --l;
                m_skip(right, sp);
                r = 0;
            }


        } else if (m_isDefaultVal(l)) {
            //########## 左辺はデフォルト値 ##########
            if (m_isSep(r)) {
                // 区切り
                // デフォルト値使用
            } else if (m_isNestnum(r)) {
                // 右辺はネスティング
                m_skip(right, sp);
                r = 0;
            } else if (m_isNormal(r)) {
                // 右辺は数値
                if (r > 0)
                    --r;
                // else デフォルト値使用
            } else {
                HMD_DEBUG_ASSERT(m_isMulti(r));
                // 右辺は*
                m_onwrite(right, rnp); // m_sigs書き込み可にする
                switch (m_expand(sp)) {
                case 1:
                    // 展開できた
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
                    // 展開データが尽きた
                    // デフォルト値使用
                    *right = SEP;
                    r = 0;
                    break;
                default:
                    // 展開不能　
                    return false;
                }
            }
                
            l = 0;

        } else {
            HMD_ASSERT(m_isNestnum(l));
            //########## 左辺はネスティング ##########
            if (m_isSep(r)) {    // 区切りなので次をチェック
                r = 0;          
                continue;
            }
            else if (m_isNestnum(r)) {
                HMD_ASSERT(m_isNestnum(r));
                // 右辺もネスティング
                if (! m_canSubst(left, right, sp, defValClass, defValOffs))
                    return false;
                l = r = 0;
                continue;
            }
            else if (m_isMulti(r)) {
                // 右辺は *
                m_onwrite(right, rnp); // m_sigs書き込み可にする
                switch (m_expand(sp)) {
                case 1:
                    // 展開できた
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
                    // 展開データが尽きた
                    *right = SEP;
                    return false;
                default:
                    // 展開不能　
                    return false;
                }
            }

            // 右辺は数値
            HMD_DEBUG_ASSERT(m_isNormal(r));
            if (r == 0) // 右辺は 0 → 失敗
                return false;

            if (! m_isExpandable(m_context->stack.getAt(sp)))
                return false;   // 1 を (*) に展開できない

            m_onwrite(right, rnp); // m_sigs書き込み可にする
            if (r == 1) {
                Sig_t rr = *right;
                if (rr > 1) {
                    // n → n-1,(*)
                    m_insert(right, 2, rnp);
                    right[0] = rr - 1;
                    right[1] = m_nestnum2sig(1);
                    right[2] = MULTI;
                    ++right; ++*rnp;
                } else {    // rr == 1
                    // 1 → (*)
                    m_insert(right, 1, rnp);
                    right[0] = m_nestnum2sig(1);
                    right[1] = MULTI;
                }
            }
            else {  // r > 1
                Sig_t d = *right - r;
                if (d == 0) {
                    // n → (*),n-1
                    ++rn; ++*rnp;
                    m_insert(right, 2, rnp);
                } else {
                    // n → d,(*),r-1    where d + r == n
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


// スタックを左辺Signatureに沿って整理する
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
            //========== ffiの可変長引数 ==========
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
            //========== もう左辺無し ==========
            HMD_DEBUG_ASSERT(r == 0 || m_isSep(r));
            while (--rn >= 0) {
                ++right;
                HMD_DEBUG_ASSERT(*right == 0 || m_isSep(*right));
            }
            return;

        } else if (m_isNormal(l)) {
            //========== 左辺は数値 ==========
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
            //========== 左辺は * ==========

            if (m_isMultiDefaultVal(l)) {
                // 左辺はデフォルト値付き *
                if (m_isMulti(r)) {
                    // 右辺も *
                    switch (m_expand(sp)) {
                    case 1:
                        // 値があるので後半の処理に続く
                        r = 1;
                        ++rn;
                        --right;
                        break;
                    case 0:
                        // 空だったのでデフォルト値代入へ
                        r = SEP;
                        break;
                    }
                }

                if (r == 0 || m_isSep(r)) {
                    // デフォルト値代入
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
                l = r = 0;      // 右辺も *
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
            //========== 左辺はデフォルト値 ==========
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
            //========== 左辺はネスト ==========
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
    m_sigs.reserve(0);           // 外部参照状態なら通常使用状態に移行
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

// 値が展開可能かどうか
// Listなら展開可能　
// Fiberなら展開可能　
// each()を持つオブジェクトなら展開可能　
// どれも当てはまらなければ展開不能　
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


// スタックのsp位置の値を展開
// (1)Listなら head と tail に展開する
// (2)Fiberならgenerate()する
// (3)Fiberでなければ、each()を呼んでFiberを生成して、generate()する
// 展開できたら、spにheadが入り, sp+1にtailが挿入され、 1 を返す
// 展開して値が尽きていたら、spの値を削除し、 0 を返す
// 展開ができない値だったら、スタックは変化させずに  -1 を返す
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
            // generateできた
            stack.insertAt(sp, 1);
            stack.setAt(sp, retval);
            stack.setAt(sp+1, v);
            return 1;
        }
    }
    // generateするものがもう無かった
    stack.removeAt(sp);
    return 0;
}

// ネスティング終了までスキップ
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
