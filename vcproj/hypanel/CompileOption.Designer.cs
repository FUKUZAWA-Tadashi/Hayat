namespace hypanel
{
    partial class CompileOption
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.f_compilerPath = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.compilerPathSelectButton = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.memSize = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.inputCharset = new System.Windows.Forms.ComboBox();
            this.label6 = new System.Windows.Forms.Label();
            this.outputCharset = new System.Windows.Forms.ComboBox();
            this.label7 = new System.Windows.Forms.Label();
            this.f_paths = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.wLabelLevel = new System.Windows.Forms.ComboBox();
            this.wLabelScope = new System.Windows.Forms.ComboBox();
            this.label9 = new System.Windows.Forms.Label();
            this.wNumsayLevel = new System.Windows.Forms.ComboBox();
            this.wNumsayLimit = new System.Windows.Forms.NumericUpDown();
            this.label10 = new System.Windows.Forms.Label();
            this.wUnknownLevel = new System.Windows.Forms.ComboBox();
            this.verbose = new System.Windows.Forms.CheckBox();
            this.printSyntaxTree = new System.Windows.Forms.CheckBox();
            this.label11 = new System.Windows.Forms.Label();
            this.debugPrintParse = new System.Windows.Forms.ComboBox();
            this.cancelButton = new System.Windows.Forms.Button();
            this.okButton = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.memSize)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.wNumsayLimit)).BeginInit();
            this.SuspendLayout();
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            this.openFileDialog1.Filter = "実行ファイル|*.exe|すべてのファイル|*.*";
            this.openFileDialog1.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog1_FileOk);
            // 
            // f_compilerPath
            // 
            this.f_compilerPath.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.f_compilerPath.Location = new System.Drawing.Point(12, 24);
            this.f_compilerPath.Name = "f_compilerPath";
            this.f_compilerPath.Size = new System.Drawing.Size(281, 19);
            this.f_compilerPath.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(10, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(49, 12);
            this.label1.TabIndex = 1;
            this.label1.Text = "コンパイラ";
            // 
            // compilerPathSelectButton
            // 
            this.compilerPathSelectButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.compilerPathSelectButton.Location = new System.Drawing.Point(299, 22);
            this.compilerPathSelectButton.Name = "compilerPathSelectButton";
            this.compilerPathSelectButton.Size = new System.Drawing.Size(50, 23);
            this.compilerPathSelectButton.TabIndex = 2;
            this.compilerPathSelectButton.Text = "選択";
            this.compilerPathSelectButton.UseVisualStyleBackColor = true;
            this.compilerPathSelectButton.Click += new System.EventHandler(this.compilerPathSelectButton_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(10, 55);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(53, 12);
            this.label2.TabIndex = 3;
            this.label2.Text = "使用メモリ";
            // 
            // memSize
            // 
            this.memSize.Location = new System.Drawing.Point(70, 55);
            this.memSize.Maximum = new decimal(new int[] {
            1024,
            0,
            0,
            0});
            this.memSize.Minimum = new decimal(new int[] {
            4,
            0,
            0,
            0});
            this.memSize.Name = "memSize";
            this.memSize.Size = new System.Drawing.Size(54, 19);
            this.memSize.TabIndex = 4;
            this.memSize.Value = new decimal(new int[] {
            16,
            0,
            0,
            0});
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(130, 57);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(25, 12);
            this.label3.TabIndex = 5;
            this.label3.Text = "MiB";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(13, 85);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(82, 12);
            this.label4.TabIndex = 6;
            this.label4.Text = "キャラクターセット";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(114, 85);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(29, 12);
            this.label5.TabIndex = 7;
            this.label5.Text = "入力";
            // 
            // inputCharset
            // 
            this.inputCharset.FormattingEnabled = true;
            this.inputCharset.Items.AddRange(new object[] {
            "UTF-8",
            "SJIS",
            "EUCJP"});
            this.inputCharset.Location = new System.Drawing.Point(149, 82);
            this.inputCharset.Name = "inputCharset";
            this.inputCharset.Size = new System.Drawing.Size(76, 20);
            this.inputCharset.TabIndex = 8;
            this.inputCharset.Text = "SJIS";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(241, 85);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(29, 12);
            this.label6.TabIndex = 9;
            this.label6.Text = "出力";
            // 
            // outputCharset
            // 
            this.outputCharset.FormattingEnabled = true;
            this.outputCharset.Items.AddRange(new object[] {
            "UTF-8",
            "SJIS",
            "EUCJP"});
            this.outputCharset.Location = new System.Drawing.Point(276, 82);
            this.outputCharset.Name = "outputCharset";
            this.outputCharset.Size = new System.Drawing.Size(73, 20);
            this.outputCharset.TabIndex = 10;
            this.outputCharset.Text = "SJIS";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(12, 111);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(60, 12);
            this.label7.TabIndex = 11;
            this.label7.Text = "includeパス";
            // 
            // f_paths
            // 
            this.f_paths.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.f_paths.Location = new System.Drawing.Point(13, 126);
            this.f_paths.Multiline = true;
            this.f_paths.Name = "f_paths";
            this.f_paths.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.f_paths.Size = new System.Drawing.Size(336, 83);
            this.f_paths.TabIndex = 12;
            this.f_paths.WordWrap = false;
            // 
            // label8
            // 
            this.label8.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(12, 227);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(98, 12);
            this.label8.TabIndex = 13;
            this.label8.Text = "飛び先ラベルチェック";
            // 
            // wLabelLevel
            // 
            this.wLabelLevel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.wLabelLevel.FormattingEnabled = true;
            this.wLabelLevel.Items.AddRange(new object[] {
            "チェックしない",
            "警告",
            "エラー"});
            this.wLabelLevel.Location = new System.Drawing.Point(116, 224);
            this.wLabelLevel.Name = "wLabelLevel";
            this.wLabelLevel.Size = new System.Drawing.Size(98, 20);
            this.wLabelLevel.TabIndex = 14;
            this.wLabelLevel.Text = "警告";
            // 
            // wLabelScope
            // 
            this.wLabelScope.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.wLabelScope.FormattingEnabled = true;
            this.wLabelScope.Items.AddRange(new object[] {
            "ファイル内チェック",
            "ブロック内チェック"});
            this.wLabelScope.Location = new System.Drawing.Point(220, 224);
            this.wLabelScope.Name = "wLabelScope";
            this.wLabelScope.Size = new System.Drawing.Size(130, 20);
            this.wLabelScope.TabIndex = 15;
            this.wLabelScope.Text = "ファイル内チェック";
            // 
            // label9
            // 
            this.label9.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(13, 258);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(97, 12);
            this.label9.TabIndex = 16;
            this.label9.Text = "sayCommand個数";
            // 
            // wNumsayLevel
            // 
            this.wNumsayLevel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.wNumsayLevel.FormattingEnabled = true;
            this.wNumsayLevel.Items.AddRange(new object[] {
            "チェックしない",
            "警告",
            "エラー"});
            this.wNumsayLevel.Location = new System.Drawing.Point(116, 255);
            this.wNumsayLevel.Name = "wNumsayLevel";
            this.wNumsayLevel.Size = new System.Drawing.Size(98, 20);
            this.wNumsayLevel.TabIndex = 17;
            this.wNumsayLevel.Text = "チェックしない";
            // 
            // wNumsayLimit
            // 
            this.wNumsayLimit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.wNumsayLimit.Location = new System.Drawing.Point(220, 256);
            this.wNumsayLimit.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.wNumsayLimit.Name = "wNumsayLimit";
            this.wNumsayLimit.Size = new System.Drawing.Size(89, 19);
            this.wNumsayLimit.TabIndex = 18;
            // 
            // label10
            // 
            this.label10.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(13, 287);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(65, 12);
            this.label10.TabIndex = 19;
            this.label10.Text = "不明識別子";
            // 
            // wUnknownLevel
            // 
            this.wUnknownLevel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.wUnknownLevel.FormattingEnabled = true;
            this.wUnknownLevel.Items.AddRange(new object[] {
            "チェックしない",
            "警告",
            "エラー"});
            this.wUnknownLevel.Location = new System.Drawing.Point(116, 284);
            this.wUnknownLevel.Name = "wUnknownLevel";
            this.wUnknownLevel.Size = new System.Drawing.Size(98, 20);
            this.wUnknownLevel.TabIndex = 20;
            this.wUnknownLevel.Text = "警告";
            // 
            // verbose
            // 
            this.verbose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.verbose.AutoSize = true;
            this.verbose.Location = new System.Drawing.Point(12, 316);
            this.verbose.Name = "verbose";
            this.verbose.Size = new System.Drawing.Size(64, 16);
            this.verbose.TabIndex = 21;
            this.verbose.Text = "verbose";
            this.verbose.UseVisualStyleBackColor = true;
            // 
            // printSyntaxTree
            // 
            this.printSyntaxTree.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.printSyntaxTree.AutoSize = true;
            this.printSyntaxTree.Location = new System.Drawing.Point(167, 316);
            this.printSyntaxTree.Name = "printSyntaxTree";
            this.printSyntaxTree.Size = new System.Drawing.Size(109, 16);
            this.printSyntaxTree.TabIndex = 22;
            this.printSyntaxTree.Text = "print syntax tree";
            this.printSyntaxTree.UseVisualStyleBackColor = true;
            // 
            // label11
            // 
            this.label11.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(12, 346);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(130, 12);
            this.label11.TabIndex = 23;
            this.label11.Text = "デバッグ用パース結果表示";
            // 
            // debugPrintParse
            // 
            this.debugPrintParse.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.debugPrintParse.FormattingEnabled = true;
            this.debugPrintParse.Items.AddRange(new object[] {
            "表示しない",
            "表示",
            "詳細表示"});
            this.debugPrintParse.Location = new System.Drawing.Point(148, 343);
            this.debugPrintParse.Name = "debugPrintParse";
            this.debugPrintParse.Size = new System.Drawing.Size(121, 20);
            this.debugPrintParse.TabIndex = 24;
            this.debugPrintParse.Text = "しない";
            // 
            // cancelButton
            // 
            this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cancelButton.Location = new System.Drawing.Point(274, 376);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 25;
            this.cancelButton.Text = "キャンセル";
            this.cancelButton.UseVisualStyleBackColor = true;
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // okButton
            // 
            this.okButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.okButton.Location = new System.Drawing.Point(193, 376);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(75, 23);
            this.okButton.TabIndex = 26;
            this.okButton.Text = "OK";
            this.okButton.UseVisualStyleBackColor = true;
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // CompileOption
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(361, 411);
            this.Controls.Add(this.okButton);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.debugPrintParse);
            this.Controls.Add(this.label11);
            this.Controls.Add(this.printSyntaxTree);
            this.Controls.Add(this.verbose);
            this.Controls.Add(this.wUnknownLevel);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.wNumsayLimit);
            this.Controls.Add(this.wNumsayLevel);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.wLabelScope);
            this.Controls.Add(this.wLabelLevel);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.f_paths);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.outputCharset);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.inputCharset);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.memSize);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.compilerPathSelectButton);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.f_compilerPath);
            this.MinimumSize = new System.Drawing.Size(369, 410);
            this.Name = "CompileOption";
            this.Text = "CompileOption";
            this.Load += new System.EventHandler(this.CompileOption_Load);
            ((System.ComponentModel.ISupportInitialize)(this.memSize)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.wNumsayLimit)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.TextBox f_compilerPath;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button compilerPathSelectButton;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown memSize;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ComboBox inputCharset;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ComboBox outputCharset;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TextBox f_paths;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.ComboBox wLabelLevel;
        private System.Windows.Forms.ComboBox wLabelScope;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.ComboBox wNumsayLevel;
        private System.Windows.Forms.NumericUpDown wNumsayLimit;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.ComboBox wUnknownLevel;
        private System.Windows.Forms.CheckBox verbose;
        private System.Windows.Forms.CheckBox printSyntaxTree;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.ComboBox debugPrintParse;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Button okButton;
    }
}