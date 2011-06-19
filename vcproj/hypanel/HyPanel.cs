using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Threading;

using System.Reflection;
using System.Runtime.InteropServices;
using System.Xml.Serialization;



namespace hypanel
{
    public partial class HyPanel : Form
    {
        delegate void VoidDelegate();

        private CompilerConf conf_compiler;
        private EngineConf conf_engine;

        // Hayatエンジンスレッドに渡すメッセージ
        private struct MtMsg {
            public enum Type {
                RELOAD,
                STOP
            }
            public Type mt;
            public string str0;
            public bool flag0;
        }

        protected TextWriter m_tw;
        protected Thread     m_scriptThread;
        private static Queue<MtMsg> m_msgq = new Queue<MtMsg>();


        private EngineOption  m_engineOptionDialog;
        private CompileOption m_compileOptionDialog;

        public HyPanel()
        {
            InitializeComponent();

            conf_compiler = new CompilerConf();
            conf_compiler.compilerPath = @"C:\proj\hayat\vcproj\hayatc.exe";
            conf_compiler.memSize = 16;
            conf_compiler.inputCharset = "SJIS";
            conf_compiler.outputCharset = "SJIS";
            conf_compiler.includePaths = new string[] {
                @"C:\proj\hayat\hyznlib\hyznlib\out",
                @"C:\proj\hayat\stdlib\out"
            };
            conf_compiler.warnLabelScope = 0;
            conf_compiler.warnLabelLevel = 1;
            conf_compiler.warnNumsayLimit = 0;
            conf_compiler.warnNumsayLevel = 0;
            conf_compiler.warnUnknownLevel = 1;
            conf_compiler.flagVerbose = false;
            conf_compiler.flagPrintSyntaxTree = false;
            conf_compiler.debugPrintParse = 0;

            conf_engine = new EngineConf();
            conf_engine.memSize = 16;
            conf_engine.debugMemSize = 16;
            conf_engine.flagReadDebugInfo = true;
            conf_engine.includePaths = new string[] {
                @"C:\proj\hayat\hyznlib\hyznlib\out",
                @"C:\proj\hayat\stdlib\out"
            };
            conf_engine.additionalPATH = new string[] {};


            m_engineOptionDialog = new EngineOption();
            m_engineOptionDialog.OK += this.engineOptionDialog_OK_Click;

            m_compileOptionDialog = new CompileOption();
            m_compileOptionDialog.OK += this.compileOptionDialog_OK_Click;

            m_scriptThread = null;

            // Console出力をTextBoxに書き出す
            Encoding enc = Encoding.GetEncoding("UTF-8");
            StreamWriter sw = new StreamWriter(new TextBoxStream(logBox, enc));
            sw.AutoFlush = true;
            TextWriter tw = TextWriter.Synchronized(sw);
            m_tw = tw;
            Console.SetOut(tw);

            Console.WriteLine("HyPanel起動");

            // DLL側の標準出力/標準エラー出力をConsoleにリダイレクトする
            HyW32.redirectStdOutputs();
        }

        protected override void OnClosed(EventArgs e)
        {
            if (m_scriptThread != null) {
                StopScript();
            }            
            base.OnClosed(e);
        }


        private void openFileDialog1_FileOk(object sender, CancelEventArgs e)
        {
            hayatFileName.Text = openScriptDialog.FileName;
        }

        private void selectFileButton_Click(object sender, EventArgs e)
        {
            openScriptDialog.ShowDialog();
        }

        private void 開くOToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openScriptDialog.ShowDialog();
        }

