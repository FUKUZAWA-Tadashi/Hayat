require "hyUnit"


class HyTest_random < HyUnit

	def hytest_random ()
	{
		// シードは必ず変化する
		x = 0
		flag = false
		100.times{
			y = Random.rand()
			flag ||= (x == y)
			x = y
		}
		test_assert(! flag)

		// rand(n)のテスト
		100.times{
			x = Random.rand(17)
			flag ||= (x < 0 || x > 17)
		}
		test_assert(! flag)

		// seedを保存
		seed = Random.seed()
		// その次の乱数を保存
		r1 = Random.rand()

		// rand(n,m)のテスト
		a = [0,0,0,0,0,0,0]
		1000.times{
			x = Random.rand(-3,3)
			test_assert(x >= -3 && x <= 3)
			a[x+3] += 1
		}
		7.times{|i|
			// Debug.print(a[i]) 期待値142.857 本当はカイ2乗検定すべきだが適当
			test_assert(a[i] > 110 && a[i] < 175)
		}

		// seedの設定
		test_assert_equal(seed, Random.srand(seed))
		// その次の乱数が前と同じはず
		test_assert_equal(r1, Random.rand())
		test_assert(r1 != Random.rand()) // その次は違う
	}

end
