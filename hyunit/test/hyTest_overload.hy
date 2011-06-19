require "hyUnit"

class HyTest_overload < HyUnit

	@@a = nil

	def f(a) { @@a = a }
	def f(a,b) { @@a = a + b }
	def f(a,b,c) { @@a = a * b - c }
	def hytest_overload1 ()
	{
		f(1)
		test_assert_equal(1, @@a)
		f(2,3)
		test_assert_equal(5, @@a)
		f(4,5,6)
		test_assert_equal(14, @@a)
	}


	def g('()) { 0 }
	def g('(a,*b)) { a + g(b) }
	def hytest_overload2 ()
	{
		test_assert_equal(0, g('()))
		test_assert_equal(6, g('(1,2,3)))
		test_assert_equal(45, g(10.times()))
	}


	def h() { 1 }
	def h(a,*b) { 1 + a + h(*b) }
	def hytest_overload3 ()
	{
		test_assert_equal(1, h())
		test_assert_equal(12+ 3+1, h(*[3,4,5]))
		test_assert_equal(0+1+2+3+ 4+1, h(*4.times()))
	}


	def func1 (fn, '())
	{
		'()
	}
	def func1 (fn, '(x,*y))
	{
		List.cons(fn(x), func1(fn,y))
	}

	def func2 (fn, x,*y)
	{
		List.cons(fn(x), func2(fn,*y))
	}
	def func2 (fn)
	{
		'()
	}


	def hytest_overload4 ()
	{
		a = '(1,2,3)
		fn = func1(lambda(x){x * 2}, a)
		test_assert_equal('(2,4,6), fn)
		fn = func2(lambda(x){x * 10}, *a)
		test_assert_equal('(10,20,30), fn)
	}
	
end
