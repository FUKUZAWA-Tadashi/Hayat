//
// バイトコードリロードテスト
// その1
//


CONST = "const1"
$global = "glob1"


class ReloadTest
	CONST_C = "const_c1"
	@@classVar = "cv1"

	def initialize()
	{
		@insVar = "ins1"
	}

	def values()
	{
		[CONST, $global, CONST_C, @@classVar, @insVar]
	}

	// フィボナッチ
	defFiber fn ()
	{
		a = 1
		b = 1
		loop {
			yield a
			a,b = b,a+b
		}		
	}

	def test (pool)
	{
		a = 「test」
		wait(2)
		a.concat(testSub())
		pool.push(a)
	}

	def testSub ()
	{
		"Reload1"
	}

	def g0 ()
	{
		1
	}

	defFiber g1 ()
	{
		loop {
			yield g0()
		}
	}

end


class Reload_obs
	C1 = "C1"
	C3 = "C3"
	@@cv1 = "cv1"
	@@cv3 = "cv3"
	def g() { [C1,C3,@@cv1,@@cv3] }
end


class Reload_memb
	def initialize ()
	{
		@memb1 = "@1"
		:memb3
		@memb2 = "@2"
	}
end


class Reload_closure
	def c() {
		x = 0
		lambda() { x += 1 }
	}

	def f ()
	{
		wait(1)
		g = c()
		wait(1)
		g()
		wait(2)
		g()
		wait(2)
		g()
		wait(2)
		g()
		wait(2)
		g()
	}
end
