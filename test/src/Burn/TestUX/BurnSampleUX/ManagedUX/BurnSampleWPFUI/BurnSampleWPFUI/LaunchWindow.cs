//-----------------------------------------------------------------------
// <copyright file="LaunchWindow.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>winform window to launch wpf ui</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using ManagedSetupUX;
using System.Windows.Threading;
using System.Threading;
using System.Xml;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters.Binary;
using System.IO;

namespace BurnSampleWPFUI
{
    public partial class LaunchWindow : Form, IManagedSetupUX
    {
        public const uint WM_CLOSE = 0x10;

        private System.Object lockThis = new System.Object();
        private IntPtr wpfHandle;

        private bool continueInstall = true;
        private bool suspendedInstall = false;
        private bool installingPackages = false;
        private bool userInitiated = false;
        private INSTALL_MODE installMode = INSTALL_MODE.INSTALL_FULL_DISPLAY;
        private BURN_ACTION burnAction = BURN_ACTION.INSTALL;
        bool cachedComplete = false;
        private BurnSetup wpfWindow;
        private static volatile IntPtr m_controllingHwnd = IntPtr.Zero;

        private static EventWaitHandle ewhInitializeMethodCalled;  //Event raised when Initialize() method has been called.

        public LaunchWindow(Int32 parentHwnd)
        {
            //Set Manual since we need it to remains signaled.
            ewhInitializeMethodCalled = new EventWaitHandle(false, EventResetMode.ManualReset);
            //MessageBox.Show("DEBUG C# - LaunchWindow " + parentHwnd);
            InitializeComponent();
            m_controllingHwnd = new IntPtr(parentHwnd);

            // Don't show the little windows forms window but make sure the Load event occurs
            Show();
            Hide();
        }

        public bool ContinueInstall
        {
            get
            {
                return this.continueInstall;
            }

            private set
            {
                lock (lockThis)
                {
                    this.continueInstall = value;
                }
            }
        }

        public bool Initialize(UInt32 mode)
        {
            bool success = true;
            this.installMode = (INSTALL_MODE)mode;
            ewhInitializeMethodCalled.Set();
            return success;
        }

        public void Uninitialize()
        {
            Shutdown(false);
        }

        public void ShowForm()
        {
            DisplayUX();
        }

