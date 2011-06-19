require "hyUnit"


class HyTest_stack < HyUnit

	def hytest_stack ()
	{
		s = new Stack(2)
		test_assert_equal(0, s.size())

		s.push(10)
		s.push(nil)
		s.push(3.0)
		test_assert_equal(3, s.size())
		test_assert_equal(3.0, s.pop())
		test_assert_equal(nil, s.pop())
		test_assert_equal(10, s.pop())
		test_assert_equal(0, s.size())

		s = new Stack()

		s.push(20)
		s.push(40)
		s.push(60)
		test_assert_equal(3, s.size())
		test_assert_equal(60, s.getTop())
		test_assert_equal(3, s.size())
		s.drop(2)
		test_assert_equal(20, s.getTop())
		test_assert_equal(1, s.size())
		s.clean()
		test_assert_equal(0, s.size())
	}
end
