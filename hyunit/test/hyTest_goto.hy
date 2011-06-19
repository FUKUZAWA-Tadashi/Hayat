require "hyUnit"

label :a1
// goto :c1 // 呼出しの外側に :c1 という label は無い
// goto :m2 // 呼出しの外側に :m2 という label は無い

class GotoTest1
  label :c1
	// goto :a1 // クラス初期化時に :a1 にジャンプしてしまい、2回目の初期化時
				// には初期化済み扱いになっているので以下のコードを実行しない
	// goto :m1 // 呼出しの外側に :m1 という label は無い
	def test ()
	{
		// goto :a2 // test()を呼び出した場所にある :a2 にジャンプ
		a = 0
		i = 0
	  label :m1
		j = 0
		// goto :c1 // このクラスの :c1 ではなく呼出し外側の :c1 にジャンプ
		// goto :c2 // このクラスの :c2 ではなく呼出し外側の :c2 にジャンプ
		while (i < 10) {
			i += 1
			if (i == 3)
				goto :m1
			if (i == 5)
				goto :m9

		  label :m2
			a += 1
			while (j < 10) {
				a += 100
				j += 1
				if (j == 4)
					goto :m2
				if (j == 8)
					goto :m1
			}
		}
	  label :m9
		return a

	}
  label :c2

end

label :a2





class HyTest_goto < HyUnit

	def hytest_goto_1 ()
	{
		test_assert_equal(3*2 + 3*8*100, GotoTest1.test())
	}

	def hytest_goto_2 ()
	{
		x = 0
		i = 0
	  label :a3
		while (i < 6) {
			try {
			  label :t1
				try {
					i += 1
					x += 1
				  label :s1
					if (i == 2)
						goto :t2
					throw 1
				} catch (e) {
					if (e.type() == :goto) {
						test_assert_equal(:t2, e.getLabel())
						goto e.getLabel()
					}
					x += 100
				}
			  label :t2
				//if (i == 4)
				//	  goto :s1 // エラー：外側に :s1 は無い
				if (i == 4) {
					goto :t1
				}
				x += 10
			} catch {
				if (e.type() == :goto) {
					test_assert_equal(:t1, e.getLabel())
					goto e.getLabel()
				}
				x += 10000
			}
		}
		test_assert_equal(556, x)
		goto :eom
		test_fail()
	  label :eom
	}


	def goout ()
	{
		return lambda () {
			goto :goout
		}
	}

	def hytest_loopout1 ()
	{
		a = 0
		10.times{|i|
			if (i == 3) next
			a += i
			if (i == 5) break
		}
		test_assert_equal(0+1+2+4+5, a)
	}

	def hytest_loopout2 ()
	{
		closure = goout()
		a = 1
	  label :goout
		a += 2
		if (a < 10) closure() // goto :goout
		test_assert_equal(11, a)
	}

	def hytest_loopout3 ()
	{
		closure = goout()
		a = 1
	  label :goout
		a += 2
		if (a < 8) {
			try {
				closure()	// goto :goout
			} catch (e) {
				if (e.type() == :goto) {
					test_assert_equal(:goout, e.getLabel())
					goto e.getLabel()
				}
				test_fail()
//			} finally {
//				a += 3
			}
		}
		//test_assert_equal(8, a)
		test_assert_equal(9, a)
	}

	def onGoto(n, garr)
	{
		if (n < garr.size())
			goto garr[n]
	}

	def hytest_ongoto ()
	{
		5.times{|i|
			onGoto(i, [:g1, :g2, :g3])
			test_assert(i >= 3)
			next

		  label :g1
			test_assert_equal(0, i)
			next

		  label :g3
			test_assert_equal(2, i)
			next

		  label :g2
			test_assert_equal(1, i)
		}
	}

	def gotoVal (n)
	{
		if (n < 3) {
			n * 10
			goto :gotoValTest	// n*10 を値とする
		}
		if (n > 6) {
			goto :gotoValTest	// 値が無いので nil
		}
		n + 100					// n+100 を値とする
	  label :gotoValTest
	}
	def hytest_gotoVal ()
	{
		10.times{|i|
			if (i < 3) {
				test_assert_equal(10 * i, gotoVal(i))
			} else if (i > 6) {
				test_assert_equal(nil, gotoVal(i))
			} else {
				test_assert_equal(100 + i, gotoVal(i))
			}
		}
	}


	def breakVal (n)
	{
		for (i = 0; i < 10; i += 1) {
			if (i == n) {
				n * 10
				break			// n*10 を値とする
			}
			if (i + 10 == n)
				break			// 値が無いので nil
		}
	}			
	def hytest_breakVal ()
	{
		test_assert_equal(30, breakVal(3))
		test_assert_equal(60, breakVal(6))
		test_assert_equal(nil, breakVal(15))
		test_assert_equal(nil, breakVal(20))
	}

	def hytest_getLabel()
	{
		try {
			goto :bar
		} catch (e) {
			test_assert_equal(:bar, e.getLabel())
			goto :a1
		}
		test_fail()
	label :a1
		try {
			1 / 0
		} catch (e) {
			test_assert_equal(nil, e.getLabel())
			goto :a2
		}
	label :bar
		test_fail()
	label :a2
		test_ok()
	}

	def hytest_loop_next ()
	{
		i = 0
		loop {
			i += 1
			if (i < 5)
				next
			break
		}
		test_assert_equal(5, i)

		for (j = 0; j < 10; j += 1) {
			if (j < 5)
				next
		}
	}

end
