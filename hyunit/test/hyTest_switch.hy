require "hyUnit"

class HyTest_switch < HyUnit

	def sw1 (x)
	{
		switch (x) {
		case 1,2
			1
		case 3
			,4
			2
		case 5,6; 3
		case 7; 4; case 8; 4
		default 5
		}
	}

	def hytest_switch_1 ()
	{
		a = 10.times().map{|x|sw1(x)}.toArray()
		test_assert_equal([5,1,1,2,2,3,3,4,4,5], a)
	}

	def sw2 (x)
	{
		switch (x) {
		case "a"
			,
			"b"
			1
		case "c","d"; 2
		}
	}

	def hytest_switch_2 ()
	{
		sb = <<b>>
		a = [ sw2("a"), sw2("c"), sw2(sb), sw2("e") ]
		test_assert_equal([1,2,1,nil], a)
	}

	def sw3 (x)
	{
		switch (x) {
		case 1,2,3
			y = x + 2
			y *= 2
			y -= 6
		case 4,5,6
			y = x - 2
			y *= 3
			y -= 6
		default
			y = x - 5
			y *= 4
			y -= 8
		}
	}

	def hytest_switch_3 ()
	{
		a = 10.times().map{|x|sw3(x)}.toArray()
		test_assert_equal([-28,0,2,4,0,3,6,0,4,8], a)
	}

	def sw4 (x)
	{
		switch (x) {
		case 1, "1" {
			"a"
			}
		case 2, "2" { "b" }
		case 3, "3"
			{
				1+2
				3+4
				5+6
				"c"
			}
		case 4, "4"
			{ "d" }
		default {
			0+0
			<<e>>
			}
		}
	}
	def hytest_switch_4 ()
	{
		a = [ 1, "2", 3, "4", 5, "5", 4, "3", 2, "1" ]
		b = [ "a", "b", "c", "d", "e", "e", "d", "c", "b", "a"]
		test_assert_equal(b, a.map{|x| sw4(x)}.toArray())
	}

	def sw5 (x)
	{
		switch (x) {
		case 1
			1
		case 2
			2
			// comment

		case 3; 3
		case 4 ; 4

		case 5 ;
			5
			;

		case 6 ;
			6;

		case 11
		case 12

		case 13
			// comment
		case 14
			// comment

		case 15
			;
		case 16
			;
			;
		case 17;
		case 18;

		case 19;
			;

		case 20,21
		case 22,23
			;
		case 24,25;
		case 26,27;

		case 28,29;;;

		default; -1
		}
	}


	def hytest_switch_5 ()
	{
		a = [ -1,1,2,3,4,5,6,-1,-1,-1,
			  -1,nil,nil,nil,nil,nil,nil,nil,nil,nil,
			  nil,nil,nil,nil,nil,nil,nil,nil,nil,nil,
			  -1 ]
			  
		x = 31.times.map{|i| sw5(i)}.toArray
		test_assert_equal(a, x)
	}

end

