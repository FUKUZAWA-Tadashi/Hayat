// -*- coding: utf-8 -*-

require "hyUnit"

class HyTest_enumerable < HyUnit

	def gen4 ()
	{
		new Fiber {
			ret = nil
			try {
				for (i = 1; true; i += 1) {
					yield i
				}
			} catch (e) {
				ret = e.val()
			}
			return ret
		}
	}

	def hytest_toList ()
	{
		a = '(5,4,3,2,1)
		b = a.map(lambda(x){x*2}).toList()
		test_assert_equal('(10,8,6,4,2), b)
	}


	defFiber gadd (n)
	{
		g = 0
		n.times{|i|
			g += yield i
		}
		g
	}
	def hytest_next ()
	{
		x = gadd(5)->{|i|
			if (i == 2) {
				100
				next			// nextは直前の値をyieldに運ぶ
			}
			i * 2
		}
		test_assert_equal(0+2+100+6+8, x)
	}


	def hytest_map ()
	{
		a = 5.times().map(lambda(x){x * 2}).toArray()
		test_assert_equal([0,2,4,6,8], a)
		a = 6.times().map{|x|x * 3}.toArray()
		test_assert_equal([0,3,6,9,12,15], a)

		y = 0
		a = gen4().map{|x|
			if (x == 4) throw "d"
			x * 2
		}->{|x|
			y += x
			if (x == 4) throw "e"
		}
		test_assert_equal("e", a)
		test_assert_equal(2+4, y)
		y = 0
		a = gen4().map{|x|
			if (x == 4) throw "f"
			x * 2
		}->{|x|
			y += x
			if (x == 12) throw "g"
		}
		test_assert_equal("f", a)
		test_assert_equal(2+4+6, y)

		y = 0
		a = gen4().map(lambda(x){
						   if (x == 5) break
						   x * 2
					   })->{|z|
			y += z
			if (z >= 6) break
		}
		test_assert_equal(2+4+6, y)
		y = 0
		a = gen4().map(lambda(it){
						   if (it == 5) break
						   it * 2
					   })->{|it|
			y += it
			if (it >= 10) break
		}
		test_assert_equal(2+4+6+8, y)

		x = gen4.map{|i|
			if (i == 3) {
				"3"
				next
			}
			if (i == 5) break
			i * 2
		}.toArray()
		test_assert_equal([2,4,8], x)

		y = 0
		a = gen4().map(lambda(i){
						   if (i == 5) next
						   i * 2
					   })->{|j|
			if (j == 6) next
			y += j
			if (j >= 8) break
		}
		test_assert_equal(2+4+8, y)
		y = 0
		a = gen4().map{|i|
			if (i == 5) next
			i * 2
		}->{|j|
			if (j == 4) next
			y += j
			if (j >= 12) break
		}
		test_assert_equal(2 +6+8 +12, y)

		y = 0
		f1 = true
		f2 = true
		a = gen4().map{|it|
		  label :redo
			if (it == 5 && f1) {
				f1 = false
				goto :redo
			}
			it * 2
		}->{|it|
		  label :redo
			y += it
			if (it >= 6 && f2) {
				f2 = false
				goto :redo
			}
			if (it >= 14) break
		}
		test_assert_equal(2+4+6+6+8+10+12+14, y)
		y = 0
		f1 = true
		f2 = true
		a = gen4().map{|i|
		  label :redo
			if (i == 5 && f1) {
				f1 = false
				goto :redo
			}
			i * 2
		}->{|j|
		  label :redo
			y += j
			if (j >= 10 && f2) {
				f2 = false
				goto :redo
			}
			if (j >= 14) break
		}
		test_assert_equal(2+4+6+8+10+10+12+14, y)
	}

	def hytest_filter ()
	{
		a = 6.times().filter(lambda(x){x % 2 == 0}).toArray()
		test_assert_equal([0,2,4], a)
		a = 6.times().filter{|x|x % 3 <> 0}.toArray()
		test_assert_equal([1,2,4,5], a)

		y = 0
		a = gen4().filter{|x|
			if (x == 7) throw "h"
			x % 2 == 0
		}->{|z|
			y += z
			if (z == 4) throw "i"
		}
		test_assert_equal("i", a)
		test_assert_equal(2+4, y)
		y = 0
		a = gen4().filter(lambda(i){
							  if (i == 7) throw "j"
							  i % 2 == 0
						  })->{|j|
			y += j
			if (j == 10) throw "k"
		}
		test_assert_equal("j", a)
		test_assert_equal(2+4+6, y)

		y = 0
		a = gen4().filter{|i|
			if (i == 7) break
			i % 2 == 0
		}->{|j|
			y += j
			if (j == 4) break
		}
		test_assert_equal(nil, a)
		test_assert_equal(2+4, y)
		y = 0
		a = gen4().filter{|x|
			if (x == 7) break
			x % 2 == 0
		}->{|x|
			y += x
			if (x == 10) break
		}
		test_assert_equal(nil, a)
		test_assert_equal(2+4+6, y)

		y = 0
		a = gen4().filter{|x|
			if (x == 7) next
			x % 2 == 0
		}->{|x|
			if (x == 6) next
			y += x
			if (x >= 10) break
		}
		test_assert_equal(nil, a)
		test_assert_equal(2+4 +8+10, y)
		y = 0
		a = gen4().filter(lambda(x){
							  if (x == 6) next
							  x % 2 == 0
						  })->{|x|
			if (x == 8) next
			y += x
			if (x == 12) break
		}
		test_assert_equal(nil, a)
		test_assert_equal(2+4 +10+12, y)

		y = 0
		f1 = true
		f2 = true
		a = gen4().filter{|x|
		  label :redo
			if (x == 7 && f1) {
				f1 = false
				goto :redo
			}
			x % 2 == 0
		}->{|x|
		  label :redo
			y += x
			if (x == 4 && f2) {
				f2 = false
				goto :redo
			}
			if (x == 10) break
		}
		test_assert_equal(nil, a)
		test_assert_equal(2+4+4+6+8+10, y)
		y = 0
		f1 = true
		f2 = true
		a = gen4().filter{|i|
		  label :redo
			if (i == 7 && f1) {
				f1 = false
				goto :redo
			}
			i % 2 == 0
		}->{|j|
		  label :redo
			y += j
			if (j == 8 && f2) {
				f2 = false
				goto :redo
			}
			if (j == 10) break
		}
		test_assert_equal(nil, a)
		test_assert_equal(2+4+6+8+8+10, y)
	}

	def hytest_find ()
	{
		test_assert_equal(5, 10.times().find{|x| x == 5})
		test_assert_equal(9, 10.times().find{|x| x == 9})
		test_assert_equal(nil, 10.times().find{|x| x == 12})
		test_assert_equal(nil, 10.times().find{|x| x == -3})
	}

	def hytest_count ()
	{
		test_assert_equal(1, 10.times().count(5))
		test_assert_equal(4, 10.times().count{|i| i > 5})
		a = [1, nil, 5, 10, nil, "a", 1, 10]
		test_assert_equal(6, a.count())
		test_assert_equal(2, a.count(nil))
		test_assert_equal(2, a.count(1))
		test_assert_equal(1, a.count(5))
		test_assert_equal(5, a.count{|x|x.getClass() == Int})
	}

	def hytest_most ()
	{
		a = [2,1,3,9,5,7]
		test_assert_equal(9, a.most())
		test_assert_equal(1, a.most{|a,b| a < b})
		test_assert_equal(9, a.most{|a,b| a > b})
		test_assert_equal(3, a.most{|a,b| (a%4) > (b%4)})
		test_assert_equal(7, a.most{|a,b| (a%4) >= (b%4)})
	}

	def hytest_flatten ()
	{
		test_assert_equal([1,2,3,4,5,6,7], [[1,2],[3,4],5,[6,7]].flatten().toArray())
		test_assert_equal([1,2,3,4,5,6], [[1,2],[3,[4]],[[5]],6].flatten().toArray())
		test_assert_equal([0,1,2,0,2,4],
						  [3.times(),
							  10.times().map{|x|x*2}.take(3)
						  ].flatten().toArray())
		test_assert_equal([1,2,3,4],
						  '('(1,2),'(3,'(4))).flatten().toArray())

	}

end
