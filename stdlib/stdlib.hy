//
// 標準ライブラリ
//


// ============================================================
// 基本クラス
// Engine内部では、ObjectにはならずValueだけで表現する
// ============================================================


// concatToStringBuffer の mode は以下の意味を持つ。
// mode < 0  なら、型を追加
// mode == 0 なら、通常
// mode > 0  なら、nilの場合何も追加しない。Symbolなら、先頭の : を追加しない


// 組み込み関数
c++ {
	// Debugクラスとは違い、デバッグビルドでなくても実行する。
	// 表示系は、C++レベルでHMD_PRINTFマクロを呼び出すので、
	// 機種依存部分(machdep.h)にてHMD_PRINTFが動作するように
	// なっているのであれば表示する。
	// {
	// 引数がfalseだとエラー停止
	assert(x:Bool):NilClass
	// 引数がfalseだとエラー停止、メッセージ付き
	assertMsg(...):NilClass
	// デバッグメッセージ出力
	warning(...):NilClass
	// }

	// 現在実行中のスレッドを返す  
	getRunningThread()  
	// 現在実行中のスレッドをスリープさせる  
	sleep():NilClass  
}


// ------------------------------------------------------------
// Objectクラス ： 全てのクラスの暗黙のスーパークラス
// メソッドは全インスタンスに共通なメソッドとして使用される。
// ------------------------------------------------------------
class Object
	c++ {
		// クラスを返す
		getClass():Class
		// クラスのシンボルを返す
		classSymbol():Symbol
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		// ハッシュコードを計算
		hashCode():Int
		sameObject?(x):Bool	// 比較： 参照レベルで同じオブジェクトか
		"=="(x):Bool	        // 比較
		"<>"(x):Bool			// 比較
		"!="(x):Bool			// 比較


		// selfがメソッドを持っているかどうか
		haveMethod?(methodSymbol:Symbol):Bool
	}

	def "!" () { false }

	// メソッドシンボルから、Methodオブジェクトを作成して返す
	def getMethod (methodSymbol)
	{
		new Method(self, methodSymbol)
	}

	def callIfMethodExist (methodSymbol, *args)
	{
		if (haveMethod?(methodSymbol))
			getMethod(methodSymbol).call(*args)
		else
			nil
	}

end


// ------------------------------------------------------------
// Debugクラス
// デバッグ用メソッド群。
// ------------------------------------------------------------
class Debug
	c++ {
		// 何もしない。
		// C++のデバッガで、HSca_Debug.cppのこのメソッド定義部分に
		// ブレークポイントを仕掛けるという用途に使える。
		nop():NilClass

		// デバッグバージョンかどうか
		// C++レベルで HMD_DEBUG がdefineされていれば true
		debug?():Bool

		// 引数オブジェクトを並べて型情報付きデバッグ表示
		// デバッグバージョンでなければ何もしない
		p(...):NilClass // 可変長引数

		// 引数がfalseだとエラー停止
		// デバッグバージョンでなければ何もしない
		assert(x:Bool):NilClass

		// 引数がfalseだとエラー停止、メッセージ付き
		// デバッグバージョンでなければ何もしない
		assertMsg(...):NilClass

		// デバッグメッセージ出力
		// デバッグバージョンでなければ何もしない
		print(...):NilClass
		// sayCommand開始 ; デバッグバージョンでなければ何もしない
		sayCommandStart(x:Int, n:Int):NilClass
		// 文字列化してデバッグ出力 ; デバッグバージョンでなければ何もしない
		sayCommand(x):NilClass
		// sayCommand終了 ; デバッグバージョンでなければ何もしない
		sayCommandEnd():NilClass


		// VM命令1ステップ毎にオペコードをデバッグ表示するかどうかを設定
		// デバッグバージョンでなければ設定しても何も表示しない
		setPrintOpcode(b:Bool):NilClass

		// VM命令1ステップ毎にスタック状態をデバッグ表示するかどうかを設定
		// デバッグバージョンでなければ設定しても何も表示しない
		setPrintStack(b:Bool):NilClass

		// 全スレッドのスタックトレースを表示
		// デバッグバージョンでなければ何もしない
		printAllStackTrace():NilClass

		// GCフェーズの表示
		// デバッグバージョンでなければ何もしない
		printGCPhase():NilClass
		// GC用markフラグの状況を表示
		// デバッグバージョンでなければ何もしない
		printGCMark(val):NilClass

		// メモリ使用状況をデバッグ表示
		// デバッグバージョンでなければ何もしない
		dumpMemory():NilClass
	}
