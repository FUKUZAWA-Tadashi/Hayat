require "hyUnit"

class HyTest_bitArray < HyUnit

	def hytest_bitArray ()
	{
		ba = new BitArray(0)
		test_assert_equal(0, ba.size())
		test_assert_throw_type(:array_index_out_of_bounds){ba[-1]}
		test_assert_throw_type(:array_index_out_of_bounds){ba[0]}
		ba.changeSize(20)
		test_assert_equal(20, ba.size())
		test_assert_equal(false, ba[0])
		test_assert_equal(false, ba[10])
		test_assert_equal(false, ba[19])
		test_assert_throw_type(:array_index_out_of_bounds){ba[20]}
		ba.setAll(true)
		test_assert_equal(true, ba[0])
		test_assert_equal(true, ba[10])
		test_assert_equal(true, ba[19])
		ba[3],ba[10],ba[15] = false,false,false
		test_assert_equal(false, ba[3])
		test_assert_equal(false, ba[10])
		test_assert_equal(true, ba[14])

		ba.setAll(true)
		ba2 = ba.clone()
		test_assert(ba == ba2)
		test_assert_equal(ba, ba2)

		h = {}
		h[ba] = 123
		test_assert_equal(123, h[ba2])

		ba.setRange(1,18,false)
		ba.setRange(2,17,true)
		ba.setRange(3,16,false)
		ba2.setAll(false)
		ba2[0],ba2[19] = true,true
		ba2[2],ba2[17] = true,true
		test_assert(ba == ba2)

		ba.changeSize(15)
		ba.changeSize(20)

		test_assert(ba != ba2)
		test_assert_equal(false, ba[17])
		test_assert_equal(false, ba[19])

		ba[1] = nil
		test_assert_equal(false, ba[1])
		ba[1] = 123
		test_assert_equal(true, ba[1])
		ba[1] = false
		test_assert_equal(false, ba[1])
		ba[1] = ba
		test_assert_equal(true, ba[1])
	}

end
