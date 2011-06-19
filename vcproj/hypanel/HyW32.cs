using System;
using System.IO;
using System.Text;
using System.Runtime.InteropServices;
using System.ComponentModel;
using Microsoft.Win32.SafeHandles;

namespace hypanel
{

    public class HyW32
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct SECURITY_ATTRIBUTES {
            public uint     nLength;
            public IntPtr   lpSecurityDescriptor;
            public bool     bInheritHandle;
        }
        public const uint STD_INPUT_HANDLE = unchecked((uint)-10);
        public const uint STD_OUTPUT_HANDLE = unchecked((uint)-11);
        public const uint STD_ERROR_HANDLE = unchecked((uint)-12);

        public const uint DUPLICATE_CLOSE_SOURCE = 1;
        public const uint DUPLICATE_SAME_ACCESS = 2;


        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool CreatePipe(out IntPtr readPipe, out IntPtr writePipe, ref SECURITY_ATTRIBUTES attr, uint nSize);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr GetStdHandle(uint stdHandle);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern void SetStdHandle(uint stdHandle, IntPtr handle);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool DuplicateHandle(IntPtr hSourceProcessHandle,  // 複製元プロセスのハンドル
                             IntPtr hSourceHandle,         // 複製対象のハンドル
                             IntPtr hTargetProcessHandle,  // 複製先プロセスのハンドル
                             out IntPtr lpTargetHandle,      // 複製先ハンドル
                             uint dwDesiredAccess,        // 複製ハンドルで必要なアクセス権
                             bool bInheritHandle,          // ハンドル継承フラグ
                             uint dwOptions               // オプションの動作
                             );

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr GetCurrentProcess();

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool CloseHandle(IntPtr handle);



        protected static SafeFileHandle getPipedStdHandle(uint stdHandle) {
            SECURITY_ATTRIBUTES security_attributes1 = new SECURITY_ATTRIBUTES();
            security_attributes1.nLength = 0;
            security_attributes1.bInheritHandle = true;

            IntPtr oldHandle;
            IntPtr hStdOutReadPipe = IntPtr.Zero;
            IntPtr hStdOutWritePipe = IntPtr.Zero;
            IntPtr hDupStdOutReadPipe = IntPtr.Zero;

            try {
                if (!CreatePipe(out hStdOutReadPipe, out hStdOutWritePipe,
                                ref security_attributes1, 0))
                {
                    int errNo = Marshal.GetLastWin32Error();
                    throw new Win32Exception(errNo);
                }

                oldHandle = GetStdHandle(stdHandle);
                SetStdHandle(stdHandle, hStdOutWritePipe);

                if (!DuplicateHandle(GetCurrentProcess(),
                                     hStdOutReadPipe,
                                     GetCurrentProcess(),
                                     out hDupStdOutReadPipe,
                                     0, false, DUPLICATE_SAME_ACCESS))
                {
                    int errNo = Marshal.GetLastWin32Error();
                    throw new Win32Exception(errNo);
                }
            }
            finally {
                if (hStdOutReadPipe != IntPtr.Zero) {
                    CloseHandle(hStdOutReadPipe);
                }
            }

            SafeFileHandle handle = new SafeFileHandle(hDupStdOutReadPipe, true);
            if (handle.IsInvalid) {
                Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());
            }
            return handle;
        }




        //==================================================
        // 標準出力/標準エラー出力をConsoleにリダイレクトする
        //==================================================

        private struct FsReadState {
            public byte[] buf;
            public int size;
            public FileStream stream;
            public int id;
        }

        private static void fsReadCallback(IAsyncResult asyncResult)
        {
            FsReadState stat = (FsReadState)asyncResult.AsyncState;
            int numRead = stat.stream.EndRead(asyncResult);
            
            Encoding enc = Encoding.GetEncoding("Shift_JIS");
            string str = enc.GetString(stat.buf, 0, numRead);
            Console.Write(str);
            
            AsyncCallback readCallback = new AsyncCallback(fsReadCallback);
            stat.stream.BeginRead(stat.buf, 0, stat.size, readCallback, stat);
        }

        private static void fsAsyncRead(SafeFileHandle handle, int id)
        {
            byte[] buf = new byte[4096];
            FileStream fs = new FileStream(handle, FileAccess.Read);
            FsReadState stat = new FsReadState();
            AsyncCallback readCallback = new AsyncCallback(fsReadCallback);
            stat.buf = buf;
            stat.size = 4096;
            stat.stream = fs;
            stat.id = id;
            fs.BeginRead(buf, 0, 4096, readCallback, stat);
        }

        public static void redirectStdOutputs()
        {
            SafeFileHandle stdoutHandle = getPipedStdHandle(STD_OUTPUT_HANDLE);
            SafeFileHandle stderrHandle = getPipedStdHandle(STD_ERROR_HANDLE);

            fsAsyncRead(stdoutHandle, 0);
            fsAsyncRead(stderrHandle, 1);
        }            

    }
}