end



// ------------------------------------------------------------
// NilClassクラス
// ------------------------------------------------------------
class NilClass
	c++ {
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		// mode<0: "nil:<NilClass>"   mode==0: "nil"   mode>0: ""
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		// ハッシュコードを計算
		hashCode():Int
		"=="(x):Bool	        // 比較
	}
	def "!" () { true }
end


// ------------------------------------------------------------
// Symbolクラス
// ------------------------------------------------------------
class Symbol
	c++ {
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		// ハッシュコードを計算
		hashCode() :Int
		"=="(x):Bool	        // 比較
	}
end


// ------------------------------------------------------------
// 整数クラス (32bit符号付き)
// ------------------------------------------------------------
class Int
	MIN = 0x80000000
	MAX = 0x7fffffff

	c++ {
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		hashCode() :Int			// ハッシュコード
		"+@"():Int				// 単項プラス
		"-@"():Int				// 単項マイナス
		"+"(x:Int):Int			// 加算
		"-"(x:Int):Int			// 減算
		"*"(x:Int):Int	        // 乗算
		"/"(x:Int):Int	        // 除算
		"%"(x:Int):Int	        // 剰余
		"**"(x:Int):Int			// べき乗
		"=="(x):Bool			// 比較
		"<=>"(x:Int):Int		// 比較 x<=>y: x==yなら0, x<yなら-1, x>yなら1
		"<"(x:Int):Bool			// 比較
		"<="(x:Int):Bool		// 比較
		">"(x:Int):Bool			// 比較
		">="(x:Int):Bool		// 比較
		"&"(x:Int):Int	        // ビットAND
		"|"(x:Int):Int	        // ビットOR
		"^"(x:Int):Int	        // ビットXOR
		"~"():Int				// ビットNOT
		toFloat():Float			// Float化
		abs():Int				//絶対値
	}

	def toInt() { self }

	defFiber times () {
		for (n = 0; n < self; n += 1)
			yield n
	}
	def times (f) {
		for (n = 0; n < self; n += 1)
			f.callAway(n)
	}
	

	defFiber upto(max) {
		for (n = self; n <= max; n += 1)
			yield n
	}
	def upto(max, f) {
		for (n = self; n <= max; n += 1)
			f.callAway(n)
	}

	defFiber downto(min) {
		for (n = self; n >= min; n -= 1)
			yield n
	}
	def downto(min, f) {
		for (n = self; n >= min; n -= 1)
			f.callAway(n)
	}

	defFiber countto(goal) {
		if (self < goal)
			for (n = self; n <= goal; n += 1)
				yield n
		else
			for (n = self; n >= goal; n -= 1)
				yield n
	}
	def countto(goal, f) {
		if (self < goal)
			for (n = self; n <= goal; n += 1)
				f.callAway(n)
		else
			for (n = self; n >= goal; n -= 1)
				f.callAway(n)
	}

			
end

