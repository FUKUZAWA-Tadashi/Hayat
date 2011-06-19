コンパイルには、platform SDK (user32.lib) が必要です。
参考： http://www.cppdrive.jp/vc/sdk/index.html
       ダウンロード、インストール、パスの設定までを実行します。
Visual C++ 2008 には同梱されている模様。



テストコードのコンパイルには、CppUnit のインストールが必要です。
$(VCInstallDir)CppUnit にインストールされているという事にしてあります。
それ以外の場所にインストールした場合は、各テストプロジェクト
(commontest, compilertest, enginetest, parsertest)のプロパティから、
追加のインクルードディレクトリと追加のライブラリディレクトリを変更して下さい。

参考： http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_01.html



テストを実行しない場合は、libhayat をビルドすればOKです。
(自動的にhoukenとcompilerもビルドされます。)
DebugとReleaseの両方でビルドして下さい。
