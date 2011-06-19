using System;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace hypanel
{
    public partial class CompileOption : Form
    {
        public delegate void OKEventHandler();
        public event OKEventHandler OK;

        public CompileOption()
        {
            InitializeComponent();
        }

        private void openFileDialog1_FileOk(object sender, CancelEventArgs e)
        {
            f_compilerPath.Text = openFileDialog1.FileName;
        }

        private void compilerPathSelectButton_Click(object sender, EventArgs e)
        {
            openFileDialog1.ShowDialog();
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            if (OK != null)
                OK();
            Close();
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
            Close();
        }

        private static string[] PATHS_SEP = new string[] {"\r\n"};
        private static string[] s_wLabelScope = new string[] {
            "ファイル内チェック",   "ブロック内チェック" };
        private static string[] s_wLevel = new string[] {
            "チェックしない",   "警告", "エラー" };
        private static string[] s_debugPrintParse = new string[] {
            "表示しない",   "表示", "詳細表示" };

        public CompilerConf conf
        {
            set {
                f_compilerPath.Text = value.compilerPath;
                memSize.Value = (decimal)value.memSize;
                inputCharset.Text = value.inputCharset;
                outputCharset.Text = value.outputCharset;
                f_paths.Text = String.Join(PATHS_SEP[0], value.includePaths);
                wLabelScope.Text = s_wLabelScope[value.warnLabelScope];
                wLabelLevel.Text = s_wLevel[value.warnLabelLevel];
                wNumsayLimit.Value = (decimal)value.warnNumsayLimit;
                wNumsayLevel.Text = s_wLevel[value.warnNumsayLevel];
                wUnknownLevel.Text = s_wLevel[value.warnUnknownLevel];
                verbose.Checked = value.flagVerbose;
                printSyntaxTree.Checked = value.flagPrintSyntaxTree;
                debugPrintParse.Text = s_debugPrintParse[value.debugPrintParse];

            }

            get {
                CompilerConf x = new CompilerConf();
                x.compilerPath = f_compilerPath.Text;
                x.memSize = (int)memSize.Value;
                x.inputCharset = inputCharset.Text;
                x.outputCharset = outputCharset.Text;
                x.includePaths = f_paths.Text.Split(PATHS_SEP, StringSplitOptions.RemoveEmptyEntries);
                x.warnLabelScope = Array.IndexOf(s_wLabelScope, wLabelScope.Text);
                x.warnLabelLevel = Array.IndexOf(s_wLevel, wLabelLevel.Text);
                x.warnNumsayLimit = (int)wNumsayLimit.Value;
                x.warnNumsayLevel = Array.IndexOf(s_wLevel, wNumsayLevel.Text);
                x.warnUnknownLevel = Array.IndexOf(s_wLevel, wUnknownLevel.Text);
                x.flagVerbose = verbose.Checked;
                x.flagPrintSyntaxTree = printSyntaxTree.Checked;
                x.debugPrintParse = Array.IndexOf(s_debugPrintParse, debugPrintParse.Text);
                return x;
            }
        }

        private void CompileOption_Load(object sender, EventArgs e)
        {

        }

    }
}
