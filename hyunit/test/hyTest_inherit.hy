require "hyUnit"


class Ac
	def f1 () { return "Ac:f1" }
	def f2 () { return "Ac:f2" }
	def ff () { return f2() }
	def c1 () { return "Ac:c1" }
	def c2 () { return "Ac:c2" }

	def initialize (x) { @x = x }
	def setx (x) { @x = x }
	def xp (x) { @x += x }
	def getAx () { return @x }
	def sp (x) { self.xp(x) }

	def call_c2() { return c2() }

	def scv_a(x) { @@a = x }
	def gcv_a() { @@a }
	def scv_b(x) { @@b = x }
	def gcv_b() { @@b }
end

class Bc < Ac
	def f2 () { return "Bc:f2" }
	def c2 () { return "Bc:c2" }

	def initialize (x) {
		Ac::initialize(x)
	}
	def xp (x) { @x += x + 200 }
	def xpA (x) { Ac::xp(x) }
	def getBx () { return @x }

	def scv_a(x) { @@a = x + 1 }
	def gcv_a() { @@a - 1 }
end




class Mc1
	def initialize (x) {
		@m1 = x
		setXYZ(x)
	}
	def get_m1() { @m1 }
	def inc() { @m1 += 1 }
	c++ CppMc {
		includeFile "CppMc.h"
		setXYZ(x:Int):NilClass			// X = x; Y = x+1; Z = x+2
		getY():Int
	}
end

class Mc2 < Mc1
	def initialize (x) {
		Mc1::initialize(x + 2)
		@m2 = x * 2
		@mx = x - 2
	}
	def get_mx() { @mx }
end

class Mc3 < Mc1
	def initialize (x) {
		Mc1::initialize(x + 3)
		@m2 = x * 3
		@my = x - 3
	}
	def get_my() { @my }
end

class Mc5 < Mc2,Mc3
	def initialize (x) {
		Mc1::initialize(x + 5)
		@m2 = x * 5
		@mx = x - 5
		@my = x / 5
	}
end



class HyTest_inherit < HyUnit

	def hytest_inherit ()
	{
		a = new Ac(10)
		b = new Bc(20)
		test_assert_equal("Ac:f2", a.f2())
		test_assert_equal("Bc:f2", b.f2())
		test_assert_equal("Ac:f2", a.ff())
		test_assert_equal("Bc:f2", b.ff())
		test_assert_equal("Ac:c1", Ac.c1())
		test_assert_equal("Ac:c2", Ac.c2())
		test_assert_equal("Ac:c1", Bc.c1())
		test_assert_equal("Bc:c2", Bc.c2())
		test_assert_equal("Ac:c2", a.call_c2())
		test_assert_equal("Bc:c2", b.call_c2())

		test_assert_equal(20, b.getAx())
		test_assert_equal(20, b.getBx())
		b.setx(30)
		test_assert_equal(30, b.getAx())
		test_assert_equal(30, b.getBx())
		b.xp(5)
		test_assert_equal(235, b.getAx())
		test_assert_equal(235, b.getBx())
		b.sp(5)
		test_assert_equal(440, b.getAx())
		test_assert_equal(440, b.getBx())
		b.xpA(5)
		test_assert_equal(445, b.getAx())
		test_assert_equal(445, b.getBx())


		a.scv_a(55)
		a.scv_b(555)
		test_assert_equal(55, a.gcv_a())
		test_assert_equal(555, a.gcv_b())
		b.scv_a(66)
		b.scv_b(666)
		test_assert_equal(67, a.gcv_a())
		test_assert_equal(666, a.gcv_b())
		test_assert_equal(66, b.gcv_a())
		test_assert_equal(666, b.gcv_b())
		a.scv_a(77)
		a.scv_b(777)
		test_assert_equal(77, a.gcv_a())
		test_assert_equal(777, a.gcv_b())
		test_assert_equal(76, b.gcv_a())
		test_assert_equal(777, b.gcv_b())
	}

	def hytest_inherit_2 ()
	{
		m1 = new Mc1(10)
		m2 = new Mc2(10)
		m3 = new Mc3(10)
		m5 = new Mc5(10)

		test_assert_equal(10, m1.m1)
		test_assert_equal(10, m1.get_m1())
		test_assert_equal(12, m2.m1)
		test_assert_equal(12, m2.get_m1())
		test_assert_equal(8, m2.mx)
		test_assert_equal(8, m2.get_mx())
		test_assert_equal(13, m3.m1)
		test_assert_equal(13, m3.get_m1())
		test_assert_equal(7, m3.my)
		test_assert_equal(7, m3.get_my())
		test_assert_equal(15, m5.m1)
		test_assert_equal(15, m5.get_m1())
		test_assert_equal(5, m5.get_mx())
		test_assert_equal(5, m5.mx)
		test_assert_equal(2, m5.get_my())
		test_assert_equal(2, m5.my)

		m1.inc()
		test_assert_equal(11, m1.get_m1())
		m2.inc()
		test_assert_equal(13, m2.get_m1())
		m3.inc()
		test_assert_equal(14, m3.get_m1())
		m5.inc()
		test_assert_equal(16, m5.get_m1())
	}

	def hytest_inherit_method ()
	{
		m1 = new Mc1(10)
		m2 = new Mc2(10)
		m3 = new Mc3(10)
		m5 = new Mc5(10)
		
		test_assert(m1.haveMethod?(:get_m1))
		test_assert(! m1.haveMethod?(:get_mx))

		test_assert(m2.haveMethod?(:get_m1))
		test_assert(m2.haveMethod?(:get_mx))
		test_assert(! m2.haveMethod?(:get_my))

		test_assert(m3.haveMethod?(:get_m1))
		test_assert(! m3.haveMethod?(:get_mx))
		test_assert(m3.haveMethod?(:get_my))

		test_assert(m5.haveMethod?(:get_m1))
		test_assert(m5.haveMethod?(:get_mx))
		test_assert(m5.haveMethod?(:get_my))
	}

	def hytest_inherit_cpp ()
	{
		m1 = new Mc1(10)
		m2 = new Mc2(10)
		m3 = new Mc3(10)
		m5 = new Mc5(10)

		test_assert_equal(11, m1.getY())
		test_assert_equal(13, m2.getY())
		test_assert_equal(14, m3.getY())
		test_assert_equal(16, m5.getY())
	}		

	def hytest_inherit_test ()
	{
		test_assert(Bc < Ac)
		test_assert(Bc <= Ac)
		test_assert(not Bc > Ac)
		test_assert(not Bc >= Ac)

		test_assert(Mc1 == Mc1)
		test_assert(not Mc1 < Mc1)
		test_assert(Mc1 <= Mc1)
		test_assert(not Mc1 > Mc1)
		test_assert(Mc1 >= Mc1)
		test_assert(Mc2 < Mc1)
		test_assert(Mc2 <= Mc1)
		test_assert(not Mc2 > Mc1)
		test_assert(not Mc2 >= Mc1)
		test_assert(Mc3 < Mc1)
		test_assert(Mc3 <= Mc1)
		test_assert(not Mc3 > Mc1)
		test_assert(not Mc3 >= Mc1)
		test_assert(Mc5 < Mc1)
		test_assert(Mc5 <= Mc1)
		test_assert(not Mc5 > Mc1)
		test_assert(not Mc5 >= Mc1)

		test_assert(not Mc2 <= Mc3)
		test_assert(not Mc2 >= Mc3)
		test_assert(Mc5 < Mc2)
		test_assert(Mc5 < Mc3)
		test_assert(Mc2 > Mc5)
		test_assert(Mc3 > Mc5)
	}

end
