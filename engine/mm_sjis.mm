Mm ?abcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDabcdABCDArray index %d out of bounds (size=%d) 配列のインデックス%dが範囲を越えました (配列サイズ=%d) Array index out of bounds 配列のインデックスが範囲を越えました Array index out of bounds: idx1 = %d, idx2 = %d, array size = %d インデックス %d または %d は配列の範囲外です: 配列サイズ = %d Array index out of bounds: index = %d, array size = %d インデックス %d は配列の範囲外です: 配列サイズ = %d Bad FFI call %s::%s
sp=%d,numRetVal=%d,newsp=%d FFI呼び出し %s::%s の結果が異常です
sp=%d,numRetVal=%d,newsp=%d Closure required クロージャが必要です PROGRAM BUG :  ThreadManager::firstOfAll() must called first 初期化ルーチンにバグがあります: まず ThreadManager::firstOfAll() が呼ばれなければなりません StringBuffer overflow detected StringBufferのバッファオーバーフローが検出されました addrAt(%d) stack over (sp %d) addrAt(%d) はスタックの範囲を越えています(sp=%d) already finished Fiberは既に終了しています argument of BitArray::changeSize() must >= 0 BitArray::changeSize()の引数は、0以上でなければなりません assertion failed assert失敗 call alloc() inside alloc() : finalize may called alloc() alloc()の内側でalloc()が呼ばれています: finalizeでalloc()が呼ばれている可能性があります call head() of empty List 空リストに対して head() を呼びました call tail() of empty List 空リストに対して tail() を呼びました cannot access constant '%s' from class '%s' 定数 '%s' は、クラス '%s' からはアクセスできません cannot access constant '%s::%s' 定数 '%s::%s' にはアクセスできません cannot get member or call method '%s' in class '%s' メンバ変数またはメソッド '%s' は、クラス '%s' には存在しません cannot set member '%s' or call setter '%s' in class '%s' メンバ変数 '%s' またはsetterメソッド '%s' は、クラス '%s' には存在しません class %s not initialized for default value クラス '%s' が初期化されていないので、メソッドのデフォルト値を使えません class method call for instance method インスタンスメソッド呼び出しを想定しているのにクラスメソッド呼び出しをされました class method not found or signature mismatch: %s::%s メソッドが無いか、シグネチャが一致しません: %s::%s closure call signature mismatch クロージャ呼び出しのシグネチャが一致しません delete BUG !!  in class %s
 クラス '%s' で、呼んではいけない delete を呼んでいます division by zero 0で除算しました ffi table mismatch: table has entry '%s' but class %s does not have it FFIのテーブルに不整合があります: テーブルに '%s' がありますが、クラス '%s' には存在しません fiber loop 自分自身を再帰的に実行しようとしました fiber was finished Fiberは既に終了しています file %s reload as %s failed ファイル%sのリロードに失敗しました(%sに置き換えてのリロード) file %s reload failed ファイル%sのリロードに失敗しました frameSP=%d,frameStack.sp=%d frameSP=%d,frameStack.sp=%d generate() loop 自分自身を再帰的に実行しようとしました getAt(%d) stack over (sp %d) getAt(%d) はスタックの範囲を越えています(sp=%d) getReturnVal() called but not returned FiberがreturnしていないのにgetReturnVal()が呼ばれました getScopeClass: class not found: %s クラス '%s' が見つかりません getScopeClass: inner class not found: %s インナークラス '%s' が見つかりません getYieldVal() called but not yielding FiberがyieldしていないのにgetYieldVal()が呼ばれました initialize() must return an object of class '%s' initialize() は、'%s' クラスのインスタンスを返さなければなりません instance method call for a class method クラスメソッド呼び出しを想定しているのにインスタンスメソッド呼び出しをされました instance method not found or signature mismatch: %s::%s メソッドが無いか、シグネチャが一致しません: %s::%s internal error: incorrect finally process 内部エラー:finally処理にバグがあるようです no link bytecode found %s バイトコード '%s' がみつかりません no method %s found in scope %s.  self.type=%s メソッド '%s' は、スコープ '%s' には見つかりません。 self.type=%s offs=%d m_stringTableSize=%d offs=%d m_stringTableSize=%d reload feature not available リロードの使用が禁止されています return assertion failed:
  context=%x, sp=%d, framePos=%d, numArgs=%d, nulLocals=%d return時スタック異常
 context=%x, sp=%d, framePos=%d, numArgs=%d, nulLocals=%d setAt(%d) stack over (sp %d) etAt(%d) はスタックの範囲を越えています(sp=%d) sleep() called out of thread スレッド実行でないのにsleep()を実行しました srand(0) is not allowed sramd(0)は禁止です。0以外を使用しなければなりません stack size expected %d but actually %d コンストラクタ実行後のスタックサイズは %d であるべきなのに %d になっています stack underflow スタックの下限を越えました substitution to Constant '%s' not allowed 定数 '%s' への代入は禁止されています super class %s not found スーパークラス '%s' が見つかりません symbol table is not initialized シンボルテーブルが初期化されていません uncaught exception 例外が処理されずに終了しました unknown class '%s' 不明なクラス '%s' unknown member var %s (class %s) メンバ変数 '%s' はクラス '%s' にはありません using class %s not found usingのクラス '%s' が見つかりません using class ...::%s not found usingのクラス '〜::%s' が見つかりません using class ::%s not found usingのクラス '::%s' が見つかりません using member variable '%s' out of instance インスタンスが無いのにメンバ変数 '%s' を使おうとしました yield out of fiber Fiberの外で yield を実行しようとしました yieldResult() called but not yielded yieldしていないのにyieldResult()が呼ばれました 