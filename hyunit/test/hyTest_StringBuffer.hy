// -*- coding: utf-8 -*-

require "hyUnit"


class HyTest_StringBuffer < HyUnit

	def hytest_compare ()
	{
		a = 「hayataro」

		test_assert(a == "hayataro")
		test_assert(a != "Hayataro")
		test_assert((a <=> "hayataro") == 0)
		test_assert((a <=> "Hayataro") > 0)
		test_assert( ! (a == 123) )
		test_assert_throw_type(:type_mismatch){ a <=> 123 }

		b = "hayataro"
		test_assert(b == 「hayataro」)
		test_assert(b != 「Hayataro」)
		test_assert((b <=> 「hayataro」) == 0)
		test_assert((b <=> 「Hayataro」) > 0)
		test_assert( ! (b == 123) )
		test_assert_throw_type(:type_mismatch){ b <=> 123 }
	}

	def hytest_format ()
	{
		test_assert_equal("123", 「{%d % 120 + 3}」)
		test_assert_equal("-123", 「{%d % -125+2}」)
		test_assert_equal("+123", 「{%+d%123}」)
		test_assert_equal("-123", 「{%+d%-123}」)
		test_assert_equal(" 123", 「{% d%123}」)
		test_assert_equal("-123", 「{% d%-123}」)
		test_assert_equal("  123", 「{%5d%123}」)
		test_assert_equal(" -123", 「{%5d%-123}」)
		test_assert_equal("123  ", 「{%-5d%123}」)
		test_assert_equal("-123 ", 「{%-5d%-123}」)
		test_assert_equal("00123", 「{%05d%123}」)
		test_assert_equal("-0123", 「{%05d%-123}」)
		test_assert_equal("+0123", 「{%+05d%123}」)
		test_assert_equal("-0123", 「{%+05d%-123}」)

		test_assert_equal("173", 「{%o%123}」)
		test_assert_equal("123", 「{%u%123}」)
		test_assert_equal("7b", 「{%x%123}」)
		test_assert_equal("7B", 「{%X%123}」)
		test_assert_equal("0x7b", 「{%#x%123}」)

		test_assert_equal("14.250000", 「{%f%14.25}」)
		test_assert_equal(" 14.250000", 「{%10f%14.25}」)
		test_assert_equal("   14.2500", 「{%10.4f%14.25}」)
		f = 「{%e%14.25}」
		test_assert(f == "1.425000e+01" || f == "1.425000e+001")
		f = 「{%.4e%14.25}」
		test_assert(f == "1.4250e+01" || f == "1.4250e+001")
		f = 「{%14e%14.25}」
		test_assert(f == "  1.425000e+01" || f == " 1.425000e+001")
		f = 「{%14.4e%14.25}」
		test_assert(f == "    1.4250e+01" || f == "   1.4250e+001")

		test_assert_equal("A", 「{%c%65}」)
		test_assert_equal("str", 「{%s % "str"}」)
	}

end
