// -*- coding: utf-8 -*-
require "hyUnit"


CONST_OUT = 10
def func(x) { x + 1 }
aaaaa = 100
class FooBar
end

class MethodName
	def f(x) { x }
	def f(x,y) {x+y}			// OK, オーバーロード
	//def f(y) { y+1 }			// エラー、同シグネチャ

	CONST = 1
	//def CONST(x) { x }			// エラー
	def CONST_OUT(x){x}			// OK

	g = 10
	//def g(x){x}					// エラー
	def aaaaa(){nil}			// OK

	class inner
	end
	//def inner(x){x}				// エラー
	def FooBar(){}				// OK
end


class ClassName
	class FooBar				// OK, 別レベル
	end

	class FooBar				// OK、オープンクラス
	end

	aa = 1
	//class aa					// エラー
	//end

	const ConstVar = 1
	//class ConstVar				// エラー
	//end
	class CONST_OUT				// OK
	end

	def m(x) {x}
	//class m						// エラー
	//end
	class func					// OK
	end

end


class ConstName
	const CONST_OUT = 10		// オプションにより許可/警告/禁止
	//const CONST_OUT = 20		// エラー

	const FooBar = 10			// OK
	class Asd
	end
	//const Asd = 10				// エラー

	const func = 20				// OK
	def fff(){}
	//const fff = 10				// エラー

	const aaaaa = 101			// OK
	a = 9
	//const a = 99				// エラー
end


class LocalVarName
	CONSTVAR = 1
	//local CONSTVAR = 2			// エラー
	CONST_OUT = 3			// オプションにより許可/警告

	def f(){nil}
	//f = 12						// エラー
	func = 23					// オプションにより許可/警告

	class Klass
	end
	//Klass = 34					// エラー
	FooBar = 45					// オプションにより許可/警告
end


class クラス1
	def initialize (x)
	{
		@メンバ1 = x
	}

	def メソッド1 (引数)
	{
		変数 = 10
		変数 += 引数 * 2
		変数 + @メンバ1
	}

	def メソッド2 (引数)
	{
		引数 + @@クラス変数1
	}

	const 定数1 = 100
	@@クラス変数1 = 20
end


class HyTest_ident < HyUnit
	def hytest_ident ()
	{
		test_assert_equal(100, クラス1::定数1)
		オブジェクト1 = new クラス1(99)
		test_assert_equal(309, オブジェクト1.メソッド1(クラス1::定数1))
		オブジェクト1.メンバ1 = 33
		test_assert_equal(33, オブジェクト1.メンバ1)
		test_assert_equal(57, オブジェクト1.メソッド1(7))
		test_assert_equal(30, クラス1.メソッド2(10))
	}	
end
