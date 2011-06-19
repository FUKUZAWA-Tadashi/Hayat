require "hyUnit"


class HyTest_array < HyUnit

	def hytest_array ()
	{
		a = new Array(4)
		test_assert_equal(0, a.size())

		a[1] = 10
		test_assert_equal(nil, a[0])
		test_assert_equal(10, a[1])

		a[-2] = 20
		a[3] = 40
		test_assert_equal([20,10,nil,40], a)
		test_assert_equal(4, a.size())

		a.clear()
		test_assert_equal(0, a.size())
	}

	def hytest_each ()
	{
		a = [10, 30, 5.5, nil, -20, "f"]
		g = a.each()
		test_assert_equal(10, g.generate())
		test_assert_equal(30, g.generate())
		test_assert_equal(5.5, g.generate())
		test_assert_equal(nil, g.generate())
		test_assert_equal(-20, g.generate())
		test_assert_equal("f", g.generate())
		test_assert_throw_type(:fiber_return){ g.generate() }
		test_assert(g.finished?())
	}
	
	def hytest_remove ()
	{
		a = 10.times().toArray()
		test_assert_equal(4, a.removeAt(4))
		test_assert_equal(7, a.removeAt(6))
		test_assert_equal(2, a.removeAt(2))
		test_assert_equal([0,1,3,5,6,8,9], a)

		test_assert_equal(-1, a.remove(4))
		test_assert_equal(3, a.remove(5))
		test_assert_equal(4, a.remove(8))
		test_assert_equal(-1, a.remove(2))
		test_assert_equal([0,1,3,6,9], a)

		a = [3,1,4,1,5]
		test_assert_equal(1, a.remove(1))
		test_assert_equal([3,4,1,5], a)
	}

	def hytest_removeIf ()
	{
		a = 10.times().toArray()
		a.removeIf{|x| x % 3 == 0}
		test_assert_equal([1,2,4,5,7,8], a)
		a.removeIf{|x| x % 2 == 0}
		test_assert_equal([1,5,7], a)
	}

	def hytest_insert ()
	{
		a = 5.times().toArray()
		test_assert_equal(10, a.insertAt(1, 10))
		test_assert_equal(11, a.insertAt(3, 11))
		test_assert_equal(98, a.insertAt(-1, 98))
		test_assert_equal(99, a.insertAt(a.size(), 99))
		test_assert_equal([0,10,1,11,2,3,98,4,99], a)
	}

	def hytest_find ()
	{
		a = 10.times().toArray()
		test_assert_equal(3, a.find{|x| x == 3})
		test_assert_equal(8, a.find{|x| x == 8})
		test_assert_equal(nil, a.find{|x| x == 10})
		test_assert_equal(nil, a.find{|x| x == -1})
	}

	def hytest_index ()
	{
		a = [3, 1, 4, 1, 5, 9, 2]
		test_assert_equal(1, a.index(1))
		test_assert_equal(4, a.index(5))
		test_assert_equal(nil, a.index(8))
	}

	def hytst_clone ()
	{
		a = [3, 1, 4, 1, 5, 9, 2]
		b = a.clone()
		test_assert_equal(a, b)
		c = b.clone()
		test_assert_equal(a, c)
	}

	def hytest_sort ()
	{
		a = [1,4,2,8,5,7,9]
		b = a.clone().sort!()
		test_assert_equal([1,2,4,5,7,8,9], b)
		b = a.clone().sort!{|x,y| x <=> y}
		test_assert_equal([1,2,4,5,7,8,9], b)
		b = a.clone().sort!{|x,y| -(x <=> y)}
		test_assert_equal([9,8,7,5,4,2,1], b)
	}

	def hytest_sortBy ()
	{
		a = [301,104,402,508,905,207,609]
		b = a.clone().sortBy!{|x| x % 100}
		test_assert_equal([301,402,104,905,207,508,609], b)
		b = a.clone().sortBy!{|x| - x / 10}
		test_assert_equal([905,609,508,402,301,207,104], b)
		b = a.clone().sortBy!{|x| x % 17}
		// [12, 2, 11, 15, 4, 3, 14]
		test_assert_equal([104,207,905,402,301,609,508], b)
		test_assert_no_throw{ [].sortBy!{|x| x} }
	}

	def hytest_push_pop ()
	{
		a = 3.times().toArray()
		test_assert_equal(10, a.push(10))
		test_assert_equal([0,1,2,10], a)
		test_assert_equal(10, a.pop())
		test_assert_equal(2, a.pop())
		test_assert_equal(1, a.pop())
		test_assert_equal([0], a)
	}

	def hytest_shift_unshift ()
	{
		a = [10,11,12]
		test_assert_equal(1, a.unshift(1))
		test_assert_equal(2, a.unshift(2))
		test_assert_equal([2,1,10,11,12], a)
		test_assert_equal(2, a.shift())
		test_assert_equal(1, a.shift())
		test_assert_equal(10, a.shift())
		test_assert_equal([11,12], a)
	}

	def hytest_shuffle ()
	{
		a = 10.times().toArray()
		b = a.clone().shuffle!()
		test_assert(a != b)		// rare case a == b : probable 1/(10!)
		b.sort!()
		test_assert_equal(a, b)
	}

end
