require "hyUnit"

class HyTest_hash < HyUnit

	def hytest_hash ()
	{
		h = new Hash()
		h[1] = 2.0
		h[2.0] = :3
		h[:3] = "4"
		h["4"] = 5.getClass()
		h[5.getClass()] = "６"
		a = [7]
		h["６"] = a
		h[a] = 99


		x = 1
		b = []
		while (x <> nil) {
			x = h[x]
			b.push(x)
		}
		test_assert_equal([2.0, :3, "4", Int, "６", a, 99, nil], b)
		test_assert_equal(99, h[a])
		a[1] =11
		test_assert_equal(nil, h[a])
		h.rehash()
		test_assert_equal(99, h[a])
		
		keys = h.keys()
		test_assert(keys.include?(1))
		test_assert(keys.include?(2.0))
		test_assert(keys.include?(:3))
		test_assert(keys.include?("4"))
		test_assert(keys.include?(Int))
		test_assert(keys.include?("６"))
		test_assert(! keys.include?([7]))
		test_assert(keys.include?(a))
		test_assert(! keys.include?(99))

		values = h.values()
		test_assert(! values.include?(1))
		test_assert(values.include?(2.0))
		test_assert(values.include?(:3))
		test_assert(values.include?("4"))
		test_assert(values.include?(Int))
		test_assert(values.include?("６"))
		test_assert(values.include?(a))
		test_assert(! values.include?([7]))
		test_assert(values.include?(99))
	}

end

