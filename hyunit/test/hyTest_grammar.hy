// -*- coding: utf-8 -*-

require "hyUnit"


class A
	def initialize(x) { @a = x }
	def fadd(x) { @a += x; self }
	def fsub(x) { @a -= x; self }
	def fmul(x) { @a *= x; self }
	def fdiv(x) { @a /= x; self }
end

class HyTest_grammar < HyUnit

	def hytest_grammar ()
	{
		a = new A(10)
		a.fadd(10)
		test_assert_equal(20, a.a)
		a  .fsub(2)				// .の前のスペースはOK
		test_assert_equal(18, a .a)
		a .fmul(5)     .fdiv(2)
		test_assert_equal(45, a		.a)
		a   .fadd(34)  \
		    .fsub(23)  \
		    .fmul(3)
		// 行末の \ で行継続した。 \ の後ろにスペースやコメントが入るとダメ
		test_assert_equal(168, a	\
						  .a)
	}

	def swfn(x) {
		switch(x) {
		case 1,2
			x
		case 3 ; x + 2
		case 4
		case 5,6 ; x + 3
		case 7,8 {
				x + 4
			}
		case 9 {}
		case 10
			{
				x + 10
			}
		case 11


			{
			}
		case 12,
			13
			x + 11; x + 12
		case 14;
		case 15 ; 

		case 16,17; x+1; x+2; case 18

		default 99
		}
	}
	def hytest_switch ()
	{
		v = [99, 1,2, 5, nil, 8,9, 11,12, nil, 20, nil, 24,25, nil,nil, 18,19, nil, 99]
		w = 20.times.map{|x| swfn(x)}.toArray()
		test_assert_equal(v, w)
	}


	def compilableCheck ()
	{
		//switch (0) {
		//}		//ERROR: 'case' statement required

		//switch (0) {
		//default			//ERROR: 'case' statement required
		//}

		switch (0) {
		case 1;
		case 2;2
		// case 3 3		//ERROR: separator required after case expression(s)
		// case 4 case 5    //ERROR: separator required after case expression(s)
		// case			//ERROR: expression(s) required after 'case'
		case 6; case 7
		case 8; case 9;;
		case 10
			;
		case 11

		case 12}

		switch (0) {
		case 1;}
		switch (0) {
		case 1
		}
		switch (0) {
		case 1;
		}
		switch (0) {
		case 1
			;
		}

		switch (0) {
		case 1
		default}
		switch (0) {
		case 1
		default;}
		switch (0) {
		case 1
		default
		}
		switch (0) {
		case 1
		default;
		}
		switch (0) {
		case 1
		default
			;
		}
	}
end
