require "hyUnit"

class SayTest
	def initialize () {
		@mes = new StringBuffer()
		@i = 0
		@n = -1
		@z = 0
		@arr = []
	}
	def sayCommandStart(i, n) { @i = i; @n = n }
	def sayCommand(x) { @mes.concat(x); @arr.push(x) }
	def sayCommandEnd() { @z += 1 }
	def clear_mes() { @mes.clear(); @arr = [] }
	def get_i () { @i }
	def get_n () { @n }
	def get_z () { @z }
	def get_mes () { @mes }
	def get_arr () { @arr }
end


class HyTest_sayCommand < HyUnit

	def hytest_sayCommand ()
	{
		sayTest = new SayTest()
		sayTest「おはよう」
		test_assert_equal(0, sayTest.get_i())
		test_assert_equal(1, sayTest.get_n())
		test_assert_equal(1, sayTest.get_z())
		test_assert_equal("おはよう", sayTest.get_mes())
		sayTest.clear_mes()
		test_assert_equal("", sayTest.get_mes())
		sayTest「
			こんにちは
		」
		test_assert_equal(1, sayTest.get_i())
		test_assert_equal(3, sayTest.get_n())
		test_assert_equal(2, sayTest.get_z())
		test_assert_equal("\nこんにちは\n", sayTest.get_mes())

		sayTest「」
		test_assert_equal(1, sayTest.get_n())
		sayTest「
		」
		test_assert_equal(2, sayTest.get_n())
		sayTest「

		」
		test_assert_equal(3, sayTest.get_n())
		sayTest「あああ
		」
		test_assert_equal(2, sayTest.get_n())
		sayTest「
		あああ」
		test_assert_equal(2, sayTest.get_n())
		sayTest.clear_mes()
		test_assert_equal("", sayTest.get_mes())
		test_assert(! ("" != sayTest.get_mes()) )
		test_assert(! ("" <> sayTest.get_mes()) )
		test_assert_equal([], sayTest.get_arr())
		sayTest「あ{1+2}い{4.0+10.5}う{"a"}え{30<>30}お」
		test_assert_equal("あ3い14.5うaえfalseお", sayTest.get_mes())
		test_assert(! (sayTest.get_mes() != "あ3い14.5うaえfalseお") )
		test_assert(! (sayTest.get_mes() <> "あ3い14.5うaえfalseお") )
		test_assert_equal(["あ",3,"い",14.5,"う","a","え",false,"お"], sayTest.get_arr())

		sayTest.clear_mes()
		sayTest「あ{#1+2}い{#4.0+10.5}う{#"a"}え{#30<>30}お」
		test_assert_equal("あいうえお", sayTest.get_mes())

		sayTest.clear_mes()
		a = 0 ; b = 0 ; c = 0 ; d = 0
		sayTest「あ{#a=1+2}い{b=7.0+6.25}う{#c="a";d="b"}え」
		test_assert_equal("あい13.25うえ", sayTest.get_mes())
		test_assert_equal(3,a)
		test_assert_equal(13.25,b)
		test_assert_equal("a",c)
		test_assert_equal("b",d)

		sayTest.clear_mes()
		sayTest「{1}{1}ああ{2}
				 {2}いい{3}{3}」
		test_assert_equal([1,1, "ああ", 2,"\n",2, "いい", 3,3], sayTest.get_arr())

		sayTest.clear_mes()
		sayTest「{nil},{:sym},{1},{2.5},{true},{"str"},{Hash},{[1, 2]},{{:a => false}},{'( 9, '( 8 ))}」
		test_assert_equal("nil,:sym,1,2.5,true,str,Hash,[1,2],{:a=>false},'(9,'(8))",sayTest.get_mes())
		sayTest.clear_mes()
		sayTest「{new Exception()},{new Exception("foo")},{new Exception("bar", :test)}」
		test_assert_equal("((exception)nil:<NilClass>),((exception)\"foo\":<String>),((exception)\"bar\":<String>,test)", sayTest.get_mes())

		sb = 「さようなら」
		test_assert_equal("さようなら", sb)

		sb.clear()
		sb「a {"b"} c」
		test_assert_equal("a b c", sb)

		sb.clear()
		sb「d\ {"e"}\ f」
		test_assert_equal("d e f", sb)

		// 基本的に、行頭のスペースは無視。行末のスペースは、テキストファイル
		// として見た時に存在がわからない状態なら無視。
		// h,i,j の前のスペースは無視
		// hの後のスペースは無視
		// iの後のエスケープしてあるスペースまでは残るがその後は無視
		// jの後のスペースは、そこに存在する事が見てわかる状態なので残る
		sb.clear()
		sb「 g
	   h 	
		 i \    
		j  」
		test_assert_equal(" g\nh\ni  \nj  ", sb)
	}

	def hytest_stringBuffer ()
	{
		// インスタンスを指定しないsayCommandは、new StringBuffer() がインスタンスとなる。
		a = 「あ」
		test_assert_equal(StringBuffer, a.getClass())
		test_assert_equal("あ", a)

		a = 「a{1+2}b」
		test_assert_equal("a3b", a)

		a「xyz」
		test_assert_equal("a3bxyz", a)
	}

end