// ------------------------------------------------------------
// 浮動小数クラス (32bit)
// ------------------------------------------------------------
class Float
	c++ {
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		// ハッシュコードを計算
		hashCode():Int
		"+@"():Float			// 単項プラス
		"-@"():Float			// 単項マイナス
		"+"(x:Float):Float		// 加算
		"-"(x:Float):Float		// 減算
		"*"(x:Float):Float		// 乗算
		"/"(x:Float):Float		// 除算
		"**"(x:Float):Float		// べき乗
		"=="(x):Bool	        // 比較
		"<=>"(x:Float):Int		// 比較 x<=>y: x==yなら0, x<yなら-1, x>yなら1
		"<"(x:Float):Bool		// 比較
		"<="(x:Float):Bool		// 比較
		">"(x:Float):Bool		// 比較
		">="(x:Float):Bool		// 比較
		toInt():Int				// Int化 (0に近い整数)
		floor():Int				// Int化 (元の実数を越えない整数)

	}

	def toFloat() { self }

end

// ------------------------------------------------------------
// 数値演算関数群
// ------------------------------------------------------------
class Math
	E = 2.7182818284590452354
	PI = 3.14159265358979323846
	c++ {
		sin(x:Float):Float	// 正弦
		cos(x:Float):Float	// 余弦
		sqrt(x:Float):Float // 平方根
		log(x:Float):Float // 自然対数
	}
end

// ------------------------------------------------------------
// Boolクラス
// ------------------------------------------------------------
class Bool
	c++ {
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		// ハッシュコードを計算
		hashCode() :Int
		"!"():Bool			// 論理否定
		"=="(x):Bool	// 比較

		// ||,&& は通常はコンパイラ内部で処理するが、メソッドとして
		// 呼び出したい場合のために用意
		"||"(x:Bool):Bool
		"&&"(x:Bool):Bool
	}
end


// ------------------------------------------------------------
// Stringクラス
// ------------------------------------------------------------
class String
	c++ {
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		// ハッシュコードを計算
		hashCode() :Int
		"=="(x):Bool		// 比較:StringBufferの内容とも比較できる
		"<=>"(x):Int		// バイト単位で比較
		length():Int
	}
	def "+" (x) {
		sb = new StringBuffer(length() + 16)
		concatToStringBuffer(sb)
		x.concatToStringBuffer(sb)
		sb
	}
end


// ------------------------------------------------------------
// Classクラス
// ------------------------------------------------------------
class Class
	c++ {
		// クラスのシンボル
		classSymbol():Symbol
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		// ハッシュコードを計算
		hashCode() :Int
		"=="(x):Bool	        // 比較
		"<"(x):Bool				// 継承チェック
		"<="(x):Bool			// 継承チェック
		">"(x):Bool				// 継承チェック
		">="(x):Bool			// 継承チェック
	}
end


// ------------------------------------------------------------
// Bytecodeクラス
// ------------------------------------------------------------
class Bytecode
	c++ {
		// バイトコードをリロードする
		// 同じ名前のバイトコードが無ければ、単にロードするだけ
		reload(filename:String):NilClass
		// バイトコードをリロードする
		// 但し本来の名前を指定したシンボルに置き換える
		reloadAs(filename:String, name:Symbol):NilClass
		// リロード時に旧クラスにあって新クラスに無い変数を残すかどうかを設定
		// trueなら残す (初期値false)
		setObsoleteVarKeptOnReload(flag:Bool):NilClass
		// バイトコードをリロードした時に、旧クラス変数の値を
		// 新クラス変数にコピーするかどうか：trueならコピーする (初期値true)
		setClassVarCopiedOnReload(flag:Bool):NilClass
		// バイトコードをリロードした時に、旧定数の値を
		// 新定数にコピーするかどうか：trueならコピーする (初期値true)
		setConstVarCopiedOnReload(flag:Bool):NilClass
		// リロードした後、残っている旧バイトコードの名前をシンボルの配列で返す
		remains():Array
	}
end


