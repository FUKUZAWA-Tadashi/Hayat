namespace hypanel
{
    partial class HyPanel
    {
        /// <summary>
        /// 必要なデザイナ変数です。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 使用中のリソースをすべてクリーンアップします。
        /// </summary>
        /// <param name="disposing">マネージ リソースが破棄される場合 true、破棄されない場合は false です。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows フォーム デザイナで生成されたコード

        /// <summary>
        /// デザイナ サポートに必要なメソッドです。このメソッドの内容を
        /// コード エディタで変更しないでください。
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(HyPanel));
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.ファイルFToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ロードLToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.セーブSToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.開くOToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.終了XToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.操作EToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.コンパイルCToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.実行EToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.リロードRToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ツールTToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.コンパイルオプションToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.実行オプションToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.label1 = new System.Windows.Forms.Label();
            this.hayatFileName = new System.Windows.Forms.TextBox();
            this.selectFileButton = new System.Windows.Forms.Button();
            this.compileButton = new System.Windows.Forms.Button();
            this.execButton = new System.Windows.Forms.Button();
            this.reloadButton = new System.Windows.Forms.Button();
            this.logBox = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.openScriptDialog = new System.Windows.Forms.OpenFileDialog();
            this.logClearButton = new System.Windows.Forms.Button();
            this.compileOptButton = new System.Windows.Forms.Button();
            this.engineOptButton = new System.Windows.Forms.Button();
            this.checkBox_initOnReload = new System.Windows.Forms.CheckBox();
            this.openConfDialog = new System.Windows.Forms.OpenFileDialog();
            this.saveConfDialog = new System.Windows.Forms.SaveFileDialog();
            this.cleanButton = new System.Windows.Forms.Button();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ファイルFToolStripMenuItem,
            this.操作EToolStripMenuItem,
            this.ツールTToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(343, 26);
            this.menuStrip1.TabIndex = 0;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // ファイルFToolStripMenuItem
            // 
            this.ファイルFToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ロードLToolStripMenuItem,
            this.セーブSToolStripMenuItem,
            this.toolStripSeparator1,
            this.開くOToolStripMenuItem,
            this.終了XToolStripMenuItem});
            this.ファイルFToolStripMenuItem.Name = "ファイルFToolStripMenuItem";
            this.ファイルFToolStripMenuItem.Size = new System.Drawing.Size(85, 22);
            this.ファイルFToolStripMenuItem.Text = "ファイル(&F)";
            // 
            // ロードLToolStripMenuItem
            // 
            this.ロードLToolStripMenuItem.Name = "ロードLToolStripMenuItem";
            this.ロードLToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Alt | System.Windows.Forms.Keys.L)));
            this.ロードLToolStripMenuItem.Size = new System.Drawing.Size(239, 22);
            this.ロードLToolStripMenuItem.Text = "ロード(&L)";
            this.ロードLToolStripMenuItem.Click += new System.EventHandler(this.ロードLToolStripMenuItem_Click);
            // 
            // セーブSToolStripMenuItem
            // 
            this.セーブSToolStripMenuItem.Name = "セーブSToolStripMenuItem";
            this.セーブSToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Alt | System.Windows.Forms.Keys.S)));
            this.セーブSToolStripMenuItem.Size = new System.Drawing.Size(239, 22);
            this.セーブSToolStripMenuItem.Text = "セーブ(&S)";
            this.セーブSToolStripMenuItem.Click += new System.EventHandler(this.セーブSToolStripMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(236, 6);
            // 
            // 開くOToolStripMenuItem
            // 
            this.開くOToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("開くOToolStripMenuItem.Image")));
            this.開くOToolStripMenuItem.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.開くOToolStripMenuItem.Name = "開くOToolStripMenuItem";
            this.開くOToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
            this.開くOToolStripMenuItem.Size = new System.Drawing.Size(239, 22);
            this.開くOToolStripMenuItem.Text = "スクリプトを開く(&O)";
            this.開くOToolStripMenuItem.Click += new System.EventHandler(this.開くOToolStripMenuItem_Click);
            // 
            // 終了XToolStripMenuItem
            // 
            this.終了XToolStripMenuItem.Name = "終了XToolStripMenuItem";
            this.終了XToolStripMenuItem.Size = new System.Drawing.Size(239, 22);
            this.終了XToolStripMenuItem.Text = "終了(&X)";
            this.終了XToolStripMenuItem.Click += new System.EventHandler(this.終了XToolStripMenuItem_Click);
            // 
            // 操作EToolStripMenuItem
            // 
            this.操作EToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.コンパイルCToolStripMenuItem,
            this.toolStripSeparator3,
            this.実行EToolStripMenuItem,
            this.リロードRToolStripMenuItem});
            this.操作EToolStripMenuItem.Name = "操作EToolStripMenuItem";
            this.操作EToolStripMenuItem.Size = new System.Drawing.Size(63, 22);
            this.操作EToolStripMenuItem.Text = "操作(&G)";
            // 
            // コンパイルCToolStripMenuItem
            // 
            this.コンパイルCToolStripMenuItem.Name = "コンパイルCToolStripMenuItem";
            this.コンパイルCToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.コンパイルCToolStripMenuItem.Text = "コンパイル(&C)";
            this.コンパイルCToolStripMenuItem.Click += new System.EventHandler(this.compileButton_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(151, 6);
            // 
            // 実行EToolStripMenuItem
            // 
            this.実行EToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("実行EToolStripMenuItem.Image")));
            this.実行EToolStripMenuItem.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.実行EToolStripMenuItem.Name = "実行EToolStripMenuItem";
            this.実行EToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.実行EToolStripMenuItem.Text = "実行(&E)";
            this.実行EToolStripMenuItem.Click += new System.EventHandler(this.execButton_Click);
            // 
            // リロードRToolStripMenuItem
            // 
            this.リロードRToolStripMenuItem.Enabled = false;
            this.リロードRToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("リロードRToolStripMenuItem.Image")));
            this.リロードRToolStripMenuItem.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.リロードRToolStripMenuItem.Name = "リロードRToolStripMenuItem";
            this.リロードRToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.リロードRToolStripMenuItem.Text = "リロード(&R)";
            // 
            // ツールTToolStripMenuItem
            // 
            this.ツールTToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.コンパイルオプションToolStripMenuItem,
            this.実行オプションToolStripMenuItem});
            this.ツールTToolStripMenuItem.Name = "ツールTToolStripMenuItem";
            this.ツールTToolStripMenuItem.Size = new System.Drawing.Size(74, 22);
            this.ツールTToolStripMenuItem.Text = "ツール(&T)";
            // 
            // コンパイルオプションToolStripMenuItem
            // 
            this.コンパイルオプションToolStripMenuItem.Name = "コンパイルオプションToolStripMenuItem";
            this.コンパイルオプションToolStripMenuItem.Size = new System.Drawing.Size(196, 22);
            this.コンパイルオプションToolStripMenuItem.Text = "コンパイルオプション";
            this.コンパイルオプションToolStripMenuItem.Click += new System.EventHandler(this.compileOptButton_Click);
            // 
            // 実行オプションToolStripMenuItem
            // 
            this.実行オプションToolStripMenuItem.Name = "実行オプションToolStripMenuItem";
            this.実行オプションToolStripMenuItem.Size = new System.Drawing.Size(196, 22);
            this.実行オプションToolStripMenuItem.Text = "実行オプション";
            this.実行オプションToolStripMenuItem.Click += new System.EventHandler(this.engineOptButton_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(10, 31);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(76, 12);
            this.label1.TabIndex = 1;
            this.label1.Text = "Hayatスクリプト";
            // 
            // hayatFileName
            // 
            this.hayatFileName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.hayatFileName.Location = new System.Drawing.Point(12, 46);
            this.hayatFileName.Name = "hayatFileName";
            this.hayatFileName.Size = new System.Drawing.Size(253, 19);
            this.hayatFileName.TabIndex = 2;
            // 
            // selectFileButton
            // 
            this.selectFileButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.selectFileButton.Location = new System.Drawing.Point(271, 44);
            this.selectFileButton.Name = "selectFileButton";
            this.selectFileButton.Size = new System.Drawing.Size(62, 23);
            this.selectFileButton.TabIndex = 3;
            this.selectFileButton.Text = "開く(&O)";
            this.selectFileButton.UseVisualStyleBackColor = true;
            this.selectFileButton.Click += new System.EventHandler(this.selectFileButton_Click);
            // 
            // compileButton
            // 
            this.compileButton.Location = new System.Drawing.Point(12, 73);
            this.compileButton.Name = "compileButton";
            this.compileButton.Size = new System.Drawing.Size(75, 23);
            this.compileButton.TabIndex = 4;
            this.compileButton.Text = "コンパイル(&C)";
            this.compileButton.UseVisualStyleBackColor = true;
            this.compileButton.Click += new System.EventHandler(this.compileButton_Click);
            // 
            // execButton
            // 
            this.execButton.Location = new System.Drawing.Point(12, 102);
            this.execButton.Name = "execButton";
            this.execButton.Size = new System.Drawing.Size(75, 23);
            this.execButton.TabIndex = 5;
            this.execButton.Text = "実行(&E)";
            this.execButton.UseVisualStyleBackColor = true;
            this.execButton.Click += new System.EventHandler(this.execButton_Click);
            // 
            // reloadButton
            // 
            this.reloadButton.Enabled = false;
            this.reloadButton.Location = new System.Drawing.Point(12, 131);
            this.reloadButton.Name = "reloadButton";
            this.reloadButton.Size = new System.Drawing.Size(75, 23);
            this.reloadButton.TabIndex = 6;
            this.reloadButton.Text = "リロード(&R)";
            this.reloadButton.UseVisualStyleBackColor = true;
            this.reloadButton.Click += new System.EventHandler(this.reloadButton_Click);
            // 
            // logBox
            // 
            this.logBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.logBox.Location = new System.Drawing.Point(13, 186);
            this.logBox.Multiline = true;
            this.logBox.Name = "logBox";
            this.logBox.ReadOnly = true;
            this.logBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.logBox.Size = new System.Drawing.Size(321, 105);
            this.logBox.TabIndex = 7;
            this.logBox.TextChanged += new System.EventHandler(this.logBox_TextChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(39, 168);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(23, 12);
            this.label2.TabIndex = 8;
            this.label2.Text = "ログ";
            // 
            // openScriptDialog
            // 
            this.openScriptDialog.DefaultExt = "hy";
            this.openScriptDialog.Filter = "Hayatファイル|*.hy|すべてのファイル|*.*";
            this.openScriptDialog.Title = "Hayatスクリプトファイル";
            this.openScriptDialog.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog1_FileOk);
            // 
            // logClearButton
            // 
            this.logClearButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.logClearButton.Location = new System.Drawing.Point(248, 157);
            this.logClearButton.Name = "logClearButton";
            this.logClearButton.Size = new System.Drawing.Size(83, 23);
            this.logClearButton.TabIndex = 11;
            this.logClearButton.Text = "ログクリア";
            this.logClearButton.UseVisualStyleBackColor = true;
            this.logClearButton.Click += new System.EventHandler(this.logClearButton_Click);
            // 
            // compileOptButton
            // 
            this.compileOptButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.compileOptButton.Location = new System.Drawing.Point(215, 73);
            this.compileOptButton.Name = "compileOptButton";
            this.compileOptButton.Size = new System.Drawing.Size(116, 23);
            this.compileOptButton.TabIndex = 12;
            this.compileOptButton.Text = "コンパイルオプション";
            this.compileOptButton.UseVisualStyleBackColor = true;
            this.compileOptButton.Click += new System.EventHandler(this.compileOptButton_Click);
            // 
            // engineOptButton
            // 
            this.engineOptButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.engineOptButton.Location = new System.Drawing.Point(215, 102);
            this.engineOptButton.Name = "engineOptButton";
            this.engineOptButton.Size = new System.Drawing.Size(117, 23);
            this.engineOptButton.TabIndex = 13;
            this.engineOptButton.Text = "実行オプション";
            this.engineOptButton.UseVisualStyleBackColor = true;
            this.engineOptButton.Click += new System.EventHandler(this.engineOptButton_Click);
            // 
            // checkBox_initOnReload
            // 
            this.checkBox_initOnReload.AutoSize = true;
            this.checkBox_initOnReload.Location = new System.Drawing.Point(94, 137);
            this.checkBox_initOnReload.Name = "checkBox_initOnReload";
            this.checkBox_initOnReload.Size = new System.Drawing.Size(131, 16);
            this.checkBox_initOnReload.TabIndex = 14;
            this.checkBox_initOnReload.Text = "リロード時初期化実行";
            this.checkBox_initOnReload.UseVisualStyleBackColor = true;
            // 
            // openConfDialog
            // 
            this.openConfDialog.DefaultExt = "xml";
            this.openConfDialog.Filter = "XMLファイル|*.xml|すべてのファイル|*.*";
            this.openConfDialog.Title = "環境ロード";
            this.openConfDialog.FileOk += new System.ComponentModel.CancelEventHandler(this.openConfDialog_FileOk);
            // 
            // saveConfDialog
            // 
            this.saveConfDialog.DefaultExt = "xml";
            this.saveConfDialog.Filter = "XMLファイル|*.xml|すべてのファイル|*.*";
            this.saveConfDialog.Title = "環境セーブ";
            this.saveConfDialog.FileOk += new System.ComponentModel.CancelEventHandler(this.saveConfDialog_FileOk);
            // 
            // cleanButton
            // 
            this.cleanButton.Location = new System.Drawing.Point(94, 73);
            this.cleanButton.Name = "cleanButton";
            this.cleanButton.Size = new System.Drawing.Size(53, 23);
            this.cleanButton.TabIndex = 15;
            this.cleanButton.Text = "クリーン";
            this.cleanButton.UseVisualStyleBackColor = true;
            this.cleanButton.Click += new System.EventHandler(this.cleanButton_Click);
            // 
            // HyPanel
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(343, 303);
            this.Controls.Add(this.cleanButton);
            this.Controls.Add(this.checkBox_initOnReload);
            this.Controls.Add(this.engineOptButton);
            this.Controls.Add(this.compileOptButton);
            this.Controls.Add(this.logClearButton);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.logBox);
            this.Controls.Add(this.reloadButton);
            this.Controls.Add(this.execButton);
            this.Controls.Add(this.compileButton);
            this.Controls.Add(this.selectFileButton);
            this.Controls.Add(this.hayatFileName);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.MinimumSize = new System.Drawing.Size(236, 258);
            this.Name = "HyPanel";
            this.Text = "HyPanel";
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox hayatFileName;
        private System.Windows.Forms.Button selectFileButton;
        private System.Windows.Forms.Button compileButton;
        private System.Windows.Forms.Button execButton;
        private System.Windows.Forms.Button reloadButton;
        private System.Windows.Forms.TextBox logBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ToolStripMenuItem ファイルFToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 開くOToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 終了XToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 操作EToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem コンパイルCToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripMenuItem 実行EToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem リロードRToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ツールTToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 実行オプションToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem コンパイルオプションToolStripMenuItem;
        private System.Windows.Forms.OpenFileDialog openScriptDialog;
        private System.Windows.Forms.Button logClearButton;
        private System.Windows.Forms.Button compileOptButton;
        private System.Windows.Forms.Button engineOptButton;
        private System.Windows.Forms.CheckBox checkBox_initOnReload;
        private System.Windows.Forms.ToolStripMenuItem ロードLToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem セーブSToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.OpenFileDialog openConfDialog;
        private System.Windows.Forms.SaveFileDialog saveConfDialog;
        private System.Windows.Forms.Button cleanButton;
    }
}

