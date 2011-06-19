require "hyUnit"

class HyTest_multiSubst < HyUnit

	// 整数とFiberをgenerateするFiber
	defFiber func (i)
	{
		yield i
		yield func(i+1)
	}

	def hytest_multiSubst ()
	{
		a = 0 ; b = 0
		test_assert_no_throw(lambda{
								 a, b = 1, '(2, '(3, "33"), 4)
							 })
		test_assert_equal(1, a)
		test_assert_equal('(2, '(3, "33"), 4), b)

		test_assert_throw_type(:multisubst_failed) {
			// 代入できないので例外発生
			a, b = 1, 2, 3
		}

		test_assert_throw_type(:multisubst_failed){
			// 代入できないので例外発生
			a, b = 1
		}

		a = 0 ; b = 0
		m = '(1, 2)
		a, b = *m				// 右辺に * 使用
		test_assert_equal(1, a)
		test_assert_equal(2, b)
		'(b, a) = m				// 左辺が1レベルネスティング
		test_assert_equal(2, a)
		test_assert_equal(1, b)
		'('(a, b)) = '(m)		// ネスティング
		test_assert_equal(1, a)
		test_assert_equal(2, b)
		'('(b, a)) = '('(*m))	// ネスティング
		test_assert_equal(2, a)
		test_assert_equal(1, b)

		a, b = b, a				// swap
		test_assert_equal(1, a)
		test_assert_equal(2, b)

		'(a,b) = '(3,4)			// ネスティング
		test_assert_equal(3, a)
		test_assert_equal(4, b)

		'(a,*b) = '(5,6,7)		// 左辺の * は残りをまとめて受け取る
		test_assert_equal(5, a)
		test_assert_equal('(6,7), b)
		// test_assert_equal('(6,7), b.toList())

		'(a, b, c) = 3.times()	// 右辺がfiber
		test_assert_equal(0, a)
		test_assert_equal(1, b)
		test_assert_equal(2, c)
		c, a, b = *3.times()	// 右辺が*fiber
		test_assert_equal(1, a)
		test_assert_equal(2, b)
		test_assert_equal(0, c)

		test_assert_throw_type(:multisubst_failed){
			// 数が合わないので代入できない
			a, b, c = *2.times()
		}
		test_assert_throw_type(:multisubst_failed){
			// 数が合わないので代入できない
			'(a, b, c) = 4.times()
		}

		a,b,c = 1,2.times(),4
		test_assert_equal(1, a)
		test_assert_equal('(0,1), b.toList())
		test_assert_equal(4, c)

		a, '(b, '(c, '(d, *e))) = *func(1) // fiberのネスティング
		test_assert_equal(1, a)
		test_assert_equal(2, b)
		test_assert_equal(3, c)
		test_assert_equal(4, d)
		// e は yield 4 を実行した直後の状態の fiber
		f = e.generate_noThrow()
		// f は func(5)
		test_assert_equal(5, f.generate_noThrow())
		a = f.generate_noThrow()
		test_assert_equal(6, a.generate_noThrow())
		b = a.generate_noThrow()
		test_assert_equal(7, b.generate_noThrow())
	}

	def hytest_match ()
	{
		match (1,2,3) {
		case (a,b,c,d) test_fail()
		case (a,b) test_fail()
		case (a,b,c,*d)
			test_assert_equal(1, a)
			test_assert_equal(2, b)
			test_assert_equal(3, c)
			test_assert_equal('(), d)
		case (a,b,c)  // empty
		default test_fail()
		}		
		x = '(3, '(4,5))
		match (1,2,*x) {
		case (a,b,c,d) 
			{
				test_assert_equal(1,a)
				test_assert_equal(2,b)
				test_assert_equal(3,c)
				test_assert_equal('(4,5),d)
			}
		default
			test_fail()
		}
		match (1,2,*x) {
		case (*a)
			n = a.each()
			test_assert_equal(1,n.generate_noThrow())
			test_assert_equal(2,n.generate_noThrow())
			test_assert_equal(3,n.generate_noThrow())
			test_assert_equal('(4,5),n.generate_noThrow())
			test_assert_no_throw(lambda{
									 '() = n
								 })

		default test_fail()
		}


		x = 4.times()
		for (i = 0; true; i += 1) {
			match (*x) {
			case (a,*b) {
					test_assert_equal(i, a)
					x = b
				}
			case () {
					test_assert_equal(4, i)
					break
				}
			default test_fail()
			}
		}


		x = '(1,2,'(3,4,3.times(),5,6))
		match (*x) {
		case (a,b,'(c,d,'(e,f,'(g,*h),i))) test_fail()
		case (a,b,'(*c))
			test_assert_equal(1,a)
			test_assert_equal(2,b)
			match (c) {
			case ('(d,e,f,g)) test_fail()
			case ('(d,e,'(*f),*g))
				test_assert_equal(3,d)
				test_assert_equal(4,e)
				match (f) {
				case ('(h,i)) test_fail()
				case ('(h,i,*j))
					{
						test_assert_equal(0,h)
						test_assert_equal(1,i)
						match (*j) {
						case (k,l) test_fail()
						case () test_fail()
						case (k) test_assert_equal(2,k)
						default test_fail()
						}
					}
				default test_fail()
				}
				match (*g) {
				case (h,*i) {
						test_assert_equal(5,h)
					}
				default test_fail()
				}
			default test_fail()
			}
		default test_fail()
		}


		x = '(1, '(), 2)		// 途中に '() がある
		match (*x) {
		case (a, '(c), b) test_fail()
		case (a, '(), b)
			test_assert_equal(1, a)
			test_assert_equal(2, b)
		default test_fail()
		}
	}

	def testFunc1 (x,f)
	{
		match (*x) {
		case (a,*b)
			List.cons( f(a), testFunc1(b,f) )
		case ()
			'()
		}
	}

	def hytest_matchFunc ()
	{
		test_assert_equal('(2,4,6),
						  testFunc1('(1,2,3),
									lambda(x){x*2}))
		test_assert_equal('(0,3,6),
						  testFunc1(3.times()){args_.each().generate()*3})
		
	}


	def testFunc2 (x,'(y,*z))
	{
		a = x * 100 + y * 10
		z.each{|i| a += i}
		a
	}
	def testFunc3 (x,y) { y }

	def testFunc4 (x,y,z=10) { y }

	def hytest_method_multiarg ()
	{
		test_assert_equal(127, testFunc2(1,'(2,3,4)))
		test_assert_equal(234, testFunc2(2,[3,4]))
		test_assert_equal(340, testFunc2(3,[4]))
		a = [1,[3,5]]
		test_assert_equal(135, testFunc2(*a))
		a = [2,4]
		test_assert_equal(126, testFunc2(1,'(2,*a)))
		test_assert_equal(123 , testFunc2(1,'(2).append('(3))))

		test_assert_equal('(2,3), testFunc3(1,'(2,3)))
		test_assert_equal([2,3], testFunc3(1,[2,3]))
		test_assert_equal(3, testFunc3(*'(2,3)))

		test_assert_equal('(2,3), testFunc4(1,'(2,3),4))
		test_assert_equal([2,3], testFunc4(1,[2,3],4))
		test_assert_equal(3, testFunc4(*'(2,3,4)))
		test_assert_equal(3, testFunc4(*'(2,3)))
	}

end
