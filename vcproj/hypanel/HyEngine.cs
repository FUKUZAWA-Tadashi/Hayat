using System;
using System.Text;
using System.Runtime.InteropServices;

namespace hypanel
{
    class HyEngine
    {
        // Hayatで使用するメモリサイズを指定する
        // 指定しなければ、4MiBのメモリを確保する
        [DllImport("hydll.dll")]
        public extern static void  setMemSize(uint memSize);

        // デバッグ用情報をロードするかどうかを設定
        // falseだと、setDebugMemSize()を呼んでもデバッグ用メモリは確保しない。
        // trueだと、シンボル情報、デバッグ情報、SJISエラーメッセージ を読む。
        //     setDebugMemSize()を呼んでいない場合はメインのメモリに読む。
        [DllImport("hydll.dll")]
        public extern static void  setFlagReadDebugInfos(bool flag);

        // Hayatで使用するデバッグ用メモリサイズを指定する
        // 指定しなければ、デバッグ用メモリは確保しない
        [DllImport("hydll.dll")]
        public extern static void  setDebugMemSize(uint MemSize);

        // メモリ初期化
        [DllImport("hydll.dll")]
        public extern static void  init_memory();

        // ロードパスを追加する
        [DllImport("hydll.dll")]
        public extern static void addLoadPath([MarshalAs(UnmanagedType.LPStr)] string path);

        // 基本初期化
        [DllImport("hydll.dll")]
        public extern static void  init_main();

        // 指定バイトコードファイルを読み込んで頭から新規スレッドで実行
        [DllImport("hydll.dll")]
        public extern static bool start_main([MarshalAs(UnmanagedType.LPStr)] string hybFilename);

        // スクリプトを 1tick 実行
        [DllImport("hydll.dll")]
        public extern static bool  tick_main();

        // 終了処理
        [DllImport("hydll.dll")]
        public extern static void  term_main();

        // バイトコードリロード
        [DllImport("hydll.dll")]
        public extern static bool  reload_bytecode([MarshalAs(UnmanagedType.LPStr)] string path, bool bInit);
    }
}
