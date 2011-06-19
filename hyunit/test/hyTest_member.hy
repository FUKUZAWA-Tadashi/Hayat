require "hyUnit"

class HyTest_member < HyUnit

	class Hoge
		def initialize (x = 0)
		{
			@m = x
		}

		def mm ()			// getter
		{
			@m
		}

		def mm= (x)			// setter
		{
			@m = x
		}

		def getmm ()
		{
			self.mm				// 自クラスのgetter(mm)呼び出し
		}

		def getmm2 ()
		{
			mm					// 自クラスのgetter(mm)呼び出し
		}

		def setmm (x)
		{
			self.mm = x			// 自クラスのsetter(mm=)呼び出し
		}
	end



	def hytest_member ()
	{
		hoge = new Hoge()
		test_assert_equal(0, hoge.m)
		hoge.m = 1
		test_assert_equal(1, hoge.m)
		hoge.m += 2
		test_assert_equal(3, hoge.m)
		hoge.m, x = 10,20
		test_assert_equal(10, hoge.m)
		x, hoge.m = 10,20
		test_assert_equal(20, hoge.m)
		*hoge.m = 10,20,30
		test_assert_equal([10,20,30], hoge.m.toArray())
	}

	def hytest_setter_getter ()
	{
		hoge = new Hoge(5)

		test_assert_equal(5, hoge.mm)
		hoge.mm = 10
		test_assert_equal(10, hoge.mm)
		hoge.mm += 20
		test_assert_equal(30, hoge.mm)
		hoge.mm, x = 100,200
		test_assert_equal(100, hoge.mm)
		x, hoge.mm = 100,200
		test_assert_equal(200, hoge.mm)
		*hoge.mm = 100,200,300
		test_assert_equal([100,200,300], hoge.mm.toArray())

		hoge.setmm(99)
		test_assert_equal(99, hoge.m)
		test_assert_equal(99, hoge.mm)
		test_assert_equal(99, hoge.getmm())
		test_assert_equal(99, hoge.getmm2())
		hoge.mm = 88
		test_assert_equal(88, hoge.m)
		test_assert_equal(88, hoge.mm)
		test_assert_equal(88, hoge.getmm())
		test_assert_equal(88, hoge.getmm2())
		hoge.m = 77
		test_assert_equal(77, hoge.m)
		test_assert_equal(77, hoge.mm)
		test_assert_equal(77, hoge.getmm())
		test_assert_equal(77, hoge.getmm2())
	}


	@@ha = [1]
	@@h1 = new Hoge(1)
	@@h2 = new Hoge(2)
	def gha() { @@ha }
	def gh1(x) { @@h1 }
	def gh2(x,y) { @@h2 }
	def hytest_objMember ()
	{
		hoge = new Hoge()
		fa = lambda() { @@ha }
		f1 = lambda(x){ @@h1 }
		f2 = lambda(x,y){ @@h2 }
		
		@@ha = [1]
		test_assert_equal(1, @@ha[0])
		@@ha[0] += 1
		test_assert_equal(2, @@ha[0])
		test_assert_equal(2, gha()[0])
		gha()[0] += 1
		test_assert_equal(3, gha()[0])
		test_assert_equal(3, fa()[0])
		fa()[0] += 1
		test_assert_equal(4, fa()[0])
		test_assert_equal(4, @@ha[0])

		a = [hoge]
		test_assert_equal(0, a[0].m)
		test_assert_equal(0, a[0].mm)
		a[0].m += 3
		test_assert_equal(3, a[0].m)
		test_assert_equal(3, a[0].mm)
		a[0].mm += 4
		test_assert_equal(7, a[0].m)
		test_assert_equal(7, a[0].mm)

		test_assert_equal(1, @@h1.m)
		gh1(99).m += 2
		test_assert_equal(3, @@h1.m)
		gh1(99).mm += 2
		test_assert_equal(5, @@h1.m)
		f1(99).m *= 2
		test_assert_equal(10, @@h1.m)
		f1(99).mm *= 3
		test_assert_equal(30, @@h1.m)

		test_assert_equal(2, @@h2.mm)
		gh2(99,98).m += 3
		test_assert_equal(5, @@h2.mm)
		gh2(99,98).mm += 3
		test_assert_equal(8, @@h2.mm)
		f2(99,98).m *= 10
		test_assert_equal(80, @@h2.mm)
		f2(99,98).mm /= 8
		test_assert_equal(10, @@h2.mm)
		
		a[0] = f1
		a[0](97).m += 4
		test_assert_equal(34, @@h1.m)
		a[0](97).mm -= 22
		test_assert_equal(12, @@h1.m)

		b = new Hoge(new Hoge(new Hoge(-9)))
		test_assert_equal(-9, b.m.mm.m)
		b.m.mm.m -= 5
		test_assert_equal(-14, b.m.mm.m)
		a[0] = b
		a[0].mm.m.mm -= 8
		test_assert_equal(-22, b.m.mm.m)
	}


	class Fuga
		def initialize() { @m,@get,@set = 0,0,0 }
		def m () { @get += 1; @m }
		def m=(x) { @set += 1; @m = x }
	end

	def hytest_priority ()
	{
		// メンバ変数と同名のsetter,getterメソッドがある場合はメソッドを優先 
		fuga = new Fuga()
		test_assert_equal(0, fuga.get)
		test_assert_equal(0, fuga.set)
		test_assert_equal(0, fuga.m)
		test_assert_equal(1, fuga.get)
		fuga.m = 10
		test_assert_equal(1, fuga.get)
		test_assert_equal(1, fuga.set)
		test_assert_equal(10, fuga.m)
		test_assert_equal(2, fuga.get)
		fuga.m += 100
		test_assert_equal(3, fuga.get)
		test_assert_equal(2, fuga.set)
		test_assert_equal(110, fuga.m)
		test_assert_equal(4, fuga.get)
		fuga.m, x = 40,50
		test_assert_equal(4, fuga.get)
		test_assert_equal(3, fuga.set)
		test_assert_equal(40, fuga.m)
		test_assert_equal(5, fuga.get)
		x, fuga.m = *[40,50]
		test_assert_equal(5, fuga.get)
		test_assert_equal(4, fuga.set)
		test_assert_equal(50, fuga.m)
		test_assert_equal(6, fuga.get)
		test_assert_equal(4, fuga.set)
	}


	class Foo
		@@classVar = nil
		def cv { @@classVar }
		def cv= (x) { @@classVar = x }
	end

	def hytest_classVarAccess ()
	{
		Foo.classVar = 2
		test_assert_equal(2, Foo.classVar)
		test_assert_equal(2, Foo.cv)
		Foo.cv = 3
		test_assert_equal(3, Foo.classVar)
		test_assert_equal(3, Foo.cv)
		Foo.classVar += 5
		test_assert_equal(8, Foo.classVar)
		test_assert_equal(8, Foo.cv)
		Foo.cv += 7
		test_assert_equal(15, Foo.classVar)
		test_assert_equal(15, Foo.cv)
	}

end
