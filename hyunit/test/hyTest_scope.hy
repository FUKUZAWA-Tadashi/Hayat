// -*- coding: utf-8 -*-

require "hyUnit"
require "scope_required"


C = 100
def f () { return C }

class AClass
	C = 1
	def f () { return C }
	class BClass
		C = 2
		def f () { return C }
	end
end


class HyTest_scope1 < HyUnit
	def hytest_scope1 ()
	{
		test_assert_equal(100, C)
		test_assert_equal(100, f())
		test_assert_equal(1, AClass::C)
		test_assert_equal(1, AClass::f())
		test_assert_equal(2, AClass::BClass::C)
		test_assert_equal(2, AClass::BClass::f())
		test_assert_equal(100, ::C)
		test_assert_equal(100, ::f())
		test_assert_equal(1, ::AClass::C)
		test_assert_equal(1, ::AClass::f())
		test_assert_equal(2, ::AClass::BClass::C)
		test_assert_equal(2, ::AClass::BClass::f())
		test_assert_equal(99, D)
		test_assert(g() >= 101)
	}
end

class HyTest_scope2 < AClass, HyUnit
	def hytest_scope2 ()
	{
		test_assert_equal(1, C)
		test_assert_equal(1, f())
		test_assert_equal(2, BClass::C)
		test_assert_equal(2, BClass::f())
		test_assert_equal(1, AClass::C)
		test_assert_equal(1, AClass::f())
		test_assert_equal(2, AClass::BClass::C)
		test_assert_equal(2, AClass::BClass::f())
		test_assert_equal(100, ::C)
		test_assert_equal(100, ::f())
		test_assert_equal(1, ::AClass::C)
		test_assert_equal(1, ::AClass::f())
		test_assert_equal(2, ::AClass::BClass::C)
		test_assert_equal(2, ::AClass::BClass::f())
		test_assert_equal(99, D)
		v = g()
		test_assert(v >= 101)
		test_assert_equal(v+1, g())
	}
end

class HyTest_scope3 < HyUnit
	C = 200
	def f () { return C }
	class AClass
		C = 11
		def f () { return C }
		class BClass
			C = 22
			def f () { return C }
		end
		class CClass
			C = 33
			def f () { return C }
			def g () { return BClass::f() }
			def h () { return AClass::f() }
		end
	end
	def hytest_scope3 ()
	{
		test_assert_equal(11, AClass::C)
		test_assert_equal(11, AClass::f())
		test_assert_equal(22, AClass::BClass::C)
		test_assert_equal(22, AClass::BClass::f())
		test_assert_equal(200, C)
		test_assert_equal(200, f())
		test_assert_equal(1, ::AClass::C)
		test_assert_equal(1, ::AClass::f())
		test_assert_equal(2, ::AClass::BClass::C)
		test_assert_equal(2, ::AClass::BClass::f())
		test_assert_equal(100, ::C)
		test_assert_equal(100, ::f())
		test_assert_equal(33, AClass::CClass::C)
		test_assert_equal(33, AClass::CClass::f())
		test_assert_equal(22, AClass::CClass::g())
		test_assert_equal(11, AClass::CClass::h())
		test_assert_equal(99, D)
		test_assert(g() >= 101)
	}

	def hytest_scopedClass ()
	{
		test_assert_equal(:BClass, (AClass::BClass).classSymbol())
		test_assert_equal(:CClass, (AClass::CClass).classSymbol())
		test_assert_equal(:AClass, (::AClass).classSymbol())
		test_assert_equal(:BClass, (::AClass::BClass).classSymbol())
		test_assert(AClass::BClass != ::AClass::BClass)

		// DClass,EClass,FClass ともこのファイルの下の方にあるので
		// 現時点ではコンパイルされていない
		// DClassはクラスと推定、EClassは定数かインナークラスか不明、
		// FClassは何だか不明な状態
		test_assert_equal(::DClass::EClass, FClass.extScope())
	}

	def hytest_implicit_method_call ()
	{
		x = FClass.arr			// getSomething FClass
		test_assert_equal([9,8], x)
		y = FClass.arr()		// getSomething FClass
		test_assert_equal([9,8], y)

		t = FClass				// getSomething FClass
		x = t.arr
		test_assert_equal([9,8], x)
		y = t.arr()
		test_assert_equal([9,8], y)

		x = ExtClass.xxx
		test_assert_equal([91,92,93], x)
		y = ExtClass.xxx()
		test_assert_equal([91,92,93], y)
		t = ExtClass
		x = t.xxx
		test_assert_equal([91,92,93], x)
		y = t.xxx()
		test_assert_equal([91,92,93], y)

		x = something			// getSomething something
		test_assert_equal([3,1,4], x)
		x = something()			// method_0 something
		test_assert_equal([3,1,4], x)
		x = something[1]		// getSomething something
		test_assert_equal(1, x)
	}

end

class HyTest_scope4 < HyUnit
	using AClass
	def hytest_scope4 ()
	{
		test_assert_equal(1, C)
		test_assert_equal(1, f())
		test_assert_equal(2, BClass::C)
		test_assert_equal(2, BClass::f())
		test_assert_equal(1, AClass::C)
		test_assert_equal(1, AClass::f())
		test_assert_equal(2, AClass::BClass::C)
		test_assert_equal(2, AClass::BClass::f())
		test_assert_equal(1, ::AClass::C)
		test_assert_equal(1, ::AClass::f())
		test_assert_equal(2, ::AClass::BClass::C)
		test_assert_equal(2, ::AClass::BClass::f())
		test_assert_equal(100, ::C)
		test_assert_equal(100, ::f())
		test_assert_equal(99, D)
		test_assert_equal("E!", E[0])
		test_assert_equal(2, E_02)
		test_assert(g() >= 101)
	}
end

class HyTest_scope5 < HyUnit
	def hytest_scope5 ()
	{
		test_assert_equal(90, ExtClass::INTCONST)
		test_assert_equal(1.732, ExtClass::FLOATCONST)
		test_assert_equal(1, ExtClass::EC_01)
		test_assert_equal(92, ExtClass::ARR[1])
	}
end



class CClass
	def getSelf() { self }
end

class CSubClass < CClass
	def f () {
		CClass::getSelf()
	}
end

class DClass
	def f () {
		CClass::getSelf()
	}

	class EClass
	end
end

class FClass
	def extScope () {
		::DClass::EClass
	}
	def arr () {
		[9,8]
	}
end

def something () {
	[3,1,4]
}

class HyTest_scope6 < HyUnit
	def hytest_scope6 ()
	{
		test_assert_equal(CClass, CClass::getSelf())
		test_assert_equal(CClass, CClass.getSelf())
		test_assert_equal(CSubClass, CSubClass::f())
		test_assert_equal(CSubClass, CSubClass.f())
		test_assert_equal(CClass, DClass::f())
		test_assert_equal(CClass, DClass.f())
	}
end
