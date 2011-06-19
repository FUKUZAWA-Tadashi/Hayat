using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace hypanel
{
    public class PanelConf {
        public string   scriptPath;
        public bool     flagInitOnReload;
    }

    public class CompilerConf {
        public string   compilerPath;
        public int      memSize;
        public string   inputCharset;
        public string   outputCharset;
        public string[] includePaths;
        public int      warnLabelLevel;
        public int      warnLabelScope;
        public int      warnNumsayLevel;
        public int      warnNumsayLimit;
        public int      warnUnknownLevel;
        public bool     flagVerbose;
        public bool     flagPrintSyntaxTree;
        public int      debugPrintParse;

        public CompilerConf()
        {
            compilerPath = null;
            memSize = 16;
            inputCharset = "SJIS";
            outputCharset = "SJIS";
            includePaths = null;
            warnLabelScope = 0;
            warnLabelLevel = 1;
            warnNumsayLimit = 0;
            warnNumsayLevel = 0;
            warnUnknownLevel = 1;
            flagVerbose = false;
            flagPrintSyntaxTree = false;
            debugPrintParse = 0;
        }

        public string getCompileArgument()
        {
            ArrayList args = new ArrayList();
            args.Add("-M" + memSize.ToString() + "M");
            args.Add("-ci" + inputCharset);
            args.Add("-co" + outputCharset);
            
            foreach (string p in includePaths) {
                args.Add("-I" + p);
            }
            
            string s;
            
            s = warnLabelScope.ToString();
            switch (warnLabelLevel) {
            case 0:
                s = "0"; break;
            case 2:
                s += "!"; break;
            }
            args.Add("-Wlabel=" + s);
            
            s = warnNumsayLimit.ToString();
            switch (warnNumsayLevel) {
            case 0:
                s = "0"; break;
            case 2:
                s += "!"; break;
            }
            args.Add("-Wnumsay=" + s);

            s = warnUnknownLevel.ToString();
            args.Add("-Wunknown=" + s);

            if (flagVerbose)
                args.Add("-v");
            if (flagPrintSyntaxTree)
                args.Add("-s");

            switch (debugPrintParse) {
            case 1:
                args.Add("-d"); break;
            case 2:
                args.Add("-d2"); break;
            }


            StringBuilder sb = new StringBuilder();
            foreach (string a in args) {
                if (sb.Length > 0)
                    sb.Append(" ");
                sb.Append(a);
            }

            return sb.ToString();
        }
    }

    public class EngineConf {
        public int      memSize;
        public int      debugMemSize;
        public bool     flagReadDebugInfo;
        public string[] includePaths;   // Hayatのパス
        public string[] additionalPATH; // 環境変数PATHに追加するパス

        public EngineConf()
        {
            memSize = 16;
            debugMemSize = 16;
            flagReadDebugInfo = true;
            includePaths = null;
            additionalPATH = null;
        }
    }

    public class Confs {
        public PanelConf    conf_panel;
        public CompilerConf conf_compiler;
        public EngineConf   conf_engine;
    };

}