        private void 終了XToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (m_scriptThread != null) {
                StopScript();
            }            
            Environment.Exit(0);
        }

        private void compileOptButton_Click(object sender, EventArgs e)
        {
            m_compileOptionDialog.conf = conf_compiler;
            m_compileOptionDialog.ShowDialog();
        }

        private void compileOptionDialog_OK_Click()
        {
            conf_compiler = m_compileOptionDialog.conf;
        }

        private void engineOptButton_Click(object sender, EventArgs e)
        {
            m_engineOptionDialog.conf = conf_engine;
            m_engineOptionDialog.ShowDialog();
        }

        private void engineOptionDialog_OK_Click()
        {
            conf_engine = m_engineOptionDialog.conf;
        }



        private void execButton_Click(object sender, EventArgs e)
        {
            if (m_scriptThread == null) {
                if (!File.Exists(hayatFileName.Text)) {
                    Console.WriteLine("file not found");
                    return;
                }
                string outDir = Path.Combine(Path.GetDirectoryName(hayatFileName.Text), "out");
                string hybName = Path.ChangeExtension(Path.GetFileName(hayatFileName.Text), ".hyb");
                if (!File.Exists(Path.Combine(outDir, hybName))) {
                    Console.WriteLine("not compiled");
                    return;
                }
                m_scriptThread = new Thread(new ThreadStart(ExecuteScript));
                m_scriptThread.Start();
                lock (execButton) {
                    execButton.Text = "中断(&X)";
                    実行EToolStripMenuItem.Text = "中断(&X)";
                    reloadButton.Enabled = true;
                    リロードRToolStripMenuItem.Enabled = true;
                }
            } else {
                StopScript();
                restoreExecButton();
            }
        }

        private void restoreExecButton()
        {
            if (InvokeRequired) {
                Invoke(new VoidDelegate(restoreExecButton));
            } else {
                lock (execButton) {
                    execButton.Text = "実行(&E)";
                    実行EToolStripMenuItem.Text = "実行(&E)";
                    reloadButton.Enabled = false;
                    リロードRToolStripMenuItem.Enabled = false;
                }
            }
        }


        private static void CommandOutputDataHandler(object sendingProcess, DataReceivedEventArgs outLine)
        {
            if (!String.IsNullOrEmpty(outLine.Data)) {
                Console.WriteLine(outLine.Data);
            }
        }

        private void compileButton_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(conf_compiler.compilerPath)) {
                Console.WriteLine("コンパイラパスが指定されていません");
                return;
            }
            if (String.IsNullOrEmpty(hayatFileName.Text)) {
                Console.WriteLine("スクリプトファイルが指定されていません");
                return;
            }
            string dirName = Path.GetDirectoryName(hayatFileName.Text);
            string argstr = conf_compiler.getCompileArgument();

            Console.WriteLine("{0} {2} {3}",
                              conf_compiler.compilerPath,
                              dirName,
                              argstr,
                              hayatFileName.Text);

            Process compProc = new Process();
            compProc.StartInfo.FileName = conf_compiler.compilerPath;
            compProc.StartInfo.Arguments = argstr + " " + hayatFileName.Text;
            compProc.StartInfo.WorkingDirectory = dirName;
            compProc.StartInfo.UseShellExecute = false;
            compProc.StartInfo.RedirectStandardOutput = true;
            compProc.StartInfo.RedirectStandardError = true;
            compProc.OutputDataReceived += new DataReceivedEventHandler(CommandOutputDataHandler);
            compProc.ErrorDataReceived += new DataReceivedEventHandler(CommandOutputDataHandler);
            compProc.StartInfo.CreateNoWindow = true;
            compProc.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;

            compProc.Start();
            compProc.BeginOutputReadLine();
            compProc.BeginErrorReadLine();

            //compProc.WaitForExit();
            //compProc.Close();
        }



        private void StopScript()
        {
            MtMsg msg = new MtMsg();
            msg.mt = MtMsg.Type.STOP;
            lock (m_msgq) {
                m_msgq.Enqueue(msg);
            }
        }

        private void ScriptStopped()
        {
            if (InvokeRequired) {
                Invoke(new VoidDelegate(ScriptStopped));
            } else {
                if (m_scriptThread != null) {
                    m_scriptThread.Abort();
                    //m_scriptThread.Join();
                    m_scriptThread = null;
                }
                restoreExecButton();
            }
        }

        private void ExecuteScript()
        {
            try {
                string path = Environment.GetEnvironmentVariable("PATH") + ";" + String.Join(";", conf_engine.additionalPATH);;
                Environment.SetEnvironmentVariable("PATH", path);
            } catch (Exception e) {
                Console.WriteLine("環境変数PATHの設定に失敗しました: {0}", e);
                ScriptStopped();
                return;
            }

            try {
                HyEngine.setMemSize((uint)conf_engine.memSize * 1024 * 1024);
                HyEngine.setDebugMemSize((uint)conf_engine.debugMemSize * 1024 * 1024);
                HyEngine.setFlagReadDebugInfos(conf_engine.flagReadDebugInfo);
                HyEngine.init_memory();
            } catch (DllNotFoundException e) {
                Console.WriteLine("dllの起動に失敗しました: {0}", e);
                ScriptStopped();
                return;
            } catch (Exception e) {
                Console.WriteLine("Hayatエンジンの初期化に失敗しました: {0}", e);
                ScriptStopped();
                return;
            }

            try {
                string dirName = Path.GetDirectoryName(hayatFileName.Text);
                string hyName = Path.GetFileName(hayatFileName.Text);
                string hybName = Path.ChangeExtension(hyName, ".hyb");
                Directory.SetCurrentDirectory(dirName);
                HyEngine.addLoadPath(Path.Combine(dirName, "out"));
                foreach (string path in conf_engine.includePaths) {
                    HyEngine.addLoadPath(path);
                }                

                HyEngine.init_main();
                HyEngine.start_main(hybName);

                long lastTick = DateTime.Now.Ticks;
                while (HyEngine.tick_main()) {
                    int waitMS = (1000/60) - (int)((DateTime.Now.Ticks - lastTick) / 10000);
                    if (waitMS > 0)
                        {
                            // Console.WriteLine("sleep {0} ms",waitMS);
                            Thread.Sleep(waitMS);
                        }
                    else
                        Thread.Sleep(0);

                    lock (m_msgq) {
                        while (m_msgq.Count > 0) {
                            MtMsg msg = m_msgq.Dequeue();
                            if (msg.mt == MtMsg.Type.RELOAD) {
                                if (HyEngine.reload_bytecode(msg.str0, msg.flag0))
                                    Console.WriteLine("バイトコード {0} リロード成功", msg.str0);
                                else
                                    Console.WriteLine("バイトコード {0} リロード失敗", msg.str0);
                            } else if (msg.mt == MtMsg.Type.STOP) {
                                goto Stop;
                            }
                        }
                    }

                    lastTick = DateTime.Now.Ticks;
                }
            } catch (Exception e) {
                Console.WriteLine("Hayatエンジンが例外を送出: {0}", e);
            }                

        Stop:
            try {
                HyEngine.term_main();
            } catch (Exception e) {
                Console.WriteLine("Hayatエンジンの停止に失敗: {0}", e);
            }
            ScriptStopped();
        }

        private void logBox_TextChanged(object sender, EventArgs e)
        {
            logBox.Select(logBox.TextLength, logBox.TextLength);
            logBox.ScrollToCaret();
        }

        private void logClearButton_Click(object sender, EventArgs e)
        {
            logBox.Clear();
        }

        private void reloadButton_Click(object sender, EventArgs e)
        {
            MtMsg msg = new MtMsg();
            msg.mt = MtMsg.Type.RELOAD;
            msg.str0 = Path.ChangeExtension(Path.GetFileName(hayatFileName.Text), ".hyb");
            msg.flag0 = checkBox_initOnReload.Checked;
            lock (m_msgq) {
                m_msgq.Enqueue(msg);
            }
        }


        private void ロードLToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openConfDialog.ShowDialog();
        }
        private void openConfDialog_FileOk(object sender, CancelEventArgs e)
        {
            if (String.IsNullOrEmpty(openConfDialog.FileName)) {
                return;
            }
            Console.Write("環境を {0} からロード", openConfDialog.FileName);
            try {
                TextReader rd = new StreamReader(openConfDialog.FileName);
                XmlSerializer ser = new XmlSerializer(typeof(Confs));
                Confs confs = (Confs)ser.Deserialize(rd);
                rd.Close();
                conf_compiler = confs.conf_compiler;
                conf_engine = confs.conf_engine;
                hayatFileName.Text = confs.conf_panel.scriptPath;
                checkBox_initOnReload.Checked = confs.conf_panel.flagInitOnReload;
            } catch {
                Console.WriteLine("... 失敗");
                return;
            }
            Console.WriteLine("... 成功");
            saveConfDialog.FileName = openConfDialog.FileName;
        }

        private void セーブSToolStripMenuItem_Click(object sender, EventArgs e)
        {
            saveConfDialog.ShowDialog();
        }
        private void saveConfDialog_FileOk(object sender, CancelEventArgs e)
        {
            if (String.IsNullOrEmpty(saveConfDialog.FileName)) {
                return;
            }
            Console.Write("環境を {0} にセーブ", saveConfDialog.FileName);
            try {
                TextWriter wr = new StreamWriter(saveConfDialog.FileName);
                XmlSerializer ser = new XmlSerializer(typeof(Confs));
                Confs confs = new Confs();
                confs.conf_compiler = conf_compiler;
                confs.conf_engine = conf_engine;
                confs.conf_panel = new PanelConf();
                confs.conf_panel.scriptPath = hayatFileName.Text;
                confs.conf_panel.flagInitOnReload = checkBox_initOnReload.Checked;
                ser.Serialize(wr, confs);
                wr.Close();
            } catch {
                Console.WriteLine("... 失敗");
                return;
            }
            Console.WriteLine("... 成功");
            openConfDialog.FileName = saveConfDialog.FileName;
        }

        private void cleanButton_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(hayatFileName.Text)) {
                Console.WriteLine("Hayatスクリプトが指定されていません");
                return;
            }
            string dir = Path.GetDirectoryName(hayatFileName.Text);
            string outDir = Path.Combine(dir, "out");
            Console.WriteLine("ディレクトリ {0} を削除", outDir);
            try {
                Directory.Delete(outDir, true);
            } catch {
            }

            string ffioutDir = Path.Combine(dir, "ffiout");
            Console.WriteLine("ディレクトリ {0} を削除", ffioutDir);
            try {
                Directory.Delete(ffioutDir, true);
            } catch {
            }

        }

    }
}
