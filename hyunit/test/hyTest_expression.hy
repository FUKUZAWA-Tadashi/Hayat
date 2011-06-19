require "hyUnit"


class HyTest_expression < HyUnit

	defFiber boolPat (n)
	{
		a = []
		n.times{a.push(false)}
		yield a
		loop {
			for (i = 0; i < n; i += 1) {
				a[i] = ! a[i]
				if (a[i]) goto :cont
			}
			return
			label :cont
			yield a
		}
	}

	def test_boolPat_eq (n, f1, f2)
	{
		boolPat(n)->{|a|
			x1 = f1(a)
			x2 = f2(a)
			if (x1 != x2) {
				throw :test_boolPat_eq, [x1, x2, a]
			}
		}
		GC.full()
	}

	def test_boolPat_exist_ne (n, f1, f2)
	{
		boolPat(n)->{|a|
			x1 = f1(a)
			x2 = f2(a)
			if (x1 != x2) {
				test_ok()
				return
			}
		}
		test_fail()
	}


	defFiber intPat(n, arr)
	{
		if (n < 2) {
			arr.each{|x| yield [x]}
			return
		}
		arr.each{
			|x|
			intPat(n-1, arr)->{|y|
				y.push(x)
				yield y
			}
		}
	}

	def test_intPat_eq (n, arr, f1, f2)
	{
		intPat(n, arr)->{|a|
			x1 = f1(a)
			x2 = f2(a)
			if (x1 != x2) {
				throw :test_intPat_eq, [x1, x2, a]
			}
		}
		GC.full()
	}


	def hytest_priority ()
	{
		try {
			test_boolPat_eq(4){
				|x|
				(x[0] and x[1]) or (x[2] and x[3])
			}{
				|y|
				y[0] and y[1] or y[2] and y[3]
			}
			test_boolPat_exist_ne(4){|x|
				x[0] and (x[1] or x[2]) and x[3]
			}{|y|
				y[0] and y[1] or y[2] and y[3]
			}

			test_boolPat_eq(2){
				|x| (not x[0]) and x[1]
			}{
				|y| not y[0] and y[1]
			}
			test_boolPat_exist_ne(2){|x|
				not (x[0] and x[1])
			}{|y|
				not y[0] and y[1]
			}

			test_boolPat_eq(3){|x|
				not (x[0] ? x[1] : x[2])
			}{|y|
				not y[0] ? y[1] : y[2]
			}
			test_boolPat_exist_ne(3){|x|
				(not x[0]) ? x[1] : x[2]
			}{|y|
				not y[0] ? y[1] : y[2]
			}

			test_boolPat_eq(5){|x|
				x[0] ? x[1] : (x[2] ? x[3] : x[4])
			}{|y|
				y[0] ? y[1] : y[2] ? y[3] : y[4]
			}
			test_boolPat_exist_ne(5){|x|
				(x[0] ? x[1] : x[2]) ? x[3] : x[4]
			}{|y|
				y[0] ? y[1] : y[2] ? y[3] : y[4]
			}

			test_boolPat_eq(5){|x|
				x[0] ? x[1] : ((x[2] || x[3]) ? x[4] : x[5])
			}{|y|
				y[0] ? y[1] : y[2] || y[3] ? y[4] : y[5]
			}
			test_boolPat_exist_ne(5){|x|
				(x[0] ? x[1] : x[2]) || (x[3] ? x[4] : x[5])
			}{|y|
				y[0] ? y[1] : y[2] || y[3] ? y[4] : y[5]
			}
			test_boolPat_exist_ne(5){|x|
				x[0] ? x[1] : (x[2] || (x[3] ? x[4] : x[5]))
			}{|y|
				y[0] ? y[1] : y[2] || y[3] ? y[4] : y[5]
			}
			test_boolPat_exist_ne(5){|x|
				((x[0] ? x[1] : x[2]) || x[3]) ? x[4] : x[5]
			}{|y|
				y[0] ? y[1] : y[2] || y[3] ? y[4] : y[5]
			}

			test_boolPat_eq(4){|x|
				(x[0] && x[1]) || (x[2] && x[3])
			}{|y|
				y[0] && y[1] || y[2] && y[3]
			}
			test_boolPat_exist_ne(4){|x|
				x[0] && (x[1] || x[2]) && x[3]
			}{|y|
				y[0] && y[1] || y[2] && y[3]
			}
		} catch (e) {
			if (e.type() == :test_boolPat_eq) {
				v = e.val()
				warning("expected value = ", v[0],
						" , but actual = ", v[1],
						" , test pattern = ", v[2])
				warning(e.backTraceStr(e.backTraceSize()-1))
				test_fail()
			} else {
				throw e
			}
		}



		try {

			test_intPat_eq(4, [-3, 0, 2, 5]){|x|
				(x[0] < x[1]) && (x[2] >= x[3])
			}{|y|
				y[0] < y[1] && y[2] >= y[3]
			}

			test_intPat_eq(4, [0x07, 0x0b, 0x15, 0x1e, 0x19]){|x|
				(x[0] & x[1]) | (x[2] & x[3])
			}{|y|
				y[0] & y[1] | y[2] & y[3]
			}
			
			test_intPat_eq(4, [-5, -2, 0, 3, 8, 15]){|x|
				(x[0] + x[1]) & (x[2] - x[3])
			}{|y|
				y[0] + y[1] & y[2] - y[3]
			}

			test_intPat_eq(4, [-5, -2, 0, 3, 8, 15]){|x|
				(x[0] * x[1]) + (x[2] * x[3])
			}{|y|
				y[0] * y[1] + y[2] * y[3]
			}

			test_intPat_eq(4, [0,2,3,4]){|x|
				(x[0] ** x[1]) * (x[2] ** x[3])
			}{|y|
				y[0] ** y[1] * y[2] ** y[3]
			}

			test_intPat_eq(3, [0,2,3,4]){|x|
				x[0] ** (x[1] ** x[2])
			}{|y|
				y[0] ** y[1] ** y[2]
			}

			test_intPat_eq(2, [0,1,2,3,4]){|x|
				(-x[0]) ** (+x[1])
			}{|y|
				-y[0] ** +y[1]
			}				
		} catch (e) {
			if (e.type() == :test_intPat_eq) {
				v = e.val()
				warning("expected value = ", v[0],
						" , but actual = ", v[1],
						" , test pattern = ", v[2])
				warning(e.backTraceStr(e.backTraceSize()-1))
				test_fail()
			} else {
				throw e
			}
		}

		test_assert_equal(10, 6 + 2 * 3 - 8 / 2 ** 2)

	}



	def hytest_controls ()
	{
		f = lambda(x) { if (x) 1 }
		test_assert_equal(1, f(true))
		test_assert_equal(nil, f(false))

		f = lambda(x) {
			if (x) goto :hoge
			else 2
			label :hoge
		}
		test_assert_equal(nil, f(true))
		test_assert_equal(2, f(false))

		f = lambda(x) {
			if (x) { 3; goto :hoge }
			4
			label :hoge
		}
		test_assert_equal(3, f(true))
		test_assert_equal(4, f(false))

		f = lambda(i,x) {
			while (i < x) {
				if (i == 1) { 5; break }
				i += 1
				999
			}
		}
		test_assert_equal(5, f(0,3))
		test_assert_equal(nil, f(5,3))
		test_assert_equal(nil, f(3,5))

		f = lambda(x) {
			for (i = 0; i < 3; i += 1) {
				if (i == x) { 6; break }
				7
			}
		}
		test_assert_equal(6, f(1))
		test_assert_equal(nil, f(5))

		x,y = 0,0
		if (x == 0)
			y += 10;
		// ↑セミコロンが最後にあっても、

		// コメントがelseの前にあっても、

		// ↓このelseは前のifに対応している。
		else
			y += 3;

		y += 1;
		test_assert_equal(11, y)
	}

	def hytest_not_nil ()
	{
		if (nil) test_fail(); else test_ok()
		if (! nil) test_ok(); else test_fail()
		if (!! nil) test_fail(); else test_ok()

		if (nil == nil) test_ok(); else test_fail()
		if (NilClass == nil) test_fail(); else test_ok()
		if (nil == NilClass) test_fail(); else test_ok()
		if (NilClass == NilClass) test_ok(); else test_fail()
	}

	def hytest_bit_calc ()
	{
		a = 0b10011101
		b = 0b10100110
		test_assert_equal(0b10000100, a & b)
		test_assert_equal(0b10111111, a | b)
		test_assert_equal(0b00111011, a ^ b)
		test_assert_equal(0b01100010, ~a & 0xff)
	}

	def hytest_str_comp ()
	{
		test_assert_equal(0, "aaa" <=> "aaa")
		test_assert(("aaa" <=> "aab") < 0)
		test_assert(("aaba" <=> "aaab") > 0)

		test_assert_equal(0, "aaa" <=> <<aaa>>)
		test_assert(("aaa" <=> <<aab>>) < 0)
		test_assert(("aaba" <=> <<aaab>>) > 0)

		test_assert_equal(0, <<aaa>> <=> "aaa")
		test_assert((<<aaa>> <=> "aab") < 0)
		test_assert((<<aaba>> <=> "aaab") > 0)

		test_assert_equal(0, <<aaa>> <=> <<aaa>>)
		test_assert((<<aaa>> <=> <<aab>>) < 0)
		test_assert((<<aaba>> <=> <<aaab>>) > 0)
	}

	def hytest_emptyStatement ()
	{
		if (false)
			;
		else
			test_ok()

		f = lambda(){
			if (true) ;
		}
		test_assert_equal(nil, f())

		f = lambda(x){ if (x) ; else ; }
		test_assert_equal(nil, f(true))
		test_assert_equal(nil, f(false))

		f = lambda(){
			100
			;
		}
		test_assert_equal(100, f())
	}

	;
	;  // 空文
	;

end

;
;  // 空文
;

