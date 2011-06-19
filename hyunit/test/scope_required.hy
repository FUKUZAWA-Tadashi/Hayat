
// hyTest_scope.hy から require される


D = 99
E = ["E!"]
enum {
	E_00,
	E_01,
	E_02,
}

def g () {
	@@x ||= 100
	@@x += 1
	//return @@x
}

class ExtClass
	enum {
		EC_00,
		EC_01,
		EC_02,
	}
	INTCONST = 90
	FLOATCONST = 1.732
	ARR = [91,92,93]
	def xxx ()
	{
		ARR
	}
end
