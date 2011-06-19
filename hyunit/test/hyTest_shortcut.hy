require "hyUnit"


class HyTest_shortcut < HyUnit

	def f1 () { $a += 1; true }
	def f2 () { $a += 1; false }
	def g1 () { $b += 1; true }
	def g2 () { $b += 1; false }

	def hytest_shortcut ()
	{
		$a = 0; $b = 0
		x = f1() || g1()
		test_assert_equal(true, x)
		test_assert_equal(1, $a)
		test_assert_equal(0, $b)
		$a = 0; $b = 0
		x = f1() && g1()
		test_assert_equal(true, x)
		test_assert_equal(1, $a)
		test_assert_equal(1, $b)
		
		$a = 0; $b = 0
		x = f2() || g1()
		test_assert_equal(true, x)
		test_assert_equal(1, $a)
		test_assert_equal(1, $b)
		$a = 0; $b = 0
		x = f2() && g1()
		test_assert_equal(false, x)
		test_assert_equal(1, $a)
		test_assert_equal(0, $b)
		
		$a = 0; $b = 0
		x = f1() || g2()
		test_assert_equal(true, x)
		test_assert_equal(1, $a)
		test_assert_equal(0, $b)
		$a = 0; $b = 0
		x = f1() && g2()
		test_assert_equal(false, x)
		test_assert_equal(1, $a)
		test_assert_equal(1, $b)
		
		$a = 0; $b = 0
		x = f2() || g2()
		test_assert_equal(false, x)
		test_assert_equal(1, $a)
		test_assert_equal(1, $b)
		$a = 0; $b = 0
		x = f2() && g2()
		test_assert_equal(false, x)
		test_assert_equal(1, $a)
		test_assert_equal(0, $b)
	}

	def hytest_substOp ()
	{
		x = true ; $a = 0
		x &&= f1()
		test_assert_equal(true, x)
		test_assert_equal(1, $a)
		x = true ; $a = 0
		x ||= f1()
		test_assert_equal(true, x)
		test_assert_equal(0, $a)
		x = false ; $a = 0
		x &&= f1()
		test_assert_equal(false, x)
		test_assert_equal(0, $a)
		x = false ; $a = 0
		x ||= f1()
		test_assert_equal(true, x)
		test_assert_equal(1, $a)

		x = true ; $a = 0
		x &&= f2()
		test_assert_equal(false, x)
		test_assert_equal(1, $a)
		x = true ; $a = 0
		x ||= f2()
		test_assert_equal(true, x)
		test_assert_equal(0, $a)
		x = false ; $a = 0
		x &&= f2()
		test_assert_equal(false, x)
		test_assert_equal(0, $a)
		x = false ; $a = 0
		x ||= f2()
		test_assert_equal(false, x)
		test_assert_equal(1, $a)
	}

	def hytest_substOp_array ()
	{
		y = []
		y[0] = true ; $a = 0
		y[0] &&= f1()
		test_assert_equal(true, y[0])
		test_assert_equal(1, $a)
		y[1] = true ; $a = 0
		y[1] ||= f1()
		test_assert_equal(true, y[1])
		test_assert_equal(0, $a)
		y[2] = false ; $a = 0
		y[2] &&= f1()
		test_assert_equal(false, y[2])
		test_assert_equal(0, $a)
		y[3] = false ; $a = 0
		y[3] ||= f1()
		test_assert_equal(true, y[3])
		test_assert_equal(1, $a)

		z = 3
		y[z] = true ; $a = 0
		y[z] &&= f2()
		test_assert_equal(false, y[z])
		test_assert_equal(1, $a)
		z = 1
		y[z] = true ; $a = 0
		y[z] ||= f2()
		test_assert_equal(true, y[z])
		test_assert_equal(0, $a)
		z = 2
		y[z] = false ; $a = 0
		y[z] &&= f2()
		test_assert_equal(false, y[z])
		test_assert_equal(0, $a)
		z = 0
		y[z] = false ; $a = 0
		y[z] ||= f2()
		test_assert_equal(false, y[z])
		test_assert_equal(1, $a)
	}
	

end
