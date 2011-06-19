require "hyUnit"


class AClass
	def initialize (x) {
		@a = x + 1
		@b = nil
	}
	def initialize (x,y) {
		@a = x + 1
		@b = y + 1
	}
end



class HyTest_constructor < HyUnit

	def hytest_methodNotFound_inInitialize ()
	{
		test_assert_throw_type(:method_not_found){new AClass(nil)}
		// nil + 1 なので NilClassクラスに + メソッドが無く method_not_found
	}

	def hytest_initialize_overload ()
	{
		x = nil
		test_assert_no_throw{ x = new AClass(1) }
		test_assert_equal(2, x.a)
		test_assert_equal(nil, x.b)
		test_assert_no_throw{ x = new AClass(3,4) }
		test_assert_equal(4, x.a)
		test_assert_equal(5, x.b)
//		test_assert_throw(){new AClass()}
//		test_assert_throw(){new AClass(5,6,7)}
	}

end