// ------------------------------------------------------------
// Randomクラス
// ------------------------------------------------------------
// 手抜き
// シードを1つしか持っていない32bitのM系列乱数(ガロアLFSR)
// 本来は各インスタンスがシードを持ち、アルゴリズムも複数あり、
// シードのserialize,deserializeができるようにするべき。
class Random
	c++ {
		// シードセット (戻り値は x)
		srand(x:Int):Int
		// 現在のシード
		seed():Int

		// rand() --> 乱数を生成して32bit整数を返す
		// rand(x) --> 区間 [0,x) の中で乱数を生成する
		// rand(x,y) --> 区間 [x,y] の中で乱数を生成する
		rand(...):Int
	}
end


// ------------------------------------------------------------
// GCクラス
// ------------------------------------------------------------
class GC
	c++ GC {
		// GCを少し進める
		incremental():NilClass
		// unmark->mark->sweepのフルGCを行なう
		full():NilClass
		// 隣り合ったフリーなセルを結合して大きなセルにまとめる
		coalesce():NilClass

		// 使用セル数(デバッグ版のみ有効)
		countObjects():Int
		// GCフェーズを表す文字列を取得
		getPhaseStr():String
	}
end






// ------------------------------------------------------------
// Closureクラス
// メモリセルを使うタイプのクラスだが、
// Enumerableクラスで使われているため、その前に記述
// ------------------------------------------------------------
class Closure
	c++ Closure {
		includeFile "hyClosure.h"

		"*cppSize"(...):Int

		// 通常呼び出し
		// return文は、このClosureを呼び出したルーチンへのリターンとなる
		call(...)
		"()"(...)				// call()と同じ

		// return文が、このClosureを「生成した」ルーチンからのリターンになる呼び出し
		callAway(...)
	}
	def run() { call() }
end


// ============================================================
// 拡張用クラス
// メンバ変数を持たない。
// 他のクラスに継承される事により使えるようになる。
// Rubyのモジュールに相当。
// ============================================================

// ------------------------------------------------------------
// Enumerableクラス
// ------------------------------------------------------------
// Fiberを返すメソッド each() を作ってこのクラスを継承すれば、
// Fiberを使った各種メソッドを使う事ができる。
class Enumerable
	def each (f) { each().iterate(f) }
	def "->" (f) { each().iterate(f) }

	def toArray ()
	{
		a = []
		each{|x|
			a.push(x)
		}
		return a
	}

	def toList ()
	{
		a = '()
		each{|x|
			a = a.append!(x)
		}
		return a
	}

	defFiber map (appl)
	{
		each{|x|
			yield appl.callAway(x)
		}
	}

	defFiber filter (appl)
	{
		each{|x|
			if (appl.callAway(x))
			yield x
		}
	}

	def find (f)
	{
		each{|x| if (f.callAway(x)) return x}
		nil
	}

	// nilでないものの個数を数える
	def count ()
	{
		n = 0
		each{|x| if (x != nil) n += 1}
		n
	}

	// 指定インスタンス、もしくは条件にマッチしたものの個数を数える
	// 罠：クロージャを数える事はできない(条件式と判断される)
	def count (f)
	{
		n = 0
		if (f.getClass() == Closure)
			each{|x| if (f.callAway(x)) n += 1}
		else
			each{|x| if (f == x) n += 1}
		n
	}

	def index (f)
	{
		i = 0
		if (f.getClass() == Closure) {
			each{|x|
				if (f.callAway(x))
					return i
				i += 1
			}
		} else {
			each{|x|
				if (x == f)
					return i
				i += 1
			}
		}			
		nil
	}

	def most (f = nil)
	{
		if (f == nil)
			f = lambda(a,b){a > b}
		m = nil
		each{|x|
			if (m == nil) {
				m = x
			} else {
				if (f.call(x,m))
					m = x
			}
		}
		m
	}

	defFiber flatten ()
	{
		each{|x|
			if (x.haveMethod?(:flatten)) {
				x.flatten().each{|y| yield y}
			} else {
				yield x
			}
		}
	}

end






// ============================================================
// メモリセルを使うタイプのクラス
// Engine内部では、Objectになる
// ============================================================