        public CommandID OnDetectBegin(UInt32 numPackages)
        {
            while (wpfHandle.ToInt32() == 0)
            {
                wpfHandle = System.Diagnostics.Process.GetCurrentProcess().MainWindowHandle;
                System.Threading.Thread.Sleep(10);
            }

            CommandID result = CommandID.IDABORT;
            try
            {
                // Post load welcome page message
                NativeMethods.PostMessage(wpfHandle, (uint)UIAction.LoadWelcomePage, IntPtr.Zero, IntPtr.Zero);

                if (ContinueInstall == true)
                {
                    result = CommandID.IDOK;
                }
                else
                {
                    result = CommandID.IDABORT;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

            return result;
        }

        public CommandID OnDetectPackageBegin(String packageID)
        {
            CommandID result = CommandID.IDABORT;

            if (ContinueInstall)
            {
                result = CommandID.IDOK;
            }
            else
            {
                result = CommandID.IDABORT;
            }

            return result;
        }

        public bool OnDetectPackageComplete(String packageID, Int32 hrStatus,Int32 state)
        {
            if (ManagedSetupUX.ConstantConversion.HresultFromInt(hrStatus) == HRESULTS.S_OK)
            {
                NativeMethods.PostMessage(wpfHandle, (uint)UIAction.PopulateItem, Marshal.StringToHGlobalAnsi(packageID), (IntPtr)state);
                return ContinueInstall;
            }
            return false;
        }

        public bool OnDetectComplete(Int32 hrStatus)
        {
            return ContinueInstall;
        }

        private void DisplayUX()
        {
            DisplayForm();
        }

        public CommandID OnPlanBegin(UInt32 numPackages)
        {
            CommandID result = CommandID.IDABORT;

            if (ContinueInstall)
            {
                result = CommandID.IDOK;
            }
            else
            {
                result = CommandID.IDABORT;
            }

            return result;
        }

        public CommandID OnPlanPackageBegin(ref String packageID)
        {
            CommandID result;

            if (ContinueInstall)
            {
                result = CommandID.IDOK;
            }
            else
            {
                result = CommandID.IDABORT;
            }
            packageID = "2222";
            return result;
        }

        public bool OnPlanPackageComplete(String packageID, Int32 hrStatus, Int32 state, Int32 requested, Int32 execute, Int32 rollback)
        {
            return ContinueInstall;
        }

        public bool OnPlanComplete(Int32 hrStatus)
        {
            return ContinueInstall;
        }

        public CommandID OnApplyBegin()
        {
            CommandID result;

            if (ContinueInstall)
            {
                result = CommandID.IDOK;
            }
            else
            {
                result = CommandID.IDABORT;
            }

            return result;
        }

        public CommandID OnRegisterBegin()
        {
            CommandID result;

            if (ContinueInstall)
            {
                result = CommandID.IDOK;
            }
            else
            {
                result = CommandID.IDABORT;
            }

            return result;
        }

        public bool OnRegisterComplete(Int32 hrStatus)
        {
            return ContinueInstall;
        }

        public bool OnUnregisterBegin()
        {
            return ContinueInstall;
        }

        public bool OnUnregisterComplete(Int32 hrStatus)
        {
            return ContinueInstall;
        }

        public bool OnApplyComplete(Int32 hrStatus)
        {
            bool completedSuccessfully;
            const int PROGRESS_COMPLETE = 100;
            const int PROGRESS_INCOMPLETE = 0;
            Int32 pctComplete = 0;

            if (HRESULTS.S_OK == ConstantConversion.HresultFromInt(hrStatus))
            {
                pctComplete = PROGRESS_COMPLETE;
                completedSuccessfully = true;                
            }
            else if (!ContinueInstall)
            {
                pctComplete = PROGRESS_INCOMPLETE;
                completedSuccessfully = true;
            }
            else
            {
                pctComplete = PROGRESS_INCOMPLETE;
                completedSuccessfully = false;                
            }
            
            // Send Apply complete message.
            int suspended = 0;
            if (this.suspendedInstall)
            {
                suspended = 1;
            }
            NativeMethods.PostMessage(wpfHandle, (uint)UIAction.SendApplyComplete, (IntPtr)hrStatus, (IntPtr)suspended);

            return completedSuccessfully;
        }

        public CommandID OnProgress(Int32 progressPct, Int32 overallProgress)
        {
            CommandID result;

            if (ContinueInstall)
            {
                result = CommandID.IDOK;
            }
            else
            {
                result = CommandID.IDABORT;
            }

            return result;
        }

        public CommandID OnDownloadProgress(Int32 progressPct, Int32 overallProgress)
        {
            CommandID result;

            if (ContinueInstall)
            {

                // Send download progress initially
                NativeMethods.PostMessage(wpfHandle, (uint)UIAction.SendDownloadProgress, (IntPtr)progressPct, (IntPtr)overallProgress);
                result = CommandID.IDOK;
            }
            else
            {
                result = CommandID.IDABORT;
            }

            return result;
        }

        public CommandID OnExecuteProgress(Int32 progressPct, Int32 overallProgress)
        {
            CommandID result;

            if (ContinueInstall)
            {

                // Send install progress after download is complete
                NativeMethods.PostMessage(wpfHandle, (uint)UIAction.SendExecutionProgress, (IntPtr)progressPct, (IntPtr)overallProgress);
                result = CommandID.IDOK;
            }
            else
            {
                result = CommandID.IDABORT;
            }

            return result;
        }

        public bool OnCacheComplete(Int32 hrStatus)
        {
            if (HRESULTS.S_OK == ConstantConversion.HresultFromInt(hrStatus))
            {
                cachedComplete = true;
            }

            NativeMethods.PostMessage(wpfHandle, (uint)UIAction.SendDownloadComplete, IntPtr.Zero, IntPtr.Zero);

            return ContinueInstall;
        }

        public CommandID OnExecuteBegin(UInt32 numPackages)
        {
            CommandID result;

            if (ContinueInstall)
            {
                result = CommandID.IDOK;
            }
            else
            {
                result = CommandID.IDABORT;
            }

            return result;
        }

        public CommandID OnExecutePackageBegin(String packageID, bool installing)
        {
            CommandID result;
           
            if (ContinueInstall)
            {
                result = CommandID.IDOK;
            }
            else
            {
                result = CommandID.IDABORT;
            }
            
            // Send execution package name
            NativeMethods.PostMessage(wpfHandle, (uint)UIAction.SendExecutionPackageName, Marshal.StringToHGlobalAnsi(packageID), IntPtr.Zero);

            return result;
        }

        public bool OnExecutePackageComplete(String packageID, Int32 hrStatus)
        {
            return ContinueInstall;
        }

        public bool OnExecuteComplete(Int32 hrStatus)
        {
            NativeMethods.PostMessage(wpfHandle, (uint)UIAction.SendExecutionComplete, IntPtr.Zero, IntPtr.Zero);
            return ContinueInstall;
        }

        public Int32 OnError(UInt32 code, String errMsg, UInt32 mbflag)
        {
            return 0;
        }

        private void DisplayForm()
        {
            try
            {
                this.ShowDialog();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void CreateUIInAnotherThread(object obj)
        {
            // Need to wait for Initialize() method to be called first,
            // as this.installMode is initialized there.
            ewhInitializeMethodCalled.WaitOne();

            wpfWindow = new BurnSetup(m_controllingHwnd, (IntPtr)obj, this.installMode);
            wpfWindow.Show();

            try
            {
                System.Windows.Threading.Dispatcher.Run();
            }
            catch
            {
                // null reference exception might occur as the UI closes.  Not sure why...
            }
        }

        private void Shutdown(bool userInitiated)
        {
            CloseForm(userInitiated);
        }

        private void CloseForm(bool userInitiated)
        {
            this.userInitiated = userInitiated;
            this.Close();
            NativeMethods.PostMessage(LaunchWindow.m_controllingHwnd, WM_CLOSE, IntPtr.Zero, IntPtr.Zero);
        }

        [System.Security.Permissions.PermissionSet(System.Security.Permissions.SecurityAction.Demand, Name = "FullTrust")]
        protected override void WndProc(ref Message m)
        {            
            base.WndProc(ref m);
            ManagedSetupUX.UIAction uiAction = ManagedSetupUX.ConstantConversion.UIActionFromInt(m.Msg);
            if (uiAction == UIAction.SendSuspendInstall)
            {
                NativeMethods.PostMessage(LaunchWindow.m_controllingHwnd, (uint)ManagedSetupUX.WM_STDUX.SUSPEND_INSTALL, IntPtr.Zero, IntPtr.Zero);
                this.suspendedInstall = true;
                this.ContinueInstall = false;
            }
            if (uiAction == UIAction.SendCancelInstall)
            {
                this.ContinueInstall = false;                
            }
            else if (uiAction == UIAction.CloseLaunchWindow)
            {
                CloseForm(true);
            }
        }

        private void LaunchWindow_Load(object sender, EventArgs e)
        {
            try
            {
                Thread thread = new Thread(new ParameterizedThreadStart(CreateUIInAnotherThread));
                thread.SetApartmentState(ApartmentState.STA);
                thread.Start(this.Handle);

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        // Hide this window from the alt-tab menu
        protected override CreateParams CreateParams
        {
            get
            {
                // Turn on WS_EX_TOOLWINDOW style bit
                CreateParams cp = base.CreateParams;
                cp.ExStyle |= 0x80;
                return cp;
            }
        }       
    }
}
