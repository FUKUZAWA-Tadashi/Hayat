require "hyUnit"

class Xobj
	def initialize(x,v) {
		@x = x
		@v = v
	}
	def adv() {
		@x += @v
		@x > 1000
	}
	//def finalize() {
	//	Debug.print("Xobj::finalize()")
	//}
end


class HyTest_GC < HyUnit

	def hytest_1 ()
	{
		GC.full()
		arr = new Array(100)
		1000.times{
			100.times{|i|
				if (arr[i]) {
					if (arr[i].adv())
					arr[i] = nil
				} else {
					arr[i] = new Xobj(Random.rand(0,500), Random.rand(10,100))
				}
			}
			GC.incremental()
			// Debug.printGCPhase()
		}
		test_ok()
	}
	
end