// ------------------------------------------------------------
// Arrayクラス
// ------------------------------------------------------------
class Array < Enumerable
	c++ ValueArray {
		// new(n)で生成したインスタンスの初期化
		initialize(initCapacity:Int = 0):Array
		// デストラクタ
		finalize():NilClass
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		//concatToStringBuffer(sb:StringBuffer, mode = 0):StringBuffer
		concatToStringBuffer(...):StringBuffer
		// ハッシュコードを計算
		hashCode() :Int
		// 中身を全部削除
		clear():Array	// 戻り値は自分自身
		// 値取り出し
		"[]"(i:Int)
		// 値代入
		"[]="(i:Int, x)
		// 入っているデータの個数
		size():Int
		"=="(x):Bool	// 比較
		// xを含むかどうか
		include?(x):Bool
		// 値削除: 削除した値を返す
		removeAt(i:Int)
		// == で比較して等しい値を削除: インデックスを返す。無かったら -1
		remove(x):Int
		// 値挿入
		insertAt(i:Int, x)
		// コピーを作成
		clone():Array
		// quick sort
		// sort()は要素の比較に x<=>y 演算を使用する
		// sort(compFunc)は比較に compFunc(x,y) を使用する
		// 比較した値は、マイナスならx<y、0ならx==y、プラスならx>yと解釈
		// ソートした自分自身を返す
		sort!(...):Array
		// sortの前に各要素毎に演算を行ない、その結果によってソートする。
		// rubyのsort_byと同じ。
		// ソートした自分自身を返す。
		sortBy!(fun):Array

		// バイナリ化
		pack(templ:String):StringBuffer
	}

	// 末尾に追加
	def push (x) {
		self[size()] = x
	}
	def add (x) {
		self[size()] = x
	}
	// 末尾を取り除いて返す
	def pop () {
		removeAt(size() - 1)
	}

	// 先頭の値を取り除いて返す
	def shift () {
		removeAt(0)
	}
	// 値を先頭に挿入
	def unshift (x) {
		insertAt(0, x)
	}

	// closureに渡して真だったものを全部削除する
	def removeIf(closure) {
		i = 0
		while (i < size()) {
			if(closure.call(self[i]))
				removeAt(i)
			else
				i += 1
		}
	}

	// 各要素をyieldするFiberを作る	
	defFiber each ()
	{
		sz = size()
		for (i = 0; i < sz; i += 1) {
			yield self[i]
		}
	}

	def toArray() { self }

	// 各要素についてeachを実行し、yieldする
	defFiber eacheach ()
	{
		sz = size()
		for (i = 0; i < sz; i += 1) {
			self[i].each{|x| yield x }
		}
	}
	def eacheach (f) {
		eacheach().iterate(f)
	}

	def map! (appl)
	{
		sz = size()
		for (i = 0; i < sz; i += 1) {
			self[i] = appl.callAway(self[i])
		}
	}


// 	def index (f)
// 	{
// 		sz = size()
// 		if (f.getClass() == Closure) {
// 			for (i = 0; i < sz; i += 1) {
// 				if (f.callAway(self[i])) return i
// 			}
// 		} else {
// 			for (i = 0; i < sz; i += 1) {
// 				if (self[i] == f) return i
// 			}
// 		}
// 		nil
// 	}

	def shuffle! ()
	{
		for (i = size() - 1; i > 0; i -= 1) {
			j = Random.rand(i+1)
			if (i != j) {
				self[i],self[j] = self[j],self[i]
			}
		}
		self
	}

end


