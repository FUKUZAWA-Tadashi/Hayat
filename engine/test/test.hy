// -*- coding: utf-8-unix -*-
// ユニットテスト用スクリプト
//


class AClass
    def setcv(x) {
        @@cv = x
    }

    def setx(x) {
        @x = x
    }
end

class BClass
    def initialize() {
        @a = new Array(10)
    }

    def setAt(i, x) {
        @a[i] = x
    }
    def getAt(i) {
        return @a[i]
    }
end

class CClass
	def initialize() { @a = 0 }
end


// waitの時にGCしてみるテスト 
def gc_test_instance()
{

    $aobj = new AClass()
    b = new BClass()
    b.setAt(1, new CClass())
    b.setAt(3, new CClass())
    $aobj.setcv(b)

    wait        // start時

    b.setAt(1, nil)

    wait        // -1: CClassのオブジェクトが1つ消える

    $aobj.setcv(nil)
    $aobj.setx(b)

    wait        // -0: 変化なし

    b = nil

    wait        // -0: 変化なし

    $aobj.setx(nil)

    wait        // -3: BClassのオブジェクトから、Array,CClassも消える

    $aobj = nil

    wait        // -1: AClassのオブジェクトが消える

    // 終了: -2: Thread と Context が不要になる
}


// GCに終了スレッドを回収させるテスト
def gc_test_thread_sweep()
{
    a = new AClass()
    $b = new BClass()
    @@c = new CClass()
}


def gc_test_multiThread(n)
{
	a = [100]
	$b = 0
	wait(n)
	th = new Thread{
		for (k = 0; k < 5; k += 1) {
			a[0] -= 1
			$b = a[0]
			wait
		}
	}
	wait(2)
}

def getGlobalB ()
{
	$b
}




// どんどんインスタンスを生成してメモリを溢れさせて、
// 強制GCを起こすテスト
class XClass
	def initialize (x) {
		@a = new Array(x)
	}
end
class YClass
	def memb_def () {
		@m00 = 0; @m01 = 0; @m02 = 0; @m03 = 0; @m04 = 0
		@m05 = 0; @m06 = 0; @m07 = 0; @m08 = 0; @m09 = 0
		@m10 = 0; @m11 = 0; @m12 = 0; @m13 = 0; @m14 = 0
		@m15 = 0; @m16 = 0; @m17 = 0; @m18 = 0; @m19 = 0
		@m20 = 0; @m21 = 0; @m22 = 0; @m23 = 0; @m24 = 0
		@m25 = 0; @m26 = 0; @m27 = 0; @m28 = 0; @m29 = 0
		@m30 = 0; @m31 = 0; @m32 = 0; @m33 = 0; @m34 = 0
		@m35 = 0; @m36 = 0; @m37 = 0; @m38 = 0; @m39 = 0
		@m40 = 0; @m41 = 0; @m42 = 0; @m43 = 0; @m44 = 0
		@m45 = 0; @m46 = 0; @m47 = 0; @m48 = 0; @m49 = 0
		@m50 = 0; @m51 = 0; @m52 = 0; @m53 = 0; @m54 = 0
		@m55 = 0; @m56 = 0; @m57 = 0; @m58 = 0; @m59 = 0
		@m60 = 0; @m61 = 0; @m62 = 0; @m63 = 0; @m64 = 0
		@m65 = 0; @m66 = 0; @m67 = 0; @m68 = 0; @m69 = 0
		@m70 = 0; @m71 = 0; @m72 = 0; @m73 = 0; @m74 = 0
		@m75 = 0; @m76 = 0; @m77 = 0; @m78 = 0; @m79 = 0
		@m80 = 0; @m81 = 0; @m82 = 0; @m83 = 0; @m84 = 0
		@m85 = 0; @m86 = 0; @m87 = 0; @m88 = 0; @m89 = 0
		@m90 = 0; @m91 = 0; @m92 = 0; @m93 = 0; @m94 = 0
		@m95 = 0; @m96 = 0; @m97 = 0; @m98 = 0; @m99 = 0
	}
end

def create_XClass (n) {
	x = new XClass(n)
	n.times{|i| x.a[i] = n}
}
def create_YClass () {
	y = new YClass()
	y.m00 = 10
	y.m99 = 100
}



class Base_CPP
	c++ Base {
		something():NilClass
	}
end

class Derived_A_CPP < Base_CPP
end

class Derived_B_CPP < Base_CPP
	c++ Derived_B {
		something():NilClass
	}
end

class Derived_C_CPP < Base_CPP
	c++ Derived_C {
		something():NilClass
	}
end

// Derived_B と Derived_C の2つのC++クラスがあるのでエラー
// エラーを回避するには、両者を継承するC++クラスを明示的に指定すれば良い
//class Derived_BC_CPP < Derived_B_CPP, Derived_C_CPP
//end




def gc_test_hash ()
{
	a = {}
	wait
	b = new Hash(4, 0.75)
	wait
	a[1] = "1"
	wait
	a["@"] = "'@'"
	wait
	b[100] = "100"
	wait
	a = nil
	wait
	b[10] = "10"
	b[11] = "11"
	b[12] = "12"
	b[13] = "13"
	b[14] = "14"
	b[15] = "15"
	wait
	b = nil
	wait
	
}




// write barrier のテスト
// 別スレッド、グローバル変数
def gc_test_writeBarrier ()
{
	$a = 100.times.map{|i| 「{i * 100}」}.toArray()
	b =  100.times.map{|i| 「{i * -10}」}.toArray()
	wait

	th = new Thread{
		3.times {
			100.times{|i|
				$a[i],b[i] = b[i],$a[i]
				wait
			}
		}
	}
	th.join()
}


// write barrier のテスト
// 配列リテラル
def gc_test_writeBarrier_2 ()
{
	gen = lambda(i) {
		wait
		「{i}」
	}

	$a=[[gen(0),gen(1)],
		[gen(2),gen(3)],
		[gen(4),gen(5)],
		[gen(6),gen(7)],
		[gen(8),gen(9)],
		[gen(10),gen(11)],
		[gen(12),gen(13)],
		[gen(14),gen(15)],
		[gen(16),gen(17)],
		[gen(18),gen(19)],
		[gen(20),gen(21)],
		[gen(22),gen(23)],
		[gen(24),gen(25)],
		[gen(26),gen(27)],
		[gen(28),gen(29)],
		[gen(30),gen(31)],
		[gen(32),gen(33)],
		[gen(34),gen(35)],
		[gen(36),gen(37)],
		[gen(38),gen(39)],
		[gen(40),gen(41)],
		[gen(42),gen(43)],
		[gen(44),gen(45)],
		[gen(46),gen(47)],
		[gen(48),gen(49)],
		[gen(50),gen(51)],
		[gen(52),gen(53)],
		[gen(54),gen(55)],
		[gen(56),gen(57)],
		[gen(58),gen(59)],
		[gen(60),gen(61)],
		[gen(62),gen(63)],
		[gen(64),gen(65)],
		[gen(66),gen(67)],
		[gen(68),gen(69)],
		[gen(70),gen(71)],
		[gen(72),gen(73)],
		[gen(74),gen(75)],
		[gen(76),gen(77)],
		[gen(78),gen(79)],
		[gen(80),gen(81)],
		[gen(82),gen(83)],
		[gen(84),gen(85)],
		[gen(86),gen(87)],
		[gen(88),gen(89)],
		[gen(90),gen(91)],
		[gen(92),gen(93)],
		[gen(94),gen(95)],
		[gen(96),gen(97)],
		[gen(98),gen(99)],
	]
}
