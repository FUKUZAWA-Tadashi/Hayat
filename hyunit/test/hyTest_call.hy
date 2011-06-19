// -*- coding: utf-8 -*-

require "hyUnit"


class CallTest
	@@x = 100
	def initialize (a = 1) {	// constructor
		@a = a
	}
	def xa_add (x = 10)			// instance method
	{
		@a += x
		@@x += x
	}
	def x_add (x = 100)			// class method
	{
		@@x += x
	}

	def get_a ()
	{
		return @a
	}
	def get_x ()
	{
		return @@x
	}
end


class HyTest_call < HyUnit

	def hytest_call ()
	{
		t = new CallTest(10)
		test_assert_equal(10, t.get_a())
		test_assert_equal(100, t.get_x())
		test_assert_equal(100, CallTest.get_x())
		t.xa_add(5)
		test_assert_equal(15, t.get_a())
		test_assert_equal(105, t.get_x())
		test_assert_equal(105, CallTest.get_x())
		CallTest.x_add(20)
		test_assert_equal(15, t.a)
		test_assert_equal(125, t.get_x())
		test_assert_equal(125, CallTest.get_x())
	}

end
