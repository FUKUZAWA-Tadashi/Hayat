/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyCSignature.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Compiler;

class Test_CSignature : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_CSignature);
    CPPUNIT_TEST(test_addSig);
    CPPUNIT_TEST(test_canSubst);
    CPPUNIT_TEST(test_nest);
    CPPUNIT_TEST(test_canSubstNest);
    CPPUNIT_TEST(test_arity);
    CPPUNIT_TEST_SUITE_END();
public:
    Test_CSignature(void) {}

    void* hayatMemory;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(10240);
        MemPool::initGMemPool(hayatMemory, 10240);
    }

    void tearDown(void)
    {
        HMD_FREE(hayatMemory);
    }

    void test_addSig(void)
    {
        Signature s;
        CPPUNIT_ASSERT_EQUAL((hyu32)2, s.m_sigs.size());
        CPPUNIT_ASSERT_EQUAL((Signature::Sig_t)0, s.m_sigs[1]);
        CPPUNIT_ASSERT_EQUAL(1, s.m_getNestnum());
        s.m_clear();
        CPPUNIT_ASSERT_EQUAL((hyu32)2, s.m_sigs.size());
        CPPUNIT_ASSERT_EQUAL((Signature::Sig_t)0, s.m_sigs[1]);
        CPPUNIT_ASSERT_EQUAL(1, s.m_getNestnum());

        // '(a,b,c) = (3)
        s.addSig(1); s.addSig(1); s.addSig(1);
        CPPUNIT_ASSERT_EQUAL((hyu32)2, s.m_sigs.size());
        CPPUNIT_ASSERT_EQUAL(1, s.m_getNestnum());
        CPPUNIT_ASSERT_EQUAL((Signature::Sig_t)3, s.m_sigs[1]);

        // '(a,*b) = (1,*)
        s.m_clear();
        s.addSig(1); s.addSig(Signature::MULTI);
        CPPUNIT_ASSERT_EQUAL((hyu32)3, s.m_sigs.size());
        CPPUNIT_ASSERT_EQUAL(2, s.m_getNestnum());
        CPPUNIT_ASSERT_EQUAL((Signature::Sig_t)1, s.m_sigs[1]);
        CPPUNIT_ASSERT_EQUAL(Signature::MULTI, s.m_sigs[2]);

        // '(*a,b) → エラー
        s.m_clear();
        s.addSig(Signature::MULTI);
        CPPUNIT_ASSERT_THROW(s.addSig(1), Signature::IllegalArgException);

        // '(a,*b,*c) → エラー
        s.m_clear();
        s.addSig(1); s.addSig(Signature::MULTI);
        CPPUNIT_ASSERT_THROW(s.addSig(Signature::MULTI), Signature::IllegalArgException);

        // '(a,b=?,*c) → (1,$,*)
        s.m_clear();
        s.addSig(1); s.addSig(Signature::defaultValIdx2sig(0));
        s.addSig(Signature::MULTI);
        CPPUNIT_ASSERT_EQUAL(3, s.m_getNestnum());
        CPPUNIT_ASSERT_EQUAL((Signature::Sig_t)1, s.m_sigs[1]);
        CPPUNIT_ASSERT_EQUAL(0, Signature::m_sig2DefaultValIdx(s.m_sigs[2]));
        CPPUNIT_ASSERT_EQUAL(Signature::MULTI, s.m_sigs[3]);

        // '(a,b=?,*c=?) → (1,$,$*)
        s.m_clear();
        s.addSig(1); s.addSig(Signature::defaultValIdx2sig(1));
        s.addSig(Signature::defaultValIdx2multisig(2));
        CPPUNIT_ASSERT_EQUAL(3, s.m_getNestnum());
        CPPUNIT_ASSERT_EQUAL((Signature::Sig_t)1, s.m_sigs[1]);
        CPPUNIT_ASSERT_EQUAL(1, Signature::m_sig2DefaultValIdx(s.m_sigs[2]));
        CPPUNIT_ASSERT_EQUAL(2, Signature::m_sig2DefaultValIdx(s.m_sigs[3]));


        // '(a,b=?,c) → エラー
        s.m_clear();
        s.addSig(1); s.addSig(Signature::defaultValIdx2sig(1));
        CPPUNIT_ASSERT_THROW(s.addSig(1), Signature::IllegalArgException);

        // '(a=?,b=?,c) → エラー
        s.m_clear();
        s.addSig(Signature::defaultValIdx2sig(3));
        s.addSig(Signature::defaultValIdx2sig(4));
        CPPUNIT_ASSERT_THROW(s.addSig(1), Signature::IllegalArgException);

        // '(a=?,b=?,*c) → ($,$,*)
        s.m_clear();
        s.addSig(Signature::defaultValIdx2sig(5));
        s.addSig(Signature::defaultValIdx2sig(6));
        s.addSig(Signature::MULTI);
        CPPUNIT_ASSERT_EQUAL(3, s.m_getNestnum());
        CPPUNIT_ASSERT(Signature::m_isDefaultVal(s.m_sigs[1]));
        CPPUNIT_ASSERT_EQUAL(5, Signature::m_sig2DefaultValIdx(s.m_sigs[1]));
        CPPUNIT_ASSERT(Signature::m_isDefaultVal(s.m_sigs[2]));
        CPPUNIT_ASSERT_EQUAL(6, Signature::m_sig2DefaultValIdx(s.m_sigs[2]));
        CPPUNIT_ASSERT_EQUAL(Signature::MULTI, s.m_sigs[3]);

        // '(a=?,b=?,*c=?) → ($,$,$*)
        s.m_clear();
        s.addSig(Signature::defaultValIdx2sig(7));
        s.addSig(Signature::defaultValIdx2sig(8));
        s.addSig(Signature::defaultValIdx2multisig(9));
        CPPUNIT_ASSERT_EQUAL(3, s.m_getNestnum());
        CPPUNIT_ASSERT(Signature::m_isDefaultVal(s.m_sigs[1]));
        CPPUNIT_ASSERT_EQUAL(7, Signature::m_sig2DefaultValIdx(s.m_sigs[1]));
        CPPUNIT_ASSERT(Signature::m_isDefaultVal(s.m_sigs[2]));
        CPPUNIT_ASSERT_EQUAL(8, Signature::m_sig2DefaultValIdx(s.m_sigs[2]));
        CPPUNIT_ASSERT(Signature::m_isMultiDefaultVal(s.m_sigs[3]));
        CPPUNIT_ASSERT_EQUAL(9, Signature::m_sig2DefaultValIdx(s.m_sigs[3]));
    }
    
    void test_canSubst(void)
    {
        Signature l, r;
        // '(a) = '()
        l.addSig(1);
        CPPUNIT_ASSERT(! l.canSubst(r));
        // '(*a) = '()
        l.m_clear(); l.addSig(Signature::MULTI);
        CPPUNIT_ASSERT(l.canSubst(r));
        // '(a, *b) = '()
        l.m_clear(); l.addSig(1); l.addSig(Signature::MULTI);
        CPPUNIT_ASSERT(! l.canSubst(r));
        // '(a, *b) = '(x)
        r.addSig(1);
        CPPUNIT_ASSERT(l.canSubst(r));
        // '(a, *b) = '(*x)
        r.m_clear(), r.addSig(Signature::MULTI);
        CPPUNIT_ASSERT(l.canSubst(r));
        // '(a, *b) = '(x, *y)
        r.m_clear(), r.addSig(1); r.addSig(Signature::MULTI);
        CPPUNIT_ASSERT(l.canSubst(r));
        // '(a,b,c) = '(x,y)
        l.m_clear(); l.addSig(3);
        r.m_clear(); r.addSig(2);
        CPPUNIT_ASSERT(! l.canSubst(r));
        // '(a,b,c) = '(x,y,z)
        r.m_clear(); r.addSig(3);
        CPPUNIT_ASSERT(l.canSubst(r));
        // '(a,b,c) = '(*x)
        r.m_clear(); r.addSig(Signature::MULTI);
        CPPUNIT_ASSERT(l.canSubst(r));
        // '(a,b,c) = '(x,y,*z)
        r.m_clear(); r.addSig(2); r.addSig(Signature::MULTI);
        CPPUNIT_ASSERT(l.canSubst(r));
        // '(a,b,c) = '(x,y,z,*w)
        r.m_clear(); r.addSig(3); r.addSig(Signature::MULTI);
        CPPUNIT_ASSERT(l.canSubst(r));
        // '(a,b,c) = '(x,y,z,w,*v)
        r.m_clear(); r.addSig(4); r.addSig(Signature::MULTI);
        CPPUNIT_ASSERT(! l.canSubst(r));
    }

    void test_nest(void)
    {
        Signature s;

        s.addSig(1);
        s.addSig(Signature::defaultValIdx2sig(2));      // (1,$)
        // デフォルト値の後の openSub はエラー
        CPPUNIT_ASSERT_THROW(s.openSub(), Signature::IllegalArgException);

        s.m_clear();
        s.addSig(1);            // (1)
        Signature* sub = s.openSub();
        sub->addSig(1);
        Signature* p = sub->close();    // (1,(1))
        CPPUNIT_ASSERT_EQUAL(&s, p);
        CPPUNIT_ASSERT_EQUAL(2, s.m_getNestnum());
        CPPUNIT_ASSERT_EQUAL((hyu32)4, s.m_sigs.size());
        CPPUNIT_ASSERT_EQUAL((Signature::Sig_t)1, s.m_sigs[1]);
        CPPUNIT_ASSERT_EQUAL(Signature::m_nestnum2sig(1), s.m_sigs[2]);
        CPPUNIT_ASSERT_EQUAL((Signature::Sig_t)1, s.m_sigs[3]);
        s.addSig(1); s.addSig(Signature::MULTI); // (1,(1),1,*)
        // * の後の openSub はエラー
        CPPUNIT_ASSERT_THROW(s.openSub(), Signature::IllegalArgException);


        s.m_clear();
        s.addSig(2);    // (2)
        sub = s.openSub();
        sub->addSig(2);
        sub->addSig(Signature::defaultValIdx2sig(3));
        sub->addSig(Signature::MULTI);
        sub->close();   // (2,'(2,$,*))
        sub = s.openSub();
        sub->addSig(3);
        sub->addSig(Signature::defaultValIdx2sig(5));
        sub->addSig(Signature::defaultValIdx2multisig(7));
        p = sub->close();   // (2,'(2,$,*),'(3,$,$*))
        CPPUNIT_ASSERT_EQUAL(&s, p);
        CPPUNIT_ASSERT_EQUAL(3, s.m_getNestnum());
        CPPUNIT_ASSERT_EQUAL((hyu32)10, s.m_sigs.size());
        CPPUNIT_ASSERT_EQUAL((Signature::Sig_t)2, s.m_sigs[1]);
        CPPUNIT_ASSERT_EQUAL(Signature::m_nestnum2sig(3), s.m_sigs[2]);
        CPPUNIT_ASSERT_EQUAL((Signature::Sig_t)2, s.m_sigs[3]);
        CPPUNIT_ASSERT_EQUAL(Signature::defaultValIdx2sig(3), s.m_sigs[4]);
        CPPUNIT_ASSERT_EQUAL(Signature::MULTI, s.m_sigs[5]);
        CPPUNIT_ASSERT_EQUAL(Signature::m_nestnum2sig(3), s.m_sigs[6]);
        CPPUNIT_ASSERT_EQUAL((Signature::Sig_t)3, s.m_sigs[7]);
        CPPUNIT_ASSERT_EQUAL(Signature::defaultValIdx2sig(5), s.m_sigs[8]);
        CPPUNIT_ASSERT_EQUAL(Signature::defaultValIdx2multisig(7), s.m_sigs[9]);
    }

    void test_canSubstNest(void)
    {
        Signature l, r;
        Signature* sub;
        // '(a,*b) = x
        sub = l.openSub(); sub->addSig(1); sub->addSig(Signature::MULTI); sub->close();
        r.addSig(1);
        CPPUNIT_ASSERT(l.canSubst(r));
        // '(a,*b) = '(*x)
        r.m_clear(); sub = r.openSub(); sub->addSig(Signature::MULTI); sub->close();
        CPPUNIT_ASSERT(l.canSubst(r));
        // '(a,*b) = '(x,*y)
        r.m_clear(); sub = r.openSub(); sub->addSig(1); sub->addSig(Signature::MULTI); sub->close();
        CPPUNIT_ASSERT(l.canSubst(r));
        // a = '(*x)
        l.m_clear(); l.addSig(1);
        r.m_clear(); sub = r.openSub(); sub->addSig(Signature::MULTI); sub->close();
        CPPUNIT_ASSERT(l.canSubst(r));
        // '(a) = '(x,y)
        l.m_clear(); sub = l.openSub(); sub->addSig(1); sub->close();
        r.m_clear(); sub = r.openSub(); sub->addSig(2); sub->close();
        CPPUNIT_ASSERT(! l.canSubst(r));
    }

    void test_arity(void)
    {
        Signature s;
        Signature* sub;
        // '(a,b,c)
        s.addSig(3);
        CPPUNIT_ASSERT_EQUAL((hyu8)3, Signature::arity(s.m_sigs.top()));
        // '()
        s.m_clear();
        CPPUNIT_ASSERT_EQUAL((hyu8)0, Signature::arity(s.m_sigs.top()));
        // '(a,*b)
        s.m_clear();
        s.addSig(1); s.addSig(Signature::MULTI);
        CPPUNIT_ASSERT_EQUAL((hyu8)2, Signature::arity(s.m_sigs.top()));
        // '(a,'(),b)
        s.m_clear();
        s.addSig(1);
        sub = s.openSub(); sub->close();
        s.addSig(1);
        CPPUNIT_ASSERT_EQUAL((hyu8)3, Signature::arity(s.m_sigs.top()));
        // '(a, '(b, *c), d=?, *e=?)
        s.m_clear();
        s.addSig(1);
        sub = s.openSub(); sub->addSig(1); sub->addSig(Signature::MULTI); sub->close();
        s.addSig(Signature::defaultValIdx2sig(3));
        s.addSig(Signature::defaultValIdx2multisig(5));
        CPPUNIT_ASSERT_EQUAL((hyu8)5, Signature::arity(s.m_sigs.top()));
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_CSignature);

