// -*- coding: utf-8 -*-

require "hyUnit"
require "reload"



class Sup < ReloadTest
end


class HyTest_reload < HyUnit

	def setUp ()
	{
		// "reload.hy" の方をロードしておく
		Bytecode.setClassVarCopiedOnReload(false)
		Bytecode.setConstVarCopiedOnReload(false)
		Bytecode.setObsoleteVarKeptOnReload(false)
		Bytecode.reload("reload")
	}


	def hytest_reload ()
	{
		rt1 = new ReloadTest()
		test_assert_equal(["const1","glob1","const_c1","cv1","ins1"], rt1.values())

		Bytecode.setClassVarCopiedOnReload(false)
		Bytecode.setConstVarCopiedOnReload(false)
		Bytecode.reloadAs("reload2", :"reload.hyb")

		rt2 = new ReloadTest()
		test_assert_equal(["const2","glob2","const_c2","cv2","ins2"], rt2.values())
		test_assert_equal(["const2","glob2","const_c2","cv2","ins1"], rt1.values())

		Bytecode.reload("reload")

		rt3 = new ReloadTest()
		test_assert_equal(["const1","glob1","const_c1","cv1","ins1"], rt3.values())
		test_assert_equal(["const1","glob1","const_c1","cv1","ins2"], rt2.values())
		test_assert_equal(["const1","glob1","const_c1","cv1","ins1"], rt1.values())

		Bytecode.setClassVarCopiedOnReload(true)
		Bytecode.setConstVarCopiedOnReload(true)
		Bytecode.reloadAs("reload2", :"reload.hyb")

		rt4 = new ReloadTest()
		test_assert_equal(["const1","glob2","const_c1","cv1","ins2"], rt4.values())
		test_assert_equal(["const1","glob2","const_c1","cv1","ins1"], rt3.values())
		test_assert_equal(["const1","glob2","const_c1","cv1","ins2"], rt2.values())
		test_assert_equal(["const1","glob2","const_c1","cv1","ins1"], rt1.values())
	}

	def hytest_reload_fn ()
	{
		test_assert_equal([], Bytecode.remains())
		f1 = ReloadTest.fn()
		test_assert_equal(1, f1.generate())
		Bytecode.reloadAs("reload2", :"reload.hyb")
		f2 = ReloadTest.fn()
		test_assert_equal(1, f1.generate())
		test_assert_equal(1, f2.generate())
		test_assert_equal([2,3,5,8,13], f1.take(5).toArray())
		test_assert_equal([4,9,16,25,36], f2.take(5).toArray())
		GC.full();
		test_assert_equal([:"reload.hyb"], Bytecode.remains())
		test_assert_equal(21, f1.generate())
		test_assert_equal(49, f2.generate())
		f1 = nil
		GC.full();
		test_assert_equal([], Bytecode.remains())
		test_assert_equal(64, f2.generate())
	}
	
	def hytest_reload_sub ()
	{
		store = []
		th = new Thread{ ReloadTest.test(store) }
		wait(2)
		wait(2)
		test_assert_equal("testReload1", store[0])

		th = new Thread{ ReloadTest.test(store) }
		wait(2)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		wait(2)
		test_assert_equal("test reload 2", store[1])
		
		Bytecode.reload("reload") // 元に戻す
	}

	def hytest_reload_g ()
	{
		g = ReloadTest.g1()
		test_assert_equal(1, g.generate())
		Bytecode.reloadAs("reload2", :"reload.hyb")
		test_assert_equal(2, g.generate())
		Bytecode.reload("reload")
		test_assert_equal(1, g.generate())
	}

	def hytest_reload_obs ()
	{
		Bytecode.setObsoleteVarKeptOnReload(true)
		Bytecode.setClassVarCopiedOnReload(false)
		Bytecode.setConstVarCopiedOnReload(false)
		test_assert_equal(["C1","C3","cv1","cv3"], Reload_obs.g())

		Bytecode.reloadAs("reload2", :"reload.hyb")
		test_assert_equal(["2_C1","2_C2","2_C4","2_cv1","2_cv2","2_cv4"], Reload_obs.g())
		test_assert_equal(nil,Reload_obs::C3)
		Bytecode.reload("reload")
		test_assert_equal(["C1","C3","cv1","cv3"], Reload_obs.g())
		test_assert_equal(nil,Reload_obs::C2)

		Bytecode.setObsoleteVarKeptOnReload(false)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		test_assert_equal(["2_C1","2_C2","2_C4","2_cv1","2_cv2","2_cv4"], Reload_obs.g())
		test_assert_throw{Reload_obs::C3}
		Bytecode.reload("reload")
		test_assert_equal(["C1","C3","cv1","cv3"], Reload_obs.g())
		test_assert_throw{Reload_obs::C2}


		Bytecode.setObsoleteVarKeptOnReload(true)
		Bytecode.setClassVarCopiedOnReload(true)
		Bytecode.setConstVarCopiedOnReload(true)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		test_assert_equal(["C1","2_C2","2_C4","cv1","2_cv2","2_cv4"], Reload_obs.g())
		test_assert_equal("C3",Reload_obs::C3)
		Bytecode.reload("reload")
		test_assert_equal(["C1","C3","cv1","cv3"], Reload_obs.g())
		test_assert_equal("2_C2",Reload_obs::C2)

		Bytecode.setObsoleteVarKeptOnReload(false)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		test_assert_equal(["C1","2_C2","2_C4","cv1","2_cv2","2_cv4"], Reload_obs.g())
		test_assert_throw{Reload_obs::C3}
		Bytecode.reload("reload")
		test_assert_equal(["C1","C3","cv1","cv3"], Reload_obs.g())
		test_assert_throw{Reload_obs::C2}
	}


	def hytest_reload_memb ()
	{
		o = new Reload_memb()
		test_assert_equal("@1", o.memb1)
		test_assert_equal("@2", o.memb2)
		test_assert_throw{o.memb3}

		Bytecode.setObsoleteVarKeptOnReload(true)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		
		o2 = new Reload_memb()
		test_assert_equal("@1", o.memb1)
		test_assert_equal("@2", o.memb2)
		test_assert_equal(nil, o.memb3)
		test_assert_equal(nil, o.memb4)
		test_assert_equal("#1", o2.memb1)
		test_assert_equal(nil, o2.memb2)
		test_assert_equal("#3", o2.memb3)
		test_assert_equal("#4", o2.memb4)

		Bytecode.reload("reload")

		o3 = new Reload_memb()
		test_assert_equal("@1", o.memb1)
		test_assert_equal("@2", o.memb2)
		test_assert_equal(nil, o.memb3)
		test_assert_equal(nil, o.memb4)
		test_assert_equal("#1", o2.memb1)
		test_assert_equal(nil, o2.memb2)
		test_assert_equal("#3", o2.memb3)
		test_assert_equal("#4", o2.memb4)
		test_assert_equal("@1", o3.memb1)
		test_assert_equal("@2", o3.memb2)
		test_assert_equal(nil, o3.memb3)
		test_assert_equal(nil, o3.memb4)

		test_assert_throw{o.memb9}
		test_assert_throw{o2.memb9}
		test_assert_throw{o3.memb9}
	}

	def hytest_sup ()
	{
		s1 = new Sup()
		test_assert_equal(["const1","glob1","const_c1","cv1","ins1"], s1.values())
		f1 = Sup.fn()
		test_assert_equal(1, f1.generate())


		Bytecode.reloadAs("reload2", :"reload.hyb")
		s2 = new Sup()
		test_assert_equal(["const2","glob2","const_c2","cv2","ins2"], s2.values())
		test_assert_equal(["const2","glob2","const_c2","cv2","ins1"], s1.values())
		f2 = Sup.fn()
		test_assert_equal(1, f1.generate())
		test_assert_equal(1, f2.generate())
		
		Bytecode.setClassVarCopiedOnReload(true)
		Bytecode.setConstVarCopiedOnReload(true)
		Bytecode.reload("reload")
		s3 = new Sup()
		test_assert_equal(["const2","glob1","const_c2","cv2","ins1"], s3.values())
		test_assert_equal(["const2","glob1","const_c2","cv2","ins2"], s2.values())
		test_assert_equal(["const2","glob1","const_c2","cv2","ins1"], s1.values())
		f3 = Sup.fn()
		test_assert_equal([2,3,5,8,13], f1.take(5).toArray())
		test_assert_equal([4,9,16,25,36], f2.take(5).toArray())
		test_assert_equal([1,1,2,3,5], f3.take(5).toArray())
	}


	def hytest_reload_closure ()
	{
		x = 0
		th = new Thread{ x = Reload_closure.f() }
		wait(4)
		Bytecode.reload("reload")
		wait(2)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		wait(2)
		Bytecode.reload("reload")
		wait(2)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		wait(6)
		test_assert_equal(5, x)

		Bytecode.reload("reload") // 元に戻す
		wait

		x = 0
		th = new Thread{ x = Reload_closure.f() }
		wait(2)
		Bytecode.reload("reload")
		wait(2)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		wait(2)
		Bytecode.reload("reload")
		wait(2)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		wait(6)
		test_assert_equal(5, x)

		Bytecode.reload("reload") // 元に戻す
		wait

		x = 0
		th = new Thread{ x = Reload_closure.f() }
		wait(4)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		wait(2)
		Bytecode.reload("reload")
		wait(2)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		wait(2)
		Bytecode.reload("reload")
		wait(6)
		test_assert_equal(5, x)

		x = 0
		th = new Thread{ x = Reload_closure.f() }
		wait(2)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		wait(2)
		Bytecode.reload("reload")
		wait(2)
		Bytecode.reloadAs("reload2", :"reload.hyb")
		wait(2)
		Bytecode.reload("reload")
		wait(6)
		test_assert_equal(50, x)
	}

end