// ------------------------------------------------------------
// BitArrayクラス
// ------------------------------------------------------------
class BitArray < Enumerable
	c++ BitArray {
		includeFile "hyBitArray.h"
		// 容量を指定してfalseで初期化
		initialize(size:Int):BitArray
		// デストラクタ
		finalize():NilClass
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		//concatToStringBuffer(sb:StringBuffer, mode = 0):StringBuffer
		concatToStringBuffer(...):StringBuffer
		// ハッシュコードを計算
		hashCode() :Int
		// 中身を全部同じ値にする
		setAll(b:Bool):BitArray	//戻り値は自分自身
		// 容量
		size():Int
		// 容量を変更する: 増えた場合は増えた場所をfalseで初期化
		changeSize(newSize:Int):BitArray //戻り値は自分自身
		// 比較
		"=="(x):Bool
		// 値取り出し
		// 範囲外アクセスは :array_index_out_of_bounds 例外
		"[]"(idx:Int):Bool
		// 値代入
		// 範囲外アクセスは :array_index_out_of_bounds 例外
		// x は false,nil の場合に false を代入、それ以外は true を代入
		"[]="(idx:Int, x):Bool
		// [idx1,idx2]の範囲内の値を全部xにセット
		setRange(idx1:Int, idx2:Int, x:Bool):BitArray //戻り値は自分自身
		// コピーを作成
		clone():BitArray
	}

	defFiber each ()
	{
		sz = capacity()
		for (i = 0; i < sz; i += 1) {
			yield self[i]
		}
	}

end

// ------------------------------------------------------------
// StringBufferクラス
// ------------------------------------------------------------
class StringBuffer
	c++ StringBuffer {
		// new(n)で生成したインスタンスの初期化
		initialize(bufSize:Int = 64):StringBuffer
		// デストラクタ
		finalize():NilClass
		// ハッシュコードを計算
		hashCode() :Int
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		// mode<0: 型情報を付加
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		// バッファサイズ = バッファを拡張せずに入る最大文字列長
		bufSize():Int
		// 文字列長
		length():Int
		// 引数オブジェクトを文字列化して末尾に追加
		concat(...):StringBuffer
		// バッファを空にする
		clear():StringBuffer	// 戻り値は自分自身
		"=="(x):Bool	// 比較:Stringとも比較できる
		"<=>"(x):Int    // バイト単位で比較:Stringとも比較できる
		// sayCommand開始
		sayCommandStart(x:Int, n:Int):StringBuffer
		// sayCommandデータ受け取り文字列化して末尾に追加
		sayCommand(x):StringBuffer
		// sayCommand終了
		sayCommandEnd():StringBuffer

		// バイナリから復元
		unpack(templ:String):Array
	}
	def "+" (x) {
		sb = new StringBuffer(length() + 16)
		sb.concat(self)
		sb.concat(x)
		sb
	}
	// ！注意！
	// x += y は、x = x + y と同じなので、新しい StringBuffer を生成して
	// そこへの追加となる。このため、 x.concat(y) とは結果が異なる
end


// ------------------------------------------------------------
// StringReaderクラス
// 文字列からデータを読み取る
// ------------------------------------------------------------
class StringReader
	c++ StringReader {
		includeFile "hyStringReader.h"

		// コンストラクタ : str は String もしくは StringBuffer
		initialize(str):StringReader
		// 読み取り位置
		pos():Int
		// 読み取り位置を設定。設定できない位置だったら、-1を返す
		pos=(newPos:Int):Int
		// 文字列の全体長
		length():Int
		// 1バイト取得
		getc():Int
		// delimを終端とする文字列取得
		gets(delim:String = "\n"):StringBuffer
		// cがIntならその文字を、Stringならその中に含まれる1文字をサーチして
		// posをそこに移動してtrueを返す。見付からなければ移動せずfalseを返す
		search(c):Bool
		// cがIntならその文字を、StringやStringBufferならそれに含まれる
		// 文字を読み飛ばしてposを移動し、posを返す。
		skip(c):Int
		// c1 <= c <= c2 である間、posを進め、posを返す。
		skip(c1:Int, c2:Int):Int
		// 部分文字列を取得 (headを含み、tailを含まない)
		substr(head:Int,tail:Int):StringBuffer
		// posからlenバイトの部分文字列を取得、posはその次の位置に移動
		read(len:Int):StringBuffer

		// 整数を読む
		getInt():Int
		// 浮動小数を読む
		getFloat():Float
	}
