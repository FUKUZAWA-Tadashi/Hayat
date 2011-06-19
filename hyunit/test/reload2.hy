//
// バイトコードリロードテスト
// その2
//


CONST = "const2"
$global = "glob2"


class ReloadTest
	CONST_C = "const_c2"
	@@classVar = "cv2"

	def initialize()
	{
		@insVar = "ins2"
	}

	def values()
	{
		[CONST, $global, CONST_C, @@classVar, @insVar]
	}

	// 平方数
	defFiber fn ()
	{
		x = 1
		c = 3
		loop {
			yield x
			x += c
			c += 2
		}		
	}

	def testSub ()
	{
		" reload 2"
	}

	def g0 ()
	{
		2
	}

	defFiber g1 ()
	{
		loop {
			yield g0()
		}
	}

end


class Reload_obs
	C1 = "2_C1"
	C2 = "2_C2"
	C4 = "2_C4"
	@@cv1 = "2_cv1"
	@@cv2 = "2_cv2"
	@@cv4 = "2_cv4"
	def g() { [C1,C2,C4,@@cv1,@@cv2,@@cv4] }
end


class Reload_memb
	def initialize ()
	{
		@memb1 = "#1"
		@memb3 = "#3"
		:memb2
		@memb4 = "#4"
	}
end


class Reload_closure
	def c() {
		x = 0
		lambda() { x += 10 }
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
