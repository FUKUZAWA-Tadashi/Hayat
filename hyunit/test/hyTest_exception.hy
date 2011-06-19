require "hyUnit"

class Extest_1
    def extest1 ()
    {
        throw "throw test extest1"
    }
end

class Extest_2
    def extest2 ()
    {
        a = new Extest_1()
        a.extest1()
    }
end


class Extest_3
    def extest3 ()
    {
        Extest_2.extest2()
    }
end

class Thrower
    def initialize (x) 
    {
        @x = x
    }
	
    def myThrow ()
    {
        throw @x
    }
end



class HyTest_exception < HyUnit

	def hytest_extest ()
	{
		try {
			Extest_3.extest3()
			test_fail()
		} catch (xx) {
			test_assert_equal("throw test extest1", xx.val())
		}
	}

	def hytest_exception1 ()
	{
		try {
			throw new Exception()
			test_fail()
		} catch {
			test_ok()
		}

		try {
			a = 0
			try {
				a += 1
				t = new Thrower(2)
				a += 4
				t.myThrow()
				a += 8
			} catch (e) {
				a += e.val()
				throw 16
				a += 32
			}
			a += 64
		} catch (e) {
			a += e.val()
			a += 128
		}
		test_assert_equal(1+4+2+16+128, a)
	}		

	def hytest_exception2 ()
	{
		i = 0
		a = new StringBuffer(30)
		while (i < 10) {
		  label :redo
			i += 1
			a.concat(i)
			try {
				a.concat("a")
				if (i == 2)
					next
				a.concat("b")
				if (i == 3)
					throw 1
				a.concat("c")
				if (i == 4)
					goto :redo
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
		test_assert_equal("1abcde2a3ab(1)4abc5abcd", a)
	}

	def hytest_exception3 ()
	{
		try {
			a = 1 % 0
			test_fail()
		} catch (e) {
			test_assert_equal(:division_by_zero, e.type())
		}

		a = [10, 20]
		try {
			x = a[-10]
			test_fail()
		} catch (e) {
			// test_assert("Array index out of bounds".match(e.val()))
			test_assert(true)
		}
		try {
			a[-10] = 100
			test_fail()
		} catch (e) {
			// test_assert("Array index out of bounds".match(e.val()))
			test_assert(true)
		}
		
	}

	def hytest_exception4 ()
	{
		for (i = 0; i < 10; i += 1) {
		  label :redo
			try {
				if (i == 1) next
				if (i == 2) {
					i = 4
					goto :redo
				}
				if (i == 7) break;
				// breakしたのでi==8にはならない
				if (i == 8) goto :asdqwe
			} catch (e) {
				// next,breakはcatchしない
				if (e.getLabel() == :redo)
					goto :redo
				test_fail()
			}
		}
		try {
			120 + 3
			goto :asdqwe
		} catch (e) {
			test_assert_equal(:goto, e.type())
			test_assert_equal(:asdqwe, e.getLabel())
			test_assert_equal(123, e.val())
			goto :foobar
		}
		test_fail()
	label :asdqwe
		test_fail()
	label :foobar
		test_ok()
	}


	def ftest0 ()
	{
		try {
			1
		} finally {
			2
		}
	}
	def hytest_finally0 ()
	{
		test_assert_equal(1, ftest0())
	}

	def hytest_finally1 ()
	{
		try {
			a = 1
			try {
				throw 111
			} finally {
				a = 2
			}
		} catch (e) {
			test_assert_equal(111, e.val())
			test_assert_equal(2, a)
			return
		}
		test_fail()
	}

	def ftest2 ()
	{
		@@ft = 10
		try {
			return 20
		} finally {
			@@ft = 30
			40
		}
		@@ft = 50
		60
	}
	def hytest_finally2 ()
	{
		test_assert_equal(20, ftest2())
		test_assert_equal(30, @@ft)
	}

	def ftest3 ()
	{
		@@ft = 100
		try {
			return 200
		} finally {
			@@ft = 300
			return 400
		}
		@@ft = 500
		600
	}
	def hytest_finally3 ()
	{
		test_assert_equal(400, ftest3())
		test_assert_equal(300, @@ft)
	}

	def ftest4 ()
	{
		@@ft = 101
		try {
			throw 202
		} finally {
			@@ft = 303
			throw 404
			505
		}
		@@ft = 606
		707
	}
	def hytest_finally4 ()
	{
		test_assert_throw_val(404){ftest4()}
		test_assert_equal(303, @@ft)
	}
		
	def hytest_finally5 ()
	{
		try {
			a = 91
		} finally {
			a = 92
		}
		test_assert_equal(92, a)
	}


	def hytest_catchfinally ()
	{
		try {
			a = 71
			b = 61
			c = 51
			throw 81
		} catch (e) {
			a = 72
			b = e.val()
		} finally {
			c = 52
		}
		test_assert_equal(72, a)
		test_assert_equal(81, b)
		test_assert_equal(52, c)
	}

	def hytest_catchfinally2 ()
	{
		try {
			a = 73
			b = 63
			c = 53
		} catch (e) {
			a = 74
			b = e.val()
		} finally {
			c = 54
		}
		test_assert_equal(73, a)
		test_assert_equal(63, b)
		test_assert_equal(54, c)
	}

	def hytest_catchfinally3 ()
	{
		f = lambda(){
			try {
				1
				throw 2
				3
			} catch (e) {
				4
			} finally {
				5
			}
		}
		test_assert_equal(4, f())
	}

end
