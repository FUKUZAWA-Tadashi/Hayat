require "hyUnit"

class HyTest_literal < HyUnit

	def retArray ()
	{
		return [100, 200]
	}

	def hytest_literal_array ()
	{
		a = [1, "2", :3, 4.5]
		test_assert_equal(4, a.size())
		test_assert_equal(1, a[0])
		test_assert_equal("2", a[1])
		test_assert_equal(:3, a[2])
		test_assert_equal(4.5, a[3])

		b = []
		test_assert_equal(0, b.size())

		c = retArray()
		test_assert_equal(200, c[1])
	}

	def hytest_literal_hash ()
	{
		a = { 10 => 20, 30.0 => 40.0, true => false, "123" => :456 }
		test_assert_equal(4, a.size())
		test_assert_equal(20, a[10])
		test_assert_equal(40.0, a[30.0])
		test_assert_equal(false, a[true])
		test_assert_equal(:456, a["123"])
		test_assert_equal(nil, a[20])

		b = {}
		test_assert_equal(0, b.size())
	}

	def hytest_literal_list ()
	{
		a = '()
		test_assert_equal('(), a)

		b = '(1,2,3)
		test_assert_equal(1, b.head())
		b = b.tail()
		test_assert_equal(2, b.head())
		b = b.tail()
		test_assert_equal(3, b.head())
		b = b.tail()
		test_assert(b.empty?())
		test_assert_throw_type(:empty_list){ b.head() }
		test_assert_throw_type(:empty_list){ b.tail() }
		b = b.cons(10)
		test_assert_equal(10, b.head())
		b = b.tail()
		test_assert(b.empty?())
	}
end