end

// ------------------------------------------------------------
// Hashクラス
// ------------------------------------------------------------
class Hash
	c++ Hash {
		// new(bucketSize, loadFactor)で生成したインスタンスの初期化
		initialize(bucketSize:Int = 31, loadFactor:Float = 0.75):Hash
		// デストラクタ
		finalize():NilClass
		// ハッシュコードを計算
		hashCode() :Int
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		// keyに関連付けられた値を取り出す
		"[]"(key)
		// 関連付け
		"[]="(key, val)
		// 入っているデータの個数
		size():Int
		// バケットの容量
		capacity():Int
		"=="(x):Bool	// 比較
		// 全keyを配列に入れる
		keys():Array
		// 関連付けられた全要素を配列に入れる
		values():Array
		// ハッシュ再計算再構築
		rehash():NilClass
	}
end


// ------------------------------------------------------------
// Listクラス
// ------------------------------------------------------------
class List < Enumerable
	c++ {
		// newで生成するインスタンスのセルサイズ
		"*cppSize"():Int // 引数は0個
		// ハッシュコードを計算
		hashCode() :Int
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		// List.cons(hd,tl) リスト(tl)の先頭に要素(hd)を追加
		// x.cons(hd) 先頭に要素(hd)を追加した新しいリストを返す : 自分自身(x)は変化しない
		cons(...):List
		// 空リストだったらtrue
		empty?():Bool
		// 先頭要素を取得 : 空リストだったら例外発生
		head()
		// 先頭以外を取得 : 空リストだったら例外発生
		tail()
		// リスト結合した新しいリストを作成 : 元のリストに変化は無し
		append(x:List):List
		// 末尾に要素を追加 : 変化した自分自身を返す
		// '() は決して変化しないので、 a = '(); a.append!(x) では
		// aの値は変化しない。 a = '(y); a.append!(x) では変化する。
		// 常に a = a.append!(x) というように使うと良い。
		// この仕様は後で変更したい。
		append!(x):List
		// コピーを作成
		clone():List
		"=="(x):Bool	// 内容比較
	}
	def toList () { self }

	defFiber each ()
	{
		for (l = self; ! l.empty?(); l = l.tail()) {
			yield l.head()
		}
		'()
	}

end


// ------------------------------------------------------------
// Stackクラス
// ------------------------------------------------------------
class Stack
	c++ ValueStack {
		// new(n)で生成したインスタンスの初期化
		initialize(initCapacity:Int = 0):Stack
		// デストラクタ
		finalize():NilClass
		// ハッシュコードを計算
		hashCode():Int
		// 空にする
		clean():Stack
		// push
		push(x):Stack
		// pop
		pop()
		// トップを得る
		getTop()
		// トップからn個削除
		drop(n:Int):NilClass
		// 中身の個数
		size():Int
	}
end


// ------------------------------------------------------------
// Threadクラス
// ------------------------------------------------------------
class Thread
	c++ Thread {
		// newで生成したインスタンスの初期化
		// 引数を渡すと、それをstartRun()に渡して実行開始
		initialize(startInstance = nil):Thread
		// デストラクタ
		finalize():NilClass
		// 引数オブジェクトの run() メソッドから実行開始
		startRun(x):NilClass
		// 実行を中断する
		kill():NilClass
		// sleep状態にする
		sleep():NilClass
		// sleep状態から実行を再開する
		wakeup():NilClass
		// 実行中か？ (wait, sleep も実行中扱い)
		running?() :Bool
		// 終了するまで待つ
		join():NilClass

		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		// ハッシュコードを計算
		hashCode() :Int
	}
end

