namespace hypanel
{
    partial class EngineOption
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
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.f_memSize = new System.Windows.Forms.NumericUpDown();
            this.f_debugMemSize = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.f_flagReadDebugInfo = new System.Windows.Forms.CheckBox();
            this.label5 = new System.Windows.Forms.Label();
            this.f_paths = new System.Windows.Forms.TextBox();
            this.okButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.f_addPATH = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.f_memSize)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.f_debugMemSize)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 15);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(53, 12);
            this.label1.TabIndex = 0;
            this.label1.Text = "メモリ容量";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(13, 40);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(65, 12);
            this.label2.TabIndex = 1;
            this.label2.Text = "デバッグメモリ";
            // 
            // f_memSize
            // 
            this.f_memSize.Location = new System.Drawing.Point(84, 12);
            this.f_memSize.Maximum = new decimal(new int[] {
            1024,
            0,
            0,
            0});
            this.f_memSize.Minimum = new decimal(new int[] {
            4,
            0,
            0,
            0});
            this.f_memSize.Name = "f_memSize";
            this.f_memSize.Size = new System.Drawing.Size(57, 19);
            this.f_memSize.TabIndex = 2;
            this.f_memSize.Value = new decimal(new int[] {
            16,
            0,
            0,
            0});
            // 
            // f_debugMemSize
            // 
            this.f_debugMemSize.Location = new System.Drawing.Point(84, 38);
            this.f_debugMemSize.Maximum = new decimal(new int[] {
            1024,
            0,
            0,
            0});
            this.f_debugMemSize.Name = "f_debugMemSize";
            this.f_debugMemSize.Size = new System.Drawing.Size(57, 19);
            this.f_debugMemSize.TabIndex = 3;
            this.f_debugMemSize.Value = new decimal(new int[] {
            16,
            0,
            0,
            0});
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(147, 15);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(25, 12);
            this.label3.TabIndex = 4;
            this.label3.Text = "MiB";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(147, 40);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(25, 12);
            this.label4.TabIndex = 5;
            this.label4.Text = "MiB";
            // 
            // f_flagReadDebugInfo
            // 
            this.f_flagReadDebugInfo.AutoSize = true;
            this.f_flagReadDebugInfo.Checked = true;
            this.f_flagReadDebugInfo.CheckState = System.Windows.Forms.CheckState.Checked;
            this.f_flagReadDebugInfo.Location = new System.Drawing.Point(15, 63);
            this.f_flagReadDebugInfo.Name = "f_flagReadDebugInfo";
            this.f_flagReadDebugInfo.Size = new System.Drawing.Size(108, 16);
            this.f_flagReadDebugInfo.TabIndex = 6;
            this.f_flagReadDebugInfo.Text = "デバッグ情報読込";
            this.f_flagReadDebugInfo.UseVisualStyleBackColor = true;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(14, 86);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(54, 12);
            this.label5.TabIndex = 7;
            this.label5.Text = "Hayatパス";
            // 
            // f_paths
            // 
            this.f_paths.AcceptsReturn = true;
            this.f_paths.AllowDrop = true;
            this.f_paths.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.f_paths.Location = new System.Drawing.Point(16, 102);
            this.f_paths.Multiline = true;
            this.f_paths.Name = "f_paths";
            this.f_paths.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.f_paths.Size = new System.Drawing.Size(232, 67);
            this.f_paths.TabIndex = 8;
            this.f_paths.WordWrap = false;
            // 
            // okButton
            // 
            this.okButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.okButton.Location = new System.Drawing.Point(92, 257);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(75, 23);
            this.okButton.TabIndex = 9;
            this.okButton.Text = "OK";
            this.okButton.UseVisualStyleBackColor = true;
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cancelButton.Location = new System.Drawing.Point(173, 257);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 10;
            this.cancelButton.Text = "キャンセル";
            this.cancelButton.UseVisualStyleBackColor = true;
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // f_addPATH
            // 
            this.f_addPATH.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.f_addPATH.Location = new System.Drawing.Point(16, 204);
            this.f_addPATH.Multiline = true;
            this.f_addPATH.Name = "f_addPATH";
            this.f_addPATH.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.f_addPATH.Size = new System.Drawing.Size(232, 47);
            this.f_addPATH.TabIndex = 11;
            // 
            // label6
            // 
            this.label6.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(14, 189);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(154, 12);
            this.label6.TabIndex = 12;
            this.label6.Text = "環境変数PATHに追加するパス";
            // 
            // EngineOption
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(260, 292);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.f_addPATH);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.okButton);
            this.Controls.Add(this.f_paths);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.f_flagReadDebugInfo);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.f_debugMemSize);
            this.Controls.Add(this.f_memSize);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.MinimumSize = new System.Drawing.Size(190, 220);
            this.Name = "EngineOption";
            this.Text = "実行オプション";
            ((System.ComponentModel.ISupportInitialize)(this.f_memSize)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.f_debugMemSize)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown f_memSize;
        private System.Windows.Forms.NumericUpDown f_debugMemSize;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.CheckBox f_flagReadDebugInfo;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox f_paths;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.TextBox f_addPATH;
        private System.Windows.Forms.Label label6;
    }
}