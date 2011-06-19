require "hyUnit"


class HyTest_ffi < HyUnit

	c++ Membtest {
		includeFile "test_ffi_memb.h"
		memb():Int
		memb=(x:Int):Int

		paramCheck(x1:Int, x2:Float, x3:Array):Bool
		fac(x:Int):Array
		myThrow(v:Object)
		toStringTest(x:String):String
		upcase(x:StringBuffer):StringBuffer
		defval1(x1 = 1, x2 = 2, x3 = 3):Int	// x1+x2+x3+1
		defval2(x1:Int, x2:Int = 20, x3:Float = 30.0):Float	// x1+x2+x3+1

		over(x:Int):Int
		over(x1:Int, x2:Int):Int
		over(x1:Int, x2:Int, x3:Float):Int
	}

	class Inner
		c++ {
			testAdd(x:Int, y:Int):Int
		}
	end

	def hytest_innerClassFfi ()
	{
		test_assert_equal(10, Inner.testAdd(3,7))
		test_assert_equal(30, Inner.testAdd(10,20))
	}


	def hytest_member_access ()
	{
		a = new HyTest_ffi()
		a.memb = 10
		test_assert_equal(10, a.memb)
		a.memb = 20
		test_assert_equal(20, a.memb)
		a.memb += 12
		test_assert_equal(32, a.memb)
	}

	def hytest_paramCheck ()
	{
		test_assert_equal(true, paramCheck(1, 2.0, [1, 2.0]))
		test_assert_equal(false, paramCheck(2, 3.5, [2, 2.5]))
		test_assert_equal(false, paramCheck(2, 3.5, [3, 3.5]))
	}
	
	def hytest_fac ()
	{
		test_assert_equal([1,2,6,24,120], fac(5))
		test_assert_equal([1,2,6,24,120,720,5040,40320,362880,3628800], fac(10))
	}
	
	def hytest_myThrow ()
	{
		test_assert_throw_val(100){ myThrow(100) }
		test_assert_throw_val([10, -200]){ myThrow([10, -200]) }
	}
	
	def hytest_toStringTest()
	{
		s = toStringTest("asd")
		test_assert_equal("asd", s)
		sb = <<qwe>>
		sb.concat("foo")
		s = toStringTest(sb)	// String引数にStringBufferを与えられる //
		test_assert_equal("qwefoo", s)
	}

	def hytest_upcase_StringBuffer ()
	{
		sb = <<abc>>
		test_assert_equal("abc", sb)
		test_assert_equal("ABC", upcase(sb))
	}
	
    def hytest_ffi_defaultVal ()
    {
        test_assert_equal(7, defval1())
        test_assert_equal(16, defval1(10))
        test_assert_equal(34, defval1(10,20))
        test_assert_equal(61, defval1(10,20,30))

        test_assert_equal(151.0, defval2(100))
        test_assert_equal(331.0, defval2(100,200))
        test_assert_equal(601.0, defval2(100,200,300.0))
    }

	def hytest_ffi_overload ()
	{
		test_assert_equal(110, over(10))
		test_assert_equal(230, over(10,20))
		test_assert_equal(99, over(10,20,30.0))

		a = new HyTest_ffi()
		test_assert_equal(110, a.over(10))
		test_assert_equal(230, a.over(10,20))
		test_assert_equal(99, a.over(10,20,30.0))
	}


end

class Dummy_ffi_test
	c++ Membtest {
		includeFile "test_ffi_memb.h"
	}
end
