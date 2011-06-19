/* -*- coding: utf-8 -*-

Hayat Unit

Hayat上でユニットテストを行なうためのクラス。
継承して使う。


使用例:

class HyTest_hoge < HyUnit
	// クラス名は HyTest_* として、HyUnitを継承する

	def setUp ()
	{
		// フィクスチャ作成等
	}

	def tearDown ()
	{
		// 後始末
	}

	// テストは hytest_* という名前のクラスメソッドにする
	def hytest_fuga ()
	{
		test_assert(b)
		test_assert_equal(1, x)
	}

	def hytest_fugafuga ()
	{
		test_assert_message(b, "fugafuga")
		test_assert_float_equal(3.141592, x, 0.00001)
	}
end


*/
	
	
class HyUnit

	$hyUnit_total_assertions = 0

	def setUp ()
	{
	}

	def tearDown ()
	{
	}

	def test_assert (x)
	{
		$hyUnit_total_assertions += 1
		if (! x) {
			throw :HyUnit_assert_fail, "test_assert"
		}
	}

	def test_assert_message (x, mes)
	{
		$hyUnit_total_assertions += 1
		if (! x) {
			warning(mes)
			throw :HyUnit_assert_fail, "test_assert_message"
		}
	}

	def test_fail ()
	{
		throw :HyUnit_assert_fail, "test_fail"
	}

	def test_ok ()
	{
		$hyUnit_total_assertions += 1
	}

	def print_expected_actual(expected, actual)
	{
		sb = 「expected value = 」
		expected.concatToStringBuffer(sb, -1)
		sb.concat(" , but actual = ")
		actual.concatToStringBuffer(sb, -1)
		warning(sb)
	}

	def test_assert_equal (expected, actual)
	{
		$hyUnit_total_assertions += 1
		if (expected <> actual) {
			print_expected_actual(expected, actual)
			throw :HyUnit_assert_fail, "test_assert_equal"
		}
	}

	def test_assert_equal_message (expected, actual, mes)
	{
		$hyUnit_total_assertions += 1
		if (expected <> actual) {
			warning(mes)
			print_expected_actual(expected, actual)
			throw :HyUnit_assert_fail, "test_assert_equal_message"
		}
	}

	def test_assert_float_equal (expected, actual, delta)
	{
		$hyUnit_total_assertions += 1
		d = expected - actual
		if (d < 0.0) { d = -d }
		if (d > delta) {
			print_expected_actual(expected, actual)
			throw :HyUnit_assert_fail, "test_assert_float_equal"
		}
	}
			
	def test_assert_throw (closure)
	{
		$hyUnit_total_assertions += 1
		failed = false
		try {
			closure(nil)
			failed = true
		} catch {}
		if (failed) {
			throw :HyUnit_assert_fail, "test_assert_throw"
		}
	}
			
	def test_assert_no_throw (closure)
	{
		$hyUnit_total_assertions += 1
		failed = true
		try {
			closure(nil)
			failed = false
		} catch {}
		if (failed) {
			throw :HyUnit_assert_fail, "test_assert_no_throw"
		}
	}
			
	def test_assert_throw_type (type, closure)
	{
		$hyUnit_total_assertions += 1
		failed = false
		try {
			closure(nil)
			failed = true
		} catch (e) {
			if (e.type() <> type) {
				print_expected_actual(type, e.type())
				failed = true
			}
		}
		if (failed) {
			throw :HyUnit_assert_fail, "test_assert_throw_type"
		}
	}
			
	def test_assert_throw_val (val, closure)
	{
		$hyUnit_total_assertions += 1
		failed = false
		try {
			closure(nil)
			failed = true
		} catch (e) {
			if (e.val() <> val) {
				print_expected_actual(val, e.val())
				failed = true
			}
		}
		if (failed) {
			throw :HyUnit_assert_fail, "test_assert_throw_val"
		}
	}
			
	def test_assert_throw_type_val (type, val, closure)
	{
		$hyUnit_total_assertions += 1
		failed = false
		try {
			closure(nil)
			failed = true
		} catch (e) {
			if (e.type() <> type || e.val() <> val) {
				warning("expected <value:type> = <", val, ":", type, "> , but actual = <", e.val(), ":", e.type(), ">")
				failed = true
			}
		}
		if (failed) {
			throw :HyUnit_assert_fail, "test_assert_throw_type_val"
		}
	}
end

