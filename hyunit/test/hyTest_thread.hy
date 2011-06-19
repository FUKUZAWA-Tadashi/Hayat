require "hyUnit"


class TestCounter
	def initialize (first, last, diff) {
		@count = first
		@last = last
		@diff = diff
		@sum = first
	}

	def run () {
		loop {
			// warning(@count, ":", @sum)
			wait(@count)
			if (@diff > 0 && @count >= @last)
				break
			else if (@diff < 0 && @count <= @last)
				break
			@count += @diff
			@sum += @count
		}
	}

	def getCount () { return @count }
	def getSum () { return @sum }
end


class HyTest_thread < HyUnit

	def hytest_thread ()
	{
		r1 = new TestCounter(1, 20, 1)
		r2 = new TestCounter(20, 1, -1)
		t1 = new Thread(r1)
		t2 = new Thread(r2)

		while (t1.running?() || t2.running?())
		{
			wait
			d = r1.getSum() - r2.getSum()
			if (d < 0) {
				d = -d
				a = r2.getCount()
			} else {
				a = r1.getCount()
			}
			test_assert(d < a)
		}
		test_assert(r1.getSum() == r2.getSum())
	}

	def hytest_thread_kill ()
	{
		r1 = new TestCounter(1, 20, 1)
		r2 = new TestCounter(1, 20, 1)
		t1 = new Thread(r1)
		t2 = new Thread(r2)
		wait(10)
		c1 = r1.getCount()
		c2 = r2.getCount()
		t1.kill()
		wait(100)
		test_assert(c1 == r1.getCount())
		test_assert(c2 <> r2.getCount())
		t2.join()
	}

	def jugtest_thread_closure ()
	{
		x = 0
		c = lambda() {
			for (a = 0; a < 10; a += 1) {
				x += 1
				wait
			}
		}

		t1 = new Thread(c)

		wait
		y = x
		wait
		test_assert_equal(y+1, x)
		wait
		test_assert_equal(y+2, x)
		wait
		test_assert_equal(y+3, x)
		wait(10)
		test_assert_equal(10, x)
		t1.join()
	}
	
end
