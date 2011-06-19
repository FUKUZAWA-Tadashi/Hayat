// -*- coding: utf-8 -*-
require "hyUnit"

class TestObject1
	def initialize (x) { setX(x) }
	def setX (x) { @x = x }
end

class TestObject2 < TestObject1
	def initialize (x, y) {
		setX(x)
		setY(y)
	}
	def setY (y) { @y = y }
end

class BadInitialize
	def initialize() {
		// メンバ変数もC++オブジェクトも持たないクラスは、initialize()が
		// このクラスのインスタンスを返さなければならない
		1						// しかし Int を返したので例外発生
	}
end

class HyTest_object < HyUnit

	def hytest_object ()
	{
		test_assert_equal(:Int, 1.classSymbol())
		test_assert_equal(Int, 1.getClass())
		test_assert_equal(:Int, Int.classSymbol())
		test_assert_equal(:Float, 1.2.classSymbol())
		test_assert_equal(:Bool, false.classSymbol())

		sb = new StringBuffer()
		1.concatToStringBuffer(sb)
		test_assert_equal("1", sb)

		o1 = new TestObject2(1,10)
		o2 = new TestObject2(1,10)
		o3 = new TestObject2(2,10)
		o4 = new TestObject2(1,20)

		test_assert(o1 == o1)
		test_assert(o1 == o2)
		test_assert(! o1.sameObject?(o2))
		test_assert(o1 <> o3)
		test_assert(o1 <> o4)
		o3.setX(1)
		test_assert(o1 == o3)
		test_assert(! o1.sameObject?(o3))
		o4.setY(10)
		test_assert(o1 == o4)
		test_assert(! o1.sameObject?(o4))
	}

	def hytest_constructor ()
	{
		test_assert_throw_type(:bad_constructor){ new BadInitialize() }
		test_assert_equal(0, new Int())
		test_assert_equal(false, new Bool())
	}

	def hytest_not ()
	{
		test_assert_equal(false, ! 1)
		test_assert_equal(false, ! [])
		test_assert_equal(false, ! '())
		test_assert_equal(true, ! nil)
		test_assert_equal(true, ! false)
	}
end
