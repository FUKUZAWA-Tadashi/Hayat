/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hyC_opcode.h"
#include "hyCStrTable.h"
#include "hyMemPool.h"
#include "hyEndian.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace Hayat::Common;
using namespace Hayat::Compiler;

class Test_hyCOpcode : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyCOpcode);
    CPPUNIT_TEST(test_addCode);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(20480);
        MemPool::initGMemPool(hayatMemory, 20480);
    }

    void tearDown(void)
    {
        HMD_FREE(hayatMemory);
    }

    void test_addCode(void)
    {
        static const size_t SS = sizeof(SymbolID_t);
        {
            Bytecode b;
            OP_nop::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_nop, b.getCode(0));
        }
        {
            Bytecode b;
            OP_push_nil::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_nil, b.getCode(0));
        }
        {
            Bytecode b;
            OP_push_self::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_self, b.getCode(0));
        }
        {
            Bytecode b;
            OP_push_i0::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_i0, b.getCode(0));
        }
        {
            Bytecode b;
            OP_push_i1::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_i1, b.getCode(0));
        }
        {
            Bytecode b;
            OP_push_im1::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_im1, b.getCode(0));
        }
        {
            Bytecode b;
            OP_push_i8::addCodeTo(b, OPR_INT8(23));
            CPPUNIT_ASSERT_EQUAL((hyu32)2, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_i8, *p);
            CPPUNIT_ASSERT_EQUAL((hyu8)23, *(p+1));
        }
        {
            Bytecode b;
            OP_push_i::addCodeTo(b, OPR_INT(24680));
            CPPUNIT_ASSERT_EQUAL((hyu32)5, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_i, *p);
            CPPUNIT_ASSERT_EQUAL((hys32)24680, Endian::unpack<hys32>(p+1));
        }
        {
            Bytecode b;
            OP_push_f0::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_f0, b.getCode(0));
        }
        {
            Bytecode b;
            OP_push_f1::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_f1, b.getCode(0));
        }
        {
            Bytecode b;
            OP_push_fm1::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_fm1, b.getCode(0));
        }
        {
            Bytecode b;
            OP_push_f::addCodeTo(b, OPR_FLOAT(246.25f));
            CPPUNIT_ASSERT_EQUAL((hyu32)5, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_f, *p);
            CPPUNIT_ASSERT_EQUAL((hyf32)246.25f, Endian::unpack<hyf32>(p+1));
        }
        {
            Bytecode b;
            OP_push_strconst::addCodeTo(b, (hyu32)12345);
            CPPUNIT_ASSERT_EQUAL((hyu32)5, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_strconst, *p);
            CPPUNIT_ASSERT_EQUAL((hyu32)12345, Endian::unpack<hyu32>(p+1));
        }
        {
            Bytecode b;
            OP_push_symbol::addCodeTo(b, OPR_SYMBOL((SymbolID_t)123));
            CPPUNIT_ASSERT_EQUAL((hyu32)(1+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_symbol, *p);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)123, Endian::unpack<SymbolID_t>(p+1));
        }
        {
            Bytecode b;
            OP_push_true::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_true, b.getCode(0));
        }
        {
            Bytecode b;
            OP_push_false::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_push_false, b.getCode(0));
        }
        {
            Bytecode b;
            OP_substArray::addCodeTo(b, OPR_UINT8(255));
            CPPUNIT_ASSERT_EQUAL((hyu32)2, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_substArray, *p);
            CPPUNIT_ASSERT_EQUAL((hyu8)255, *(p+1));
        }
        {
            Bytecode b;
            OP_substHash::addCodeTo(b, OPR_UINT8(21));
            CPPUNIT_ASSERT_EQUAL((hyu32)2, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_substHash, *p);
            CPPUNIT_ASSERT_EQUAL((hyu8)21, *(p+1));
        }
        {
            Bytecode b;
            OP_pushEmptyList::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_pushEmptyList, b.getCode(0));
        }
        {
            Bytecode b;
            OP_cons::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_cons, b.getCode(0));
        }
        {
            Bytecode b;
            OP_pop::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_pop, b.getCode(0));
        }
        {
            Bytecode b;
            OP_drop_2nd::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_drop_2nd, b.getCode(0));
        }
        {
            Bytecode b;
            OP_insMethod_0::addCodeTo(b, OPR_METHOD((SymbolID_t)12345));
            CPPUNIT_ASSERT_EQUAL((hyu32)(1+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_insMethod_0, *p);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)12345, Endian::unpack<SymbolID_t>(p+1));
        }
        {
            Bytecode b;
            OP_insMethod_1::addCodeTo(b, OPR_METHOD((SymbolID_t)1234));
            CPPUNIT_ASSERT_EQUAL((hyu32)(1+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_insMethod_1, *p);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)1234, Endian::unpack<SymbolID_t>(p+1));
        }
        {
            Bytecode b;
            OP_insMethod_2::addCodeTo(b, OPR_METHOD((SymbolID_t)4321));
            CPPUNIT_ASSERT_EQUAL((hyu32)(1+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_insMethod_2, *p);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)4321, Endian::unpack<SymbolID_t>(p+1));
        }
        {
            Bytecode b;
            OP_insMethod::addCodeTo(b, OPR_SIGNATURE(99), OPR_METHOD((SymbolID_t)5432));
            CPPUNIT_ASSERT_EQUAL((hyu32)(3+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_insMethod, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)99, Endian::unpack<hyu16>(p+1));
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)5432, Endian::unpack<SymbolID_t>(p+3));
        }
        {
            Bytecode b;
            OP_method_0::addCodeTo(b, OPR_METHOD((SymbolID_t)6543));
            CPPUNIT_ASSERT_EQUAL((hyu32)(1+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_method_0, *p);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)6543, Endian::unpack<SymbolID_t>(p+1));
        }
        {
            Bytecode b;
            OP_method_1::addCodeTo(b, OPR_METHOD((SymbolID_t)7654));
            CPPUNIT_ASSERT_EQUAL((hyu32)(1+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_method_1, *p);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)7654, Endian::unpack<SymbolID_t>(p+1));
        }
        {
            Bytecode b;
            OP_method_2::addCodeTo(b, OPR_METHOD((SymbolID_t)8765));
            CPPUNIT_ASSERT_EQUAL((hyu32)(1+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_method_2, *p);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)8765, Endian::unpack<SymbolID_t>(p+1));
        }
        {
            Bytecode b;
            OP_method::addCodeTo(b, OPR_SIGNATURE(88), OPR_METHOD((SymbolID_t)9876));
            CPPUNIT_ASSERT_EQUAL((hyu32)(3+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_method, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)88, Endian::unpack<hyu16>(p+1));
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)9876, Endian::unpack<SymbolID_t>(p+3));
        }
        {
            Bytecode b;
            OP_scopeMethod::addCodeTo(b, OPR_SIGNATURE(66), OPR_METHOD((SymbolID_t)8765));
            CPPUNIT_ASSERT_EQUAL((hyu32)(3+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_scopeMethod, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)66, Endian::unpack<hyu16>(p+1));
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)8765, Endian::unpack<SymbolID_t>(p+3));
        }
        {
            Bytecode b;
            OP_new::addCodeTo(b, OPR_CLASS((SymbolID_t)7654), OPR_SIGNATURE(123));
            CPPUNIT_ASSERT_EQUAL((hyu32)(3+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_new, *p);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)7654, Endian::unpack<SymbolID_t>(p+1));
            CPPUNIT_ASSERT_EQUAL((hyu16)123, Endian::unpack<hyu16>(p+1+SS));
        }
        {
            Bytecode b;
            OP_getLocal::addCodeTo(b, OPR_LOCALVAR(13579));
            CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_getLocal, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)13579, Endian::unpack<hyu16>(p+1));
        }
        {
            Bytecode b;
            OP_setLocal::addCodeTo(b, OPR_LOCALVAR(13578));
            CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_setLocal, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)13578, Endian::unpack<hyu16>(p+1));
        }
        {
            Bytecode b;
            OP_getClassVar::addCodeTo(b, OPR_CLASSVAR(13577));
            CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_getClassVar, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)13577, Endian::unpack<hyu16>(p+1));
        }
        {
            Bytecode b;
            OP_setClassVar::addCodeTo(b, OPR_CLASSVAR(13576));
            CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_setClassVar, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)13576, Endian::unpack<hyu16>(p+1));
        }
        {
            Bytecode b;
            OP_getMembVar::addCodeTo(b, OPR_MEMBVAR(13575));
            CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_getMembVar, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)13575, Endian::unpack<hyu16>(p+1));
        }
        {
            Bytecode b;
            OP_setMembVar::addCodeTo(b, OPR_MEMBVAR(13574));
            CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_setMembVar, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)13574, Endian::unpack<hyu16>(p+1));
        }
        {
            Bytecode b;
            OP_getGlobal::addCodeTo(b, OPR_GLOBALVAR(13573));
            CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_getGlobal, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)13573, Endian::unpack<hyu16>(p+1));
        }
        {
            Bytecode b;
            OP_setGlobal::addCodeTo(b, OPR_GLOBALVAR(13572));
            CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_setGlobal, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)13572, Endian::unpack<hyu16>(p+1));
        }
        {
            Bytecode b;
            OP_getClass::addCodeTo(b, OPR_CLASS((SymbolID_t)5432));
            CPPUNIT_ASSERT_EQUAL((hyu32)(1+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_getClass, *p);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)5432, Endian::unpack<SymbolID_t>(p+1));
        }
        {
            Bytecode b;
            OP_getConstVar::addCodeTo(b, OPR_CONSTVAR(13571));
            CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_getConstVar, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)13571, Endian::unpack<hyu16>(p+1));
        }
        {
            Bytecode b;
            OP_setConstVar::addCodeTo(b, OPR_CONSTVAR(13570));
            CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_setConstVar, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)13570, Endian::unpack<hyu16>(p+1));
        }
        {
            Bytecode b;
            OP_return::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_return, b.getCode(0));
        }
        {
            Bytecode b;
            OP_jump::addCodeTo(b, OPR_RELATIVE(-32105));
            CPPUNIT_ASSERT_EQUAL((hyu32)5, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_jump, *p);
            CPPUNIT_ASSERT_EQUAL((hys32)-32105, Endian::unpack<hys32>(p+1));
        }
        {
            Bytecode b;
            OP_jump_ifFalse::addCodeTo(b, OPR_RELATIVE(-32109));
            CPPUNIT_ASSERT_EQUAL((hyu32)5, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_jump_ifFalse, *p);
            CPPUNIT_ASSERT_EQUAL((hys32)-32109, Endian::unpack<hys32>(p+1));
        }
        {
            Bytecode b;
            OP_jumpControl::addCodeTo(b, OPR_JUMPSYMBOL((SymbolID_t)12457));
            CPPUNIT_ASSERT_EQUAL((hyu32)5, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_jumpControl, *p);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)12457, Endian::unpack<SymbolID_t>(p+1));
        }
        {
            Bytecode b;
            OP_classInit::addCodeTo(b, OPR_CLASS((SymbolID_t)12458));
            CPPUNIT_ASSERT_EQUAL((hyu32)(1+SS), b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_classInit, *p);
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)12458, Endian::unpack<hyu16>(p+1));
        }
        {
            Bytecode b;
            OP_waitTick_1::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_waitTick_1, b.getCode(0));
        }
        {
            Bytecode b;
            OP_waitTicks::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_waitTicks, b.getCode(0));
        }
        {
            Bytecode b;
            TArray<SymbolID_t> arr;
            arr.add((SymbolID_t)321);
            arr.add((SymbolID_t)654);
            OP_getScopeClass::addCodeTo(b, OPR_SCOPE(&arr));
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_getScopeClass, *p);
            CPPUNIT_ASSERT_EQUAL((hyu8)2, *(p+1));
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)321, Endian::unpack<SymbolID_t>(p+2));
            CPPUNIT_ASSERT_EQUAL((SymbolID_t)654, Endian::unpack<SymbolID_t>(p+2+SS));
            // CPPUNIT_ASSERT_EQUAL((hyu32)321, Endian::unpack<hyu32>(p+1));
        }
        {
            Bytecode b;
            OP_getClassConst::addCodeTo(b, OPR_CONSTVAR(4673));
            CPPUNIT_ASSERT_EQUAL((hyu32)3, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_getClassConst, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)4673, Endian::unpack<hyu16>(p+1));
        }
        {
            Bytecode b;
            OP_jumpControlStart::addCodeTo(b, OPR_MAYLOCALVAR(-1234), OPR_MAYLOCALVAR(-2345), OPR_UINT8(101));
            CPPUNIT_ASSERT_EQUAL((hyu32)6, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_jumpControlStart, *p);
            CPPUNIT_ASSERT_EQUAL((hys16)-1234, Endian::unpack<hys16>(p+1));
            CPPUNIT_ASSERT_EQUAL((hys16)-2345, Endian::unpack<hys16>(p+3));
            CPPUNIT_ASSERT_EQUAL((hyu8)101, *(p+5));
        }
        {
            Bytecode b;
            OP_jumpControlEnd::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_jumpControlEnd, b.getCode(0));
        }
        {
            Bytecode b;
            OP_throw::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_throw, b.getCode(0));
        }
        {
            Bytecode b;
            OP_closure::addCodeTo(b, OPR_UINT16(2469), OPR_UINT8(103));
            CPPUNIT_ASSERT_EQUAL((hyu32)4, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_closure, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)2469, Endian::unpack<hyu16>(p+1));
            CPPUNIT_ASSERT_EQUAL((hyu8)103, *(p+3));
        }
        {
            Bytecode b;
            OP_shareLocal::addCodeTo(b, OPR_UINT16(2470), OPR_UINT16(12471));
            CPPUNIT_ASSERT_EQUAL((hyu32)5, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_shareLocal, *p);
            CPPUNIT_ASSERT_EQUAL((hyu16)2470, Endian::unpack<hyu16>(p+1));
            CPPUNIT_ASSERT_EQUAL((hyu16)12471, Endian::unpack<hyu16>(p+3));
        }
        {
            Bytecode b;
            OP_yield::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            const hyu8* p = b.getCodes(0);
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_yield, *p);
        }
        {
            Bytecode b;
            OP_beFiber::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_beFiber, b.getCode(0));
        }
        {
            Bytecode b;
            OP_copy_top_2nd::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_copy_top_2nd, b.getCode(0));
        }
        {
            Bytecode b;
            OP_swap::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_swap, b.getCode(0));
        }
        {
            Bytecode b;
            OP_rotate_r::addCodeTo(b);
            CPPUNIT_ASSERT_EQUAL((hyu32)1, b.getSize());
            CPPUNIT_ASSERT_EQUAL((hyu8)OPL_rotate_r, b.getCode(0));
        }
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyCOpcode);
