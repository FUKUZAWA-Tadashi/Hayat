/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#include "hayat.h"
#include <cppunit/extensions/HelperMacros.h>


using namespace Hayat::Common;
using namespace Hayat::Engine;


class Test_hyHash : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(Test_hyHash);
    CPPUNIT_TEST(test_hash);
    CPPUNIT_TEST_SUITE_END();
public:

    void* hayatMemory;

    void setUp(void)
    {
        hayatMemory = HMD_ALLOC(100000);
        initMemory(hayatMemory, 100000);
        initStdlib();
    }

    void tearDown(void)
    {
        finalizeAll();
    }


    void test_hash(void)
    {
        Context* context = gCodeManager.createContext();

        Hash* hash = new Hash(7, 60);

        CPPUNIT_ASSERT_EQUAL(7, (int)hash->capacity());
        CPPUNIT_ASSERT_EQUAL(0, hash->size());

        hash->put(context, Value::fromInt(10), Value::fromInt(20));
        CPPUNIT_ASSERT_EQUAL(1, hash->size());
        hash->put(context, Value::fromInt(20), Value::fromInt(30));
        CPPUNIT_ASSERT_EQUAL(2, hash->size());
        hash->put(context, Value::fromInt(17), Value::fromInt(40));
        CPPUNIT_ASSERT_EQUAL(3, hash->size());
        hash->put(context, Value::fromFloat(1.0f), Value::fromInt(50));
        CPPUNIT_ASSERT_EQUAL(4, hash->size());
        hash->put(context, Value::fromFloat(3.0f), Value::fromFloat(-2.0f));
        CPPUNIT_ASSERT_EQUAL(5, hash->size());

        Value v = hash->get(context, Value::fromInt(20));
        CPPUNIT_ASSERT_EQUAL(30, v.toInt());
        v = hash->get(context, Value::fromInt(17));
        CPPUNIT_ASSERT_EQUAL(40, v.toInt());

        hash->put(context, Value::fromInt(20), Value::fromInt(55));
        hash->put(context, Value::fromFloat(1.0f), TRUE_VALUE);
        v = hash->get(context, Value::fromInt(20));
        CPPUNIT_ASSERT_EQUAL(55, v.toInt());
        v = hash->get(context, Value::fromFloat(1.0f));
        CPPUNIT_ASSERT_EQUAL(true, v.toBool());

        CPPUNIT_ASSERT_EQUAL(5, hash->size());

        hash->rehash(context);
        CPPUNIT_ASSERT_EQUAL(5, hash->size());

        // {10=>20,20=>55,17=>40,1.0=>true,3.0=>-2.0}

        ValueArray* arr = hash->keys();
        CPPUNIT_ASSERT_EQUAL((size_t)5, arr->size());
        int chk = 0;
        for (int i = 0; i < 5; i++) {
            Value v = arr->nth(i);
            if (v == Value::fromInt(10))
                chk += 1;
            else if (v == Value::fromInt(20))
                chk += 2;
            else if (v == Value::fromInt(17))
                chk += 4;
            else if (v == Value::fromFloat(1.0f))
                chk += 8;
            else if (v == Value::fromFloat(3.0f))
                chk += 16;
            else {
                HMD_PRINTF("&%d&\n",v.data);
                chk = 99999;
            }
        }
        CPPUNIT_ASSERT_EQUAL(31, chk);

        arr = hash->values();
        CPPUNIT_ASSERT_EQUAL((size_t)5, arr->size());
        chk = 0;
        for (int i = 0; i < 5; i++) {
            Value v = arr->nth(i);
            if (v == Value::fromInt(20))
                chk += 1;
            else if (v == Value::fromInt(55))
                chk += 2;
            else if (v == Value::fromInt(40))
                chk += 4;
            else if (v == TRUE_VALUE)
                chk += 8;
            else if (v == Value::fromFloat(-2.0f))
                chk += 16;
            else {
                HMD_PRINTF("&%d&\n",v.data);
                chk = 99999;
            }
        }
        CPPUNIT_ASSERT_EQUAL(31, chk);



        hyu32 prevCapacity = hash->capacity();
        Object* obj = hash->getObj();
        for (int i = 100; i < 120; i++) {
            obj->cppObj<Hash>()->put(context, Value::fromInt(i), Value::fromFloat((hyf32)i + 0.5f));
        }
        // capaオーバーでrehash
        CPPUNIT_ASSERT(hash->capacity() > prevCapacity);
        CPPUNIT_ASSERT(hash->m_load < hash->m_bucketSize * hash->m_loadFactor);
        
        v = hash->get(context, Value::fromFloat(3.0f));
        CPPUNIT_ASSERT_EQUAL(-2.0f, v.toFloat());
        for (int i = 100; i < 120; i++) {
            v = hash->get(context, Value::fromInt(i));
            CPPUNIT_ASSERT_EQUAL((hyf32)i + 0.5f, v.toFloat());
        }


        gCodeManager.releaseContext(context);
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_hyHash);
