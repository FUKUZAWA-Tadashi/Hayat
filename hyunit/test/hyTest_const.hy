// -*- coding: utf-8 -*-

require "hyUnit"


ASD = 1
QWE = 1.5
FOO = "foo"
const 定数a = "A"
enum {
	BAR_0,
	BAR_1,
	BAR_2, BAR_3,
	BAR_4
}

BAZ = BAR_1 + ASD + 10
const 定数B = ASD * 100

class ConstTest_1
	CT_1 = "ct_1"
	CT_A = "ct_a1"

	def hoge1 () { CT_1 }
	def hoge2 () { CT_A }
end

class ConstTest_2 < ConstTest_1
	CT_2 = "ct_2"
	// CT_A = "ct_a2"  // compile error

	def fuga1 () { CT_1 }
	def fuga2 () { CT_2 }
	def fuga3 () { CT_A }
	def fuga4 () { ConstTest_1::CT_A }
end


class HyTest_const < HyUnit

	def hytest_const ()
	{
		test_assert_equal(1, ASD)
		test_assert_equal(1.5, QWE)
		test_assert_equal("foo", FOO)
		test_assert_equal("A", 定数a)
		test_assert_equal(0, BAR_0)
		test_assert_equal(1, BAR_1)
		test_assert_equal(2, BAR_2)
		test_assert_equal(3, BAR_3)
		test_assert_equal(4, BAR_4)
		test_assert_equal(12, BAZ)
		test_assert_equal(100, 定数B)

		test_assert_equal("ct_1",  ConstTest_1::CT_1)
		test_assert_equal("ct_a1", ConstTest_1::CT_A)
		test_assert_equal("ct_2",  ConstTest_2::CT_2)
		test_assert_equal("ct_a1", ConstTest_2::CT_A)

		test_assert_equal("ct_1",  ConstTest_1::hoge1())
		test_assert_equal("ct_a1", ConstTest_1::hoge2())
		test_assert_equal("ct_1",  ConstTest_2::hoge1())
		test_assert_equal("ct_a1", ConstTest_2::hoge2())

		test_assert_equal("ct_1",  ConstTest_2::fuga1())
		test_assert_equal("ct_2",  ConstTest_2::fuga2())
		test_assert_equal("ct_a1", ConstTest_2::fuga3())
		test_assert_equal("ct_a1", ConstTest_2::fuga4())
	}

	def hytest_local ()
	{
		数 = 0
		ローカル0 = 1
		for (ローカル1 = 0; ローカル1 < 3; ローカル1 += ローカル0) {
			test_assert_equal(数, ローカル1)
			数 += 1
		}
		クロ = lambda () { 数 + 10 }
		test_assert_equal(13, クロ())
		ローカル0 += 10
		ローカル_F = lambda(){ 数 + ローカル0 + 100 }
		test_assert_equal(114, ローカル_F())
	}

end
