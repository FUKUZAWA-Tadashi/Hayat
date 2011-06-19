require "hyUnit"

class TestClassA
	def methodA (x) { x + 1 }
end
class TestClassB
	def methodB (x) { x + 2 }
end


class HyTest_method < HyUnit


	def hytest_functor ()
	{
		// インスタンスメソッド呼出し
		(new HyTest_method()).test_functor()
	}

	def test_functor()
	{
		// @fを使っているのでこれはインスタンスメソッドである

		f = new Method(1, :"+")
		test_assert_equal(2, f(1)) // このfはローカル変数であり、下の方に定義されているメソッドではない
		test_assert_equal(5, f(4))
		@f = lambda(x){10+x}
		test_assert_equal(20,@f(10))
		test_assert_equal(110,@f(100))
		@@f = 50.0.getMethod(:"/")
		test_assert_equal(25.0, @@f(2.0))
		test_assert_equal(10.0, @@f(5.0))
		x = []
		x.getMethod(:push)(10)
		test_assert_equal([10], x)
	}


	def hytest_haveMethod ()
	{
		test_assert(Int.haveMethod?(:"+"))
		test_assert(! Int.haveMethod?(:foobar))
		a = "asd"
		test_assert(a.haveMethod?(:length))
		test_assert(! a.haveMethod?(:asdqwe))
		test_assert(a.haveMethod?(:haveMethod?))
		test_assert(haveMethod?(:haveMethod?))
	}


	def hytest_method ()
	{
		x = 1
		m = new Method(x, :"+")
		test_assert_equal(2, m(1))
		test_assert_equal(11, m(10))
		test_assert_equal(-5, m(-6))

		x = []
		m = x.getMethod(:push)
		m(1)
		test_assert_equal([1], x)
		m(*[2])
		test_assert_equal([1,2], x)
		m(10)
		test_assert_equal([1,2,10], x)
		m(20)
		test_assert_equal([1,2,10,20], x)
	}

	def hytest_callIfMethodExist ()
	{
		test_assert_equal(11, TestClassA.callIfMethodExist(:methodA, 10))
		test_assert_equal(nil, TestClassA.callIfMethodExist(:methodB, 10))
		test_assert_equal(nil, TestClassB.callIfMethodExist(:methodA, 10))
		test_assert_equal(12, TestClassB.callIfMethodExist(:methodB, 10))
	}


	def f(a, b=10) { a + b }
	def hytest_defaultVal1 ()
	{
		test_assert_equal(5, f(2,3))
		test_assert_equal(11, f(1))
	}
	

	def g1(a, b=f(1,2), *c='(10,20)) {
		x = a + b
		c.each{|n| x += n}
		x
	}
	def hytest_defaultVal2 ()
	{
		test_assert_equal(10, g1(1,2,3,4))
		test_assert_equal(6, g1(1,2,3))
		test_assert_equal(33, g1(1,2))
		test_assert_equal(34, g1(1))

		test_assert_equal(14, g1(*[2,3,4,5]))
		test_assert_equal(9, g1(*[2,3,4]))
		test_assert_equal(35, g1(*[2,3]))
		test_assert_equal(35, g1(*[2]))
		test_assert_throw{g1(*[])}
	}

	def h1(a, b=10, *c) {
		x = a+b
		c.each{|n| x += n}
		x
	}
	def hytest_defaultVal3 ()
	{
		test_assert_equal(10, h1(1,2,3,4))
		test_assert_equal(6, h1(1,2,3))
		test_assert_equal(3, h1(1,2))
		test_assert_equal(11, h1(1))
		
		test_assert_equal(14, h1(*[2,3,4,5]))
		test_assert_equal(9, h1(*[2,3,4]))
		test_assert_equal(5, h1(*[2,3]))
		test_assert_equal(12, h1(*[2]))
		test_assert_throw{h1(*[])}
	}



	def i1
	{
		x = 0
		args_->{|y| x += y}
		x
	}

	def hytest_omit_param
	{
		test_assert_equal(10, i1(1,2,3,4))
		test_assert_equal(16, i1(1,3,5,7))
		test_assert_equal(0, i1())
		test_assert_equal(0, i1)
	}




	@@df1 = lambda(x){x+1}
	@@df2 = lambda(x,y = 2){x+y}
	m = 3
	@@df3 = lambda(x,y = m){x+y}
	def dt1 (i, func = lambda(x){x+10}) {
		func(i)
	}
	def dt2 (x, func = lambda(x){x+11}) {
		func(x)
	}
	def dt3 (i, func = lambda(x,y = 12){x+y}) {
		func(i)
	}
	def dt4 (i, func = lambda(x){x+m+10}) {
		func(i)
	}
	def dt5 (i, func = @@df1) {
		func(i)
	}
	def dt6 (i, f1 = lambda(x, f2 = @@df2) { f2(x) }) {
		f1(i)
	}
	def dt9 (i, f1 = lambda(x, f2 = lambda(y) {y+100}) { f2(x+1000) }) {
		f1(i)
	}
	
	
	def hytest_defaultVal4 ()
	{
		test_assert_equal(2, @@df1(1))
		test_assert_equal(4, @@df2(2))
		test_assert_equal(1, @@df2(2,-1))
		test_assert_equal(6, @@df3(3))
		test_assert_equal(2, @@df3(3,-1))
		test_assert_equal(11, dt1(1))
		test_assert_equal(21, dt1(1, lambda(x){x+20}))
		test_assert_equal(2, dt1(1, @@df1))
		test_assert_equal(22, dt2(11))
		test_assert_equal(12, dt2(10,@@df2))
		test_assert_equal(35, dt3(23))
		test_assert_equal(17, dt4(4))
		test_assert_equal(101, dt5(100))
		test_assert_equal(102, dt5(100, @@df2))
		test_assert_equal(12, dt6(10))
		test_assert_equal(1109, dt9(9))
	}


	def hytest_Bool_method ()
	{
		method = new Method(true, :"&&");
		test_assert_equal(true, method.call(true))
		test_assert_equal(false, method.call(false))
		test_assert_equal(true, method.call(1))
		test_assert_equal(false, method.call(nil))

		method = new Method(false, :"&&");
		test_assert_equal(false, method.call(true))
		test_assert_equal(false, method.call(false))
		test_assert_equal(false, method.call(1))
		test_assert_equal(false, method.call(nil))

		method = new Method(true, :"||");
		test_assert_equal(true, method.call(true))
		test_assert_equal(true, method.call(false))
		test_assert_equal(true, method.call(1))
		test_assert_equal(true, method.call(nil))

		method = new Method(false, :"||");
		test_assert_equal(true, method.call(true))
		test_assert_equal(false, method.call(false))
		test_assert_equal(true, method.call(1))
		test_assert_equal(false, method.call(nil))
	}

end