// ------------------------------------------------------------
// Contextクラス
// ------------------------------------------------------------
class Context
	c++ Context {
		initialize():Context
		finalize():NilClass
	}
end

// ------------------------------------------------------------
// Exceptionクラス
// ------------------------------------------------------------
class Exception
	c++ Exception {
		//includeFile "hyException.h"
		// new(n)で生成したインスタンスの初期化
		initialize(val = nil, type:Symbol = :exception):Exception
		// デストラクタ
		finalize():NilClass
		// ハッシュコードを計算
		hashCode() :Int
		// 引数のStringBufferオブジェクトの末尾に文字列表現を追加
		concatToStringBuffer(sb:StringBuffer, mode:Int = 0):StringBuffer
		// タイプシンボルを取得
		type():Symbol
		// 保持するオブジェクトを設定
		setVal(v):NilClass
		// 保持するオブジェクトを取得
		val()
		// typeが :goto の時のみ、飛び先ラベルを取得。そうでなければ nil
		getLabel()
		// backtraceの個数
		backTraceSize():Int
		// backtrace情報をStringBufferで取得 (0 <= i < backTraceSize())
		// i = 0 が一番深いレベルでExceptionが発生した場所
		// i が範囲を越えていたら nil が返る
		backTraceStr(i:Int)
	}	
end

// ------------------------------------------------------------
// Methodクラス
// ------------------------------------------------------------
class Method
	c++ Method {
		includeFile "hyMethod.h"

		initialize(val, methodSym:Symbol):Method
		// 呼び出し
		call(...)
		"()"(...)				// call()と同じ

	}
end

// ------------------------------------------------------------
// Fiberクラス
// ------------------------------------------------------------
class Fiber < Enumerable
	c++ Fiber {
		includeFile "hyFiber.h"

		initialize(closure:Closure):Fiber

		go():Bool				// yieldまたはreturnまで実行、yieldならtrue、returnならfalse、既に終了していたら fiber_error 例外
		getYieldVal()			// yield値を取得
		getReturnVal()			// return値を取得
		yieldResult(...):NilClass	// yiledに戻り値を与える

		generate()	 // 次のyield値を取り出す : returnなら例外を投げる

		// yield値をclosureに与える事を繰り返す。
		// closureがnilの時は、selfを返す。
		iterate(closure:Closure)

		// yieldに対してExceptionを投げる
		thrown(e:Exception)

		// closureをcallAway()で呼び出し、その中で発生した全ての例外と
		// JumpControlをthrown()に渡す。例外等がthrown()で処理されたら
		// 次のgo()に移動するためにnextを実行、処理されずに
		// 漏れたらその例外を受け取る。
		// 最初の引数がclosure,以降の引数はclosureにそのまま渡す
		stakeCall(...)

		finished?():Bool
	}

//	// yield値をclosureに与える事を繰り返す。
//	// closureがnilの時は、selfを返す。
// 	def iterate (closure)
// 	{
// 		if (closure == nil)
// 			return self
// 		loop {
// 			// stakeCall中で例外が実行されると、
// 			// fiber中のyield文に対してそれを全部投げた上で、
// 			// yieldResultは実行されずにループのこの位置に戻る。
// 			if (! go()) return getReturnVal()
// 			yieldResult(stakeCall(closure, getYieldVal()))
// 		}
// 	}

	def generate_noThrow ()
	{
		if (finished?()) {
		    return nil
		} else if (go()) {
			getYieldVal()
		} else {
			getReturnVal()
		}
	}

	def each () { self }

	def each (f) { iterate(f) }
	def "->" (f) { iterate(f) }
	def "()" (f) { iterate(f) }



	defFiber take(n)
	{
		if (n <= 0) return 0
		i = 1
		iterate{|x|
			yield x
			if (i >= n)
				return n
			i += 1
		}
	}


	// class method
	defFiber joint (a, b)
	{
		a.each{|x| yield x }
		b.each{|x| yield x }
	}


end
