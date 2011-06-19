require "hyUnit"


class Rep2
	def initialize() { @a = 0 }
	def get_a() { @a }
	def rep2 (s)
	{
		(s * 2).times{|x|
			if (x == 11) break
			@a = x
			wait
		}
	}
end


class Gs2
	def initialize () {
		@x = 1
	}
	def sieve () {
		@x += 1
	}
end
class Sieve
	def initialize (x, o) {
		@x = x
		@o = o
	}
	def sieve () {
		y = @o.sieve()
		while (y % @x == 0) {
			y = @o.sieve()
		}
		y
	}
end


class HyTest_fiber < HyUnit
	
	defFiber gen1 (n)
	{
		x = 0
		for (i = 0; i < n; i += 1) {
			x += yield i
		}
		return x
	}

	def hytest_fiber ()
	{
		gen = gen1(3)
		test_assert_equal(Fiber, gen.getClass())
		test_assert_equal(0, gen.generate())
		gen.yieldResult(10)
		test_assert_equal(1, gen.generate())
		gen.yieldResult(20)
		test_assert_equal(2, gen.generate())
		gen.yieldResult(30)
		test_assert_throw_type_val(:fiber_return, 60){ gen.generate() }
		test_assert(gen.finished?())
	}

	def hytest_fiber2 ()
	{
		gen = gen1(3)
		test_assert_equal(Fiber, gen.getClass())
		test_assert_equal(true, gen.go())
		test_assert_equal(true, gen.go())
		test_assert_equal(0, gen.getYieldVal())
		test_assert_throw_type(:fiber_error){ gen.getYieldVal() }
		test_assert_throw_type(:fiber_error){ gen.getReturnVal() }
		gen.yieldResult(11)

		test_assert_equal(true, gen.go())
		test_assert_equal(false, gen.finished?())
		test_assert_equal(1, gen.getYieldVal())
		gen.yieldResult(22)
		
		test_assert_equal(true, gen.go())
		test_assert_equal(false, gen.finished?())
		test_assert_equal(2, gen.getYieldVal())
		gen.yieldResult(33)
		
		test_assert_equal(false, gen.go())
		test_assert_throw_type(:fiber_error){ gen.getYieldVal() }
		test_assert_equal(false, gen.finished?())
		test_assert_equal(66, gen.getReturnVal())
		test_assert_equal(true, gen.finished?())
		test_assert_throw_type(:fiber_error){ gen.getReturnVal() }
		test_assert_throw_type(:fiber_error){ gen.go() }
	}


	def hytest_iterate ()
	{
		a = 0
		5.times().iterate(lambda(x){a += x})
		test_assert_equal(10, a) // 0+1+2+3+4
		b = 0
		10.times{|x| b += x}
		test_assert_equal(45, b) // 0+1+..+8+9
		c = 0
		11.times{|y|c += y}
		test_assert_equal(55, c) // 0+1+..+9+10

		d = 0
		10.times{|x|
			if (x >= 5) { 99; break }
			d += x
		}
		test_assert_equal(10, d)
		e = 0
		f = 0
		10.times{|x|
		  label :redo
			e += x
			if (x == 5 && f == 0) {
				f = 1
				goto :redo
			}
		}
		test_assert_equal(45+5, e)
		g = 0
		10.times{|x|
			if (x == 5) next
			g += x
		}
		test_assert_equal(45-5, g)
	}


	def gen4 ()
	{
		new Fiber {
			ret = nil
			try {
				for (i = 1; true; i += 1) {
					yield i
				}
			} catch (e) {
				ret = e.val()
			}
			ret
		}
	}

	def hytest_thrown ()
	{
		y = 0
		a = gen4()->{|x|
			y += x
			if (x == 2) throw "a"
		}
		test_assert_equal("a", a)
		test_assert_equal(1+2, y)
	}

	def hytest_jumpControl ()
	{
		a = new StringBuffer(30)
		b = true
		x = gen4()->{|i|
		  label :redo
			a.concat(i)
			try {
				a.concat("a")
				if (i == 2)
					next
				a.concat("b")
				if (i == 3)
					throw 1
				a.concat("c")
				if (i == 4 && b) {
					b = false
					goto :redo
				}
				a.concat("d")
				if (i == 5)
					break
				a.concat("e")
			} catch (e){
				if (e.getLabel() == :redo)
					goto :redo
				a.concat("(",e.val(),")")
			}
		}
		test_assert_equal("1abcde2a3ab(1)4abc4abcde5abcd", a)
		test_assert_equal(nil, x)
	}

	def jumpOut_sub (n)
	{
		[0,1,5,10,20].each {|i|
			if (i == n) {
				if (n == 5) {
					「out{n}」
					goto :out
				} else if (n == 10) {
					"brk"
					break
				}
			}
			i * 10
		}
		label :out
	}
	def hytest_jumpOut ()
	{
		test_assert_equal("out5", jumpOut_sub(5))
		test_assert_equal("brk", jumpOut_sub(10))
		test_assert_equal(nil, jumpOut_sub(20))
	}



	// primes = [ p | (p:_) <- iterate sieve [2..] ]
	// sieve (p:ps) = [ x | x <- ps, mod x p /= 0 ]

	defFiber genFrom (n)
	{
		for ( ; true; n += 1) {
			yield n
		}
	}
	def sieve (p, gen)
	{
		gen.filter{|i| i % p <> 0}
	}
	defFiber primes ()
	{
		gen = genFrom(2)
		loop {
			x = gen.generate()
			yield x
			gen = sieve(x,gen)
		}
	}

	// sieveとprimesを1つにまとめた実装
	defFiber sieve2 (gen)
	{
		x = gen.generate()
		yield x
		z = sieve2(gen.filter{|i| i % x <> 0})
		loop { yield z.generate() }
	}

	// generate()を使わずメソッド引数の多重代入を使用した実装
	defFiber sieve3 (x, *y)
	{
		yield x
		sieve3(*(y.filter{|i| i % x != 0})).each{|w| yield w}
	}

	def hytest_sieve ()
	{
		x = primes().take(20).toArray()
		test_assert_equal([2,3,5,7,11, 13,17,19,23,29,
							  31,37,41,43,47, 53,59,61,67,71], x)
		y = sieve2(genFrom(2)).take(20).toArray()
		test_assert_equal(x, y)
		z = sieve3(*genFrom(2)).take(20).toArray()
		test_assert_equal(x, z)

		/*
		w = primes()
		1000.times{w.generate()}
		Debug.print(w.generate())
		*/
	}

	def hytest_sieve_2 ()
	{
		g = new Gs2()
		x = 0
		10.times{
			x = g.sieve()
			g = new Sieve(x, g)
		}
		//warning(x)
	}
	

	def testReturn1 ()
	{
		x1 = lambda(){
			return 10
		}
		x2 = x1.callAway() // return 10 が tertReturn4 の戻り値となる
		return 20
	}
	def testReturn2 ()
	{
		v = []
		x1 = lambda(){
			lambda(){
				return 10
			}
		}
		x2 = x1.call()
		v[0] = x2.call()
		x3 = x1.callAway() // x1にreturn文は無いのでcallと同じ
		v[1] = x3.call()
		v
	}
	def testReturn3 ()
	{
		v = []
		x1 = lambda(){
			x2 = lambda(){
				x3 = lambda() {
					return 3
				}
				v[2] = x3.call() // 普通に3
				return 2
			}
			v[1] = x2.callAway() // callAwayで呼んだので x2のreturnがx1に返る
			test_fail() // ここには来ない
			return 1
		}
		v[0] = x1.call()
		v
	}
	def testReturn4 ()
	{
		a = gen1(10)->{|x|
			// iterate()の中で、このclosureをcallAwayで呼んでいる
			if (x == 5) return -10  // この return は testReturn4 を抜ける
			x * 2
		}
		test_fail() // ここには来ない
		a * 2
	}
	def testReturn5 ()
	{
		a = lambda {
			b = lambda {
				c = lambda {
					return 10
					test_fail()
				}
				return 1 + c.callAway() + 1	// 計算終了する前に c の return 10
				test_fail()
			}
			return 2 + b.callAway() + 2 // bのreturnは実行されてないので 2+10+2
			test_fail()
		}
		return 4 + a.callAway() + 4 // 計算終了する前に a の return 14
		test_fail()
	}

	def hytest_callAway ()
	{
		test_assert_equal(10, testReturn1())
		test_assert_equal([10,10], testReturn2())
		test_assert_equal([2,nil,3], testReturn3())
		test_assert_equal(-10, testReturn4())
		test_assert_equal(14, testReturn5())
	}



	def rep1 (s)
	{
		a = 0
		(s * 2).times{|it|
			if (it == 10) break
			a = it
			wait
		}
		a
	}

	def rep_t (c)
	{
		t = new Thread()
		x = nil
		t.startRun{x = c(nil)}
		t.join()
		x
	}

	def hytest_take ()
	{
		a = 100.times().take(4).toArray()
		test_assert_equal([0,1,2,3], a)
		a = 5.times().take(100).toArray()
		test_assert_equal([0,1,2,3,4], a)

		y = 0
		a = gen4().take(10)->{|x|
			y += x
			if (x == 3) throw "b"
		}
		test_assert_equal("b", a)
		test_assert_equal(1+2+3, y)

		y = 0
		a = gen4().take(5)->{|x|
			y += x
			if (x == 10) throw "c"
		}
		test_assert_equal(5, a)
		test_assert_equal(1+2+3+4+5, y)

		y = 0
		a = gen4().take(10)->{|x|
			y += x
			if (x == 4) break
		}
		test_assert_equal(nil, a)
		test_assert_equal(1+2+3+4, y)

		y = 0
		a = gen4().take(10)->{|x|
			if (x == 4) next
			y += x
		}
		test_assert_equal(10, a)
		test_assert_equal(1+2+3 +5+6+7+8+9+10, y)

		y = 0
		z = 0
		a = gen4().take(10)->{|x|
		  label :redo
			y += x
			if (x == 5 && z == 0) {
				z = 1
				goto :redo
			}
		}
		test_assert_equal(10, a)
		test_assert_equal(55+5, y)

	}

	def hytest_fiber_thread_gc ()
	{
		c = lambda{ rep1(10) }
		x = c(nil)
		test_assert_equal(9, x)

		t = new Thread()
		y = nil
		t.startRun(lambda(){y = rep1(10)})
		t.join()
		test_assert_equal(9, y)

		t1 = new Thread()
		t2 = new Thread()
		r = new Rep2()
		c = lambda{r.rep2(100)}
		t1.startRun(lambda(){c(nil)})
		t2.startRun{loop{GC.full();wait}}
		t1.join()
		t2.kill()
		test_assert_equal(10, r.get_a())

		test_assert_equal(9, rep_t(lambda{rep1(12)}))

		r2 = new Rep2()
		test_assert_equal(10, rep_t{r2.rep2(50); r2.get_a()})

		t1 = new Thread()
		t2 = new Thread()
		t3 = new Thread()
		r3 = new Rep2()
		x = nil
		y = nil
		t1.startRun(lambda(){x=rep1(10)})
		t2.startRun{r3.rep2(20); y=r3.get_a()}
		t3.startRun{loop{GC.full();wait}}
		t1.join()
		t2.join()
		t3.kill()
		test_assert_equal(9,x)
		test_assert_equal(10,y)
	}


	// This is not a test routine.
	// dummy program of generating test data for tools/gokou
// 	def hytest_dummy()
// 	{
// 		p = primes()
// 		p.take(10).each{|x| Debug.p(x)}
// 		p.take(10).each{|x| Debug.print(x)}
// 		p.take(10).each{|x| Debug「{x};」}
// 		Debug.setPrintOpcode(true)
// 		gen4()->{|i|
// 			if (i == 3) Debug.printAllStackTrace()
// 			else if (i == 5) throw "stop"
// 			i
// 		}
// 		Debug.setPrintOpcode(false)
// 	}
end
