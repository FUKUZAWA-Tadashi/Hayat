

$hyUnit_total_assertions = 0
$hyUnit_testCount = 0
$hyUnit_okCount = 0


def _hy_test (klass,func)
{
	$hyUnit_testMethod = <<{klass.classSymbol()}.{func}>>
	try {
		$hyUnit_testCount += 1
		f = new Method(klass, :setUp)
		f.call()
		f = new Method(klass, func)
		f.call()
		// OK
		$hyUnit_okCount += 1
	} catch (e) {
		// NG
		warning($hyUnit_testMethod, " NG")
		if (e.type() == :HyUnit_assert_fail) {
			warning(e.backTraceStr(1))
		} else {
			warning("exception thrown: ", e.val())
			e.backTraceSize().times()->{|i|
				warning(e.backTraceStr(i))
			}
		}
	}
	f = new Method(klass, :tearDown)
	f.call()
	GC.full()
	GC.coalesce()
}


def _hy_result ()
{
	warning($hyUnit_testCount, " tests, ", $hyUnit_total_assertions, " assertions")
	if ($hyUnit_testCount == $hyUnit_okCount)
		warning("all OK")
	else
		warning($hyUnit_testCount - $hyUnit_okCount, " tests failed")
}
