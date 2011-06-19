using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace hypanel
{
    public partial class EngineOption : Form
    {
        public delegate void OKEventHandler();
        public event OKEventHandler OK;

        public EngineOption()
        {
            InitializeComponent();
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



        private static String[] PATHS_SEP = new String[] {"\r\n"};

        public EngineConf conf
        {
            set {
                f_memSize.Value = (decimal)value.memSize;
                f_debugMemSize.Value = (decimal)value.debugMemSize;
                f_flagReadDebugInfo.Checked = value.flagReadDebugInfo;

                if (value.includePaths == null)
                    f_paths.Text = "";
                else
                    f_paths.Text = String.Join(PATHS_SEP[0], value.includePaths);
                if (value.additionalPATH == null)
                    f_addPATH.Text = "";
                else
                    f_addPATH.Text = String.Join(PATHS_SEP[0], value.additionalPATH);
            }
            get {
                EngineConf x = new EngineConf();
                x.memSize = (int)f_memSize.Value;
                x.debugMemSize = (int)f_debugMemSize.Value;
                x.flagReadDebugInfo = f_flagReadDebugInfo.Checked;
                x.includePaths = f_paths.Text.Split(PATHS_SEP, StringSplitOptions.RemoveEmptyEntries);
                x.additionalPATH = f_addPATH.Text.Split(PATHS_SEP, StringSplitOptions.RemoveEmptyEntries);
                return x;
            }
        }

    }
}
