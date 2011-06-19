require "hyUnit"

class HyTest_closure < HyUnit

	def test1 ()
	{
		a = 0
		b = 0
		fa = lambda(x) {
			a += x
		}
		fb = lambda(x) {
			b += x
		}
		g = lambda() {
			a + b
		}
		a = 1
		b = 2
		[fa, fb, g]
	}

	def hytest_closure ()
	{
		a = test1()
		fa = a[0]
		fb = a[1]
		g = a[2]
		test_assert_equal(3, g.call())
		test_assert_equal(4, fa(3))
		test_assert_equal(6, g())
		test_assert_equal(5, fb.call(3))
		test_assert_equal(9, g.call())
		test_assert_equal(10, fb(5))
		test_assert_equal(14, g())
		test_assert_equal(12, fa.call(8))
		test_assert_equal(22, g.call())
	}
	

	def hytest_closure_arg_is_local ()
	{
		x = 10
		f = lambda (x) { x += 1; x + 10 } // この x はローカル
		test_assert_equal(10, x)
		test_assert_equal(21, f(10))
		test_assert_equal(21, f(x))
		test_assert_equal(10, x)

		CV = 20							  // 定数
		f = lambda (CV) { CV += 2; CV + 20 } // この CV はローカル
		test_assert_equal(20, CV)
		test_assert_equal(42, f(20))
		test_assert_equal(42, f(CV))
		test_assert_equal(20, CV)

		// f = lambda ($x) { $x += 1; $x + 10 } // 引数はローカル変数のみ
	}


	def hytest_closure_gc ()
	{
		GC.full()
		n = GC.countObjects()

		a = test1()
		fa = a[0]
		fb = a[1]
		g = a[2]

		GC.full()
		// 2 shared local, 3 lambda, 1 array
		test_assert_equal(n+6, GC.countObjects())

		a = nil
		GC.full()
		// remove 1 array
		test_assert_equal(n+5, GC.countObjects())

		fa = nil
		GC.full()
		// remove 1 closure (fa)
		test_assert_equal(n+4, GC.countObjects())

		g = nil
		GC.full()
		// remove 1 closure (g), 1 shared local (a)
		test_assert_equal(n+2, GC.countObjects())


		fb = nil
		GC.full()
		// remove 1 closure (fb), 1 shared local (b)
		test_assert_equal(n, GC.countObjects())
	}

	def hytest_closure_sharelocal ()
	{
		a = lambda(x) {
			b = lambda() {
				c = lambda() {
					d = lambda(y) {
						x + y
					}
					d(1)
				}
				c()
			}
			b()
		}
		10.times{|i|
			test_assert_equal(i+1, a(i))
		}
	}


	def hytest_closure_args_ ()
	{
		f = lambda {
			a = 0
			args_.each{|x| a += x}
			a
		}

		g = lambda {
			a = 1
			// args_の1つめは lambda(*args_)、2つめは each{|*args_| }
			args_.each{a += args_.each().generate()}
			a
		}

		h = lambda {
			a = 2
			// この args_ は lambda (*args_) { }
			args_.each{
				x, *y = *args_ // この args_ は each{|*args_| }
				a += x
			}
			a
		}

		i = lambda {
			a = 3
			// この args_ は lambda (*args_)
			args_.each{|x|
				 a += args_.each().generate() // この args_ も上のと同じ lambda (*args_) である！
			}
			a
		}

		test_assert_equal(0, f())
		test_assert_equal(1, f(1))
		test_assert_equal(3, f(1,2))
		test_assert_equal(6, f(1,2,3))

		test_assert_equal(1, g())
		test_assert_equal(2, g(1))
		test_assert_equal(4, g(1,2))
		test_assert_equal(7, g(1,2,3))

		test_assert_equal(2, h())
		test_assert_equal(3, h(1))
		test_assert_equal(5, h(1,2))
		test_assert_equal(8, h(1,2,3))

		test_assert_equal(3, i())
		test_assert_equal(4, i(1))
		test_assert_equal(5, i(1,2))
		test_assert_equal(6, i(1,2,3))
	}

end
