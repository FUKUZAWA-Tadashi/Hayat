using System;
using System.Windows.Forms;
using System.IO;
using System.Text;

namespace hypanel
{
    // TextBoxに対してストリーム出力する。
    // TextBoxからの入力には未対応。
    // 文字数オーバーは未検証
    public class TextBoxStream : Stream
    {
        protected TextBox   m_textBox;
        protected Encoding  m_encoding;
        public delegate void AddTextDelegate(string str);
        private AddTextDelegate addTextDele;

        public TextBoxStream(TextBox textBox, Encoding enc)
        {
            m_textBox = textBox;
            m_encoding = enc;
            addTextDele = new AddTextDelegate(addText);
        }

        public void addText(string str)
        {
            lock (m_textBox) {
                m_textBox.AppendText(str);
            }
        }


        public override void Write(byte[] buffer, int offset, int count)
        {
            string str = m_encoding.GetString(buffer, offset, count);
            if (m_textBox.InvokeRequired) {
                m_textBox.Invoke(addTextDele, new Object[] { str });
            } else {
                addText(str);
            }
        }

        public override bool CanRead { get { return false; } }
        public override bool CanSeek { get { return false; } }
        public override bool CanTimeout { get { return false; } }
        public override bool CanWrite { get { return true; } }
        public override long Length { get { return m_textBox.Text.Length; } }
        public override long Position {
            get { return m_textBox.Text.Length; }
            set { throw new System.NotImplementedException(); }
        }
        public override void Flush()
        {
        }
        public override long Seek(long offset, System.IO.SeekOrigin origin)
        {
            throw new System.NotImplementedException();
        }
        public override void SetLength(long len)
        {
            throw new System.NotImplementedException();
        }
        public override int Read(byte[] buffer, int offset, int count)
        {
            throw new System.NotImplementedException();
        }
    }

}
