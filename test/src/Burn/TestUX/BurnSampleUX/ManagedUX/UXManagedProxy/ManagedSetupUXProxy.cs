//-----------------------------------------------------------------------
// <copyright file="ManagedSetupUXProxy.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>Acts a bridgle between native layer and managed UI</summary>
//-----------------------------------------------------------------------

namespace ManagedSetupUX
{
    using System;
    using System.Windows.Forms;
    using System.Threading;
    using System.Runtime.InteropServices;
    using System.Collections.Generic;
    using System.Xml;
    using System.IO;
    using System.Reflection;

    public enum INSTALL_MODE // FROM UXCore.h - keep in sync
    {
        INSTALL_FULL_DISPLAY = 0,
        INSTALL_MIN_DISPLAY,
        UNINSTALL_FULL_DISPLAY,
        UNINSTALL_MIN_DISPLAY,
        REPAIR_FULL_DISPLAY,
        REPAIR_MIN_DISPLAY,
        MODIFY_FULL_DISPLAY // modify requires full display
    }

    public enum BURN_ACTION // FROM IBurnCore.h - keep in sync
    {
        UNKNOWN = 0,
        HELP,
        UNINSTALL,
        INSTALL,
        MODIFY,
        REPAIR,
    };

    public enum PACKAGE_STATE // FROM IBurnCore.h - keep in sync
    {
        UNKNOWN = 0,
        ABSENT,
        CACHED,
        PRESENT,
    }

    public enum REQUEST_STATE // FROM IBurnCore.h - keep in sync
    {
        NONE = 0,
        ABSENT,
        CACHE,
        PRESENT,
        REPAIR,
    }

    public enum ACTION_STATE // FROM IBurnCore.h - keep in sync
    {
        NONE = 0,
        UNINSTALL,
        INSTALL,
        ADMIN_INSTALL,
        MAINTENANCE,
        RECACHE,
        MINOR_UPGRADE,
        MAJOR_UPGRADE,
        PATCH,
    }

    public enum SOURCE_TYPE // FROM IBurnPayload.h - keep in sync
    {
        NONE = 0,
        EXTERNAL,
        EMBEDDED,
        DOWNLOAD,
    }

    public enum HRESULTS
    {
        ERROR_FAILURE = -1,
        S_OK = 0,
        S_FALSE = 1,
    }

    public enum ButtonFlags // FROM WinUser.h
    {
        MB_OK = 0x00000000,
        MB_OKCANCEL = 0x00000001,
        MB_ABORTRETRYIGNORE = 0x00000002,
        MB_YESNOCANCEL = 0x00000003,
        MB_YESNO = 0x00000004,
        MB_RETRYCANCEL = 0x00000005,
    }

    public enum CommandID // FROM WinUser.h
    {
        IDOK = 1,
        IDCANCEL = 2,
        IDABORT = 3,
        IDRETRY = 4,
        IDIGNORE = 5,
        IDYES = 6,
        IDNO = 7
    }

    public enum UIAction
    {
        None = 0x400 + 1,
        SendCancelInstall,
        SendSuspendInstall,
        LoadWelcomePage,
        PopulateItem,
        SendDownloadProgress,
        SendDownloadComplete,
        SendExecutionProgress,
        SendExecutionPackageName,
        SendExecutionComplete,
        SendApplyComplete,
        CloseLaunchWindow,
        Max_Value
    };

    public enum WM_STDUX
    {
        DETECT_PACKAGES = 0x8000 + 1,
        PLAN_PACKAGES,
        APPLY_PACKAGES,
        ELEVATE_PROCESS,
        SUSPEND_INSTALL
    };

    public enum UIPage
    {
        Welcome,
        Eula,
        Progress,
        Finish
    }

    public sealed class NativeMethods
    {
        [return: MarshalAs(UnmanagedType.Bool)]
        [DllImport("user32.dll", SetLastError = true)]
        public static extern bool PostMessage(
            IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);
    }    

    public static class ConstantConversion
    {
        public static ACTION_STATE ActionStateFromInt(int actionState)
        {
            int maxValid = (int)ACTION_STATE.UNINSTALL;

            if (actionState < 0 || actionState > maxValid)
            {
                // don't know what it is, so use NONE
                return ACTION_STATE.NONE;
            }
            else
            {
                return
                    (ACTION_STATE)Enum.ToObject(typeof(ACTION_STATE),
                                                 actionState);
            }
        }

        public static UIAction UIActionFromInt(int uiAction)
        {
            int maxValid = (int)UIAction.Max_Value;

            if (uiAction < 0 || uiAction > maxValid)
             {
                 return UIAction.None;
             }
             else
             {
                 return (UIAction)Enum.ToObject(typeof(UIAction),
                                                      uiAction);
             }
        }

        public static int UIActionToInt(UIAction uiAction)
        {
            return (int)uiAction;
        }

        public static int ActionStateToInt(ACTION_STATE actionState)
        {
            return (int)actionState;
        }

        public static PACKAGE_STATE PackageStateFromInt(int packageState)
        {
            int maxValid = (int)PACKAGE_STATE.PRESENT;

            if (packageState < 0 || packageState > maxValid)
            {
                // don't know what it is, so use UNKNOWN
                return PACKAGE_STATE.UNKNOWN;
            }
            else
            {
                return
                    (PACKAGE_STATE)Enum.ToObject(typeof(PACKAGE_STATE),
                                                 packageState);
            }
        }

        public static int PackageStateToInt(PACKAGE_STATE packageState)
        {
            return (int)packageState;
        }

        public static REQUEST_STATE RequestStateFromInt(int requestState)
        {
            int maxValid = (int)REQUEST_STATE.REPAIR;

            if (requestState < 0 || requestState > maxValid)
            {
                // don't know what it is, so use NONE
                return REQUEST_STATE.NONE;
            }
            else
            {
                return
                    (REQUEST_STATE)Enum.ToObject(typeof(REQUEST_STATE),
                                                 requestState);
            }
        }

        public static int RequestStateToInt(REQUEST_STATE requestState)
        {
            return (int)requestState;
        }

        public static HRESULTS HresultFromInt(int hr)
        {
            HRESULTS result;

            if (hr == 0)
            {
                result = HRESULTS.S_OK;
            }
            else if (hr == 1)
            {
                result = HRESULTS.S_FALSE;
            }
            else if (hr > 1)
            {
                // treat all other non-negative values as S_OK
                result = HRESULTS.S_OK;
            }
            else
            {
                result = HRESULTS.ERROR_FAILURE;
            }

            return result;
        }

        public static int HresultToInt(HRESULTS hr)
        {
            return (int)hr;
        }

        public static SOURCE_TYPE SourceTypeFromInt(int sourceType)
        {
            int maxValid = SourceTypeToInt(SOURCE_TYPE.DOWNLOAD);

            if (sourceType < 0 || sourceType > maxValid)
            {
                // don't know what it is, so use NONE
                return SOURCE_TYPE.NONE;
            }
            else
            {
                return
                    (SOURCE_TYPE)Enum.ToObject(typeof(SOURCE_TYPE),
                                               sourceType);
            }
        }

        public static int SourceTypeToInt(SOURCE_TYPE sourceType)
        {
            return (int)sourceType;
        }
    }

    public static class MsgBoxFlags
    {
        public static ButtonFlags ButtonFlagsFromUInt(uint buttonFlags)
        {
            uint maxValid = (uint)ButtonFlags.MB_RETRYCANCEL;

            if (buttonFlags > maxValid)
            {
                // don't know what it is, so use MB_OK
                return ButtonFlags.MB_OK;
            }
            else
            {
                return
                    (ButtonFlags)Enum.ToObject(typeof(ButtonFlags),
                                               buttonFlags);
            }
        }

        public static MessageBoxButtons MessageBoxButtonsFromUInt(uint buttonFlags)
        {
            ButtonFlags flags = ButtonFlagsFromUInt(buttonFlags);
            if (flags == ButtonFlags.MB_OK)
            {
                return MessageBoxButtons.OK;
            }
            else if (flags == ButtonFlags.MB_OKCANCEL)
            {
                return MessageBoxButtons.OKCancel;
            }
            else if (flags == ButtonFlags.MB_RETRYCANCEL)
            {
                return MessageBoxButtons.RetryCancel;
            }
            else if (flags == ButtonFlags.MB_YESNO)
            {
                return MessageBoxButtons.YesNo;
            }
            else if (flags == ButtonFlags.MB_YESNOCANCEL)
            {
                return MessageBoxButtons.YesNoCancel;
            }
            else if (flags == ButtonFlags.MB_ABORTRETRYIGNORE)
            {
                return MessageBoxButtons.AbortRetryIgnore;
            }
            else
            {
                return MessageBoxButtons.OK;
            }
        }
    }

    public static class ManagedSetupUXProxy
    {
        private class IManagedSetupUXSingleton
        {
            private static volatile IManagedSetupUX m_uiInstance = null;
            private static volatile IntPtr m_controllingHwnd = IntPtr.Zero;
            private static object lockObj = new object();

            private static IManagedSetupUX CreateUXObject(ConstructorInfo setupConstructorInfo, Int32 parendHwnd)
            {
                IManagedSetupUX uxObj;
                if (setupConstructorInfo == null)
                {
                    uxObj = null;
                }
                else
                {
                    try
                    {
                        uxObj = setupConstructorInfo.Invoke(new object[] { parendHwnd }) as IManagedSetupUX;                        
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show(ex.InnerException.Message);
                        uxObj = null;
                    }
                }

                return uxObj;
            }

            private static void Uninitialize()
            {
                if (m_uiInstance != null)
                {
                    try
                    {
                        m_uiInstance.Uninitialize();
                    }
                    catch (Exception)
                    {
                        // Nothing to do at this point... the installation is done anyway (or it should be)
                    }
                    finally
                    {
                        m_uiInstance = null;
                    }
                }

                m_controllingHwnd = IntPtr.Zero;
            }           

            public static bool InitializeInstance(Int32 parentHWND,
                                                    UInt32 mode,
                                                    ConstructorInfo setupConstructorInfo)
            {                
                bool initialized = false;

                if (parentHWND == 0 || setupConstructorInfo == null) // need a valid window handle and ConstructorInfo
                {                    
                    return false;
                }

                lock (lockObj)
                {
                    if (m_uiInstance == null)
                    {
                        m_controllingHwnd = IntPtr.Zero;
                        
                        m_uiInstance = CreateUXObject(setupConstructorInfo, parentHWND);
                       
                        if (m_uiInstance == null)
                        {
                            MessageBox.Show("m_uiInstance == null");
                            initialized = false;
                        }
                        else
                        {
                            //MessageBox.Show("ATTACH DEBUGGER C# - m_uiInstance.Initialize() mode=" + mode.ToString());
                            if (m_uiInstance.Initialize(mode))
                            {
                                try
                                {
                                    m_controllingHwnd = new IntPtr(parentHWND);
                                    initialized = true;                                   
                                }
                                catch
                                {
                                    MessageBox.Show("m_uiInstance.Initialize() failing");
                                    Uninitialize();
                                    initialized = false;
                                }
                            }
                            else
                            {
                                initialized = false;
                            }
                        }
                    }
                    else
                    {                        
                        initialized = true;
                    }
                }
                                
                return initialized;
            }

            public static void UnInitializeInstance()
            {
                lock (lockObj)
                {
                    Uninitialize();
                }
            }

            public static IManagedSetupUX Instance
            {
                get
                {
                    return m_uiInstance;
                }
            }

            public static IntPtr NativeParentHWND
            {
                get { return m_controllingHwnd; }
            }           
        }

        public static IntPtr NativeParentHWND
        {
            get { return IManagedSetupUXSingleton.NativeParentHWND; }
        }

        public static bool Initialize(Int32 parentHwnd,
                                        UInt32 mode,
                                        String workingDir,
                                        String setupAssembly,
                                        String setupClass)
        {
            String fullPath = FullPath(workingDir, setupAssembly);
            ConstructorInfo constructorInfo;
            if (File.Exists(fullPath))
            {
                constructorInfo = GetSetupConstructorInfo(fullPath, setupClass);                
            }
            else
            {
                constructorInfo = null;
            }

            if (constructorInfo == null)
            {
                return false;
            }
            else
            {                
                return IManagedSetupUXSingleton.InitializeInstance(parentHwnd, mode, constructorInfo);
            }
        }

        public static void Uninitialize()
        {
            IManagedSetupUXSingleton.UnInitializeInstance();
        }

        public static bool ShowForm()
        {
            if (IManagedSetupUXSingleton.Instance != null)
            {               
                IManagedSetupUXSingleton.Instance.ShowForm();               
                return true;
            }
            else
            {
                return false;
            }
        }

        public static Int32 OnDetectBegin(UInt32 numPackages)
        {
            //MessageBox.Show("DEBUG C# - OnDetectBegin() numPackages = " + numPackages.ToString());
            Int32 continueInstall = (Int32)CommandID.IDOK;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                //MessageBox.Show("DEBUG C# - OnDetectBegin() Abort");
                continueInstall = (Int32)CommandID.IDABORT;
            }
            else
            {
                //MessageBox.Show("DEBUG C# - Calling Instance.OnDetectBegin()");
                continueInstall =
                    (Int32)IManagedSetupUXSingleton.Instance.OnDetectBegin(numPackages);
            }

            return continueInstall;
        }

        public static Int32 OnDetectPackageBegin(String packageID)
        {
            Int32 continueInstall = (Int32)CommandID.IDOK;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = (Int32)CommandID.IDABORT;
            }
            else
            {
                continueInstall =
                    (Int32)IManagedSetupUXSingleton.Instance.OnDetectPackageBegin(packageID);
            }

            return continueInstall;
        }

        public static bool OnDetectPackageComplete(String packageID,
                                                   Int32 hrStatus,
                                                   Int32 state)
        {
            if (IManagedSetupUXSingleton.Instance == null)
            {
                return false;
            }
            else
            {
                return IManagedSetupUXSingleton.Instance.OnDetectPackageComplete(packageID,
                                                                                 hrStatus,
                                                                                 state);
            }
        }

        public static bool OnDetectComplete(Int32 hrStatus)
        {
            if (IManagedSetupUXSingleton.Instance == null)
            {
                return false;
            }
            else
            {
                return IManagedSetupUXSingleton.Instance.OnDetectComplete(hrStatus);
            }
        }

        public static Int32 OnPlanBegin(UInt32 numPackages)
        {
            Int32 continueInstall = (Int32)CommandID.IDOK;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = (Int32)CommandID.IDABORT;
            }
            else
            {
                continueInstall =
                    (Int32)IManagedSetupUXSingleton.Instance.OnPlanBegin(numPackages);
            }

            return continueInstall;
        }

        public static Int32 OnPlanPackageBegin(ref String packageID)
        {
            Int32 continueInstall = (Int32)CommandID.IDOK;

            packageID = "23222";

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = (Int32)CommandID.IDABORT;
            }
            else
            {
                continueInstall =
                    (Int32)IManagedSetupUXSingleton.Instance.OnPlanPackageBegin(ref packageID);
            }
            packageID = "23222";
            return continueInstall;
        }

        public static bool OnPlanPackageComplete(String packageID,
                                                 Int32 hrStatus,
                                                 Int32 state,
                                                 Int32 requested,
                                                 Int32 execute,
                                                 Int32 rollback)
        {
            if (IManagedSetupUXSingleton.Instance == null)
            {
                return false;
            }
            else
            {
                return IManagedSetupUXSingleton.Instance.OnPlanPackageComplete(packageID,
                                                                               hrStatus,
                                                                               state,
                                                                               requested,
                                                                               execute,
                                                                               rollback);
            }
        }

        public static bool OnPlanComplete(Int32 hrStatus)
        {
            if (IManagedSetupUXSingleton.Instance == null)
            {
                return false;
            }
            else
            {
                return IManagedSetupUXSingleton.Instance.OnPlanComplete(hrStatus);
            }
        }

        public static Int32 OnApplyBegin()
        {
            Int32 continueInstall = (Int32)CommandID.IDOK;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = (Int32)CommandID.IDABORT;
            }
            else
            {
                continueInstall =
                    (Int32)IManagedSetupUXSingleton.Instance.OnApplyBegin();
            }

            return continueInstall;
        }

        public static Int32 OnRegisterBegin()
        {
            Int32 continueInstall = (Int32)CommandID.IDOK;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = (Int32)CommandID.IDABORT;
            }
            else
            {
                continueInstall =
                    (Int32)IManagedSetupUXSingleton.Instance.OnRegisterBegin();
            }

            return continueInstall;
        }

        public static bool OnRegisterComplete(Int32 hrStatus)
        {
            bool continueInstall = false;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = false;
            }
            else
            {
                continueInstall =
                    IManagedSetupUXSingleton.Instance.OnRegisterComplete(hrStatus);
            }

            return continueInstall;
        }

        public static bool OnUnregisterBegin()
        {
            bool continueInstall = false;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = false;
            }
            else
            {
                continueInstall = IManagedSetupUXSingleton.Instance.OnUnregisterBegin();
            }

            return continueInstall;
        }

        public static bool OnUnregisterComplete(Int32 hrStatus)
        {
            bool continueInstall = false;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = false;
            }
            else
            {
                continueInstall =
                    IManagedSetupUXSingleton.Instance.OnUnregisterComplete(hrStatus);
            }

            return continueInstall;
        }

        public static bool OnApplyComplete(Int32 hrStatus)
        {
            bool continueInstall = false;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = false;
            }
            else
            {
                continueInstall =
                    IManagedSetupUXSingleton.Instance.OnApplyComplete(hrStatus);
            }

            return continueInstall;
        }

        public static Int32 OnProgress(Int32 progressPct, Int32 overallProgress)
        {
            Int32 continueInstall = (Int32)CommandID.IDOK;            

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = (Int32)CommandID.IDABORT;
            }
            else
            {
                continueInstall =
                    (Int32)IManagedSetupUXSingleton.Instance.OnProgress(progressPct, overallProgress);
            }

            return continueInstall;
        }

        public static Int32 OnDownloadProgress(Int32 progressPct, Int32 overallProgress)
        {
            Int32 continueInstall = (Int32)CommandID.IDOK;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = (Int32)CommandID.IDABORT;
            }
            else
            {
                continueInstall =
                    (Int32)IManagedSetupUXSingleton.Instance.OnDownloadProgress(progressPct, overallProgress);
            }

            return continueInstall;
        }

        public static Int32 OnExecuteProgress(Int32 progressPct, Int32 overallProgress)
        {
            Int32 continueInstall = (Int32)CommandID.IDOK;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = (Int32)CommandID.IDABORT;
            }
            else
            {
                continueInstall =
                    (Int32)IManagedSetupUXSingleton.Instance.OnExecuteProgress(progressPct, overallProgress);
            }

            return continueInstall;
        }

        public static bool OnCacheComplete(Int32 hrStatus)
        {
            bool continueInstall = false;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = false;
            }
            else
            {
                continueInstall =
                    IManagedSetupUXSingleton.Instance.OnCacheComplete(hrStatus);
            }

            return continueInstall;
        }

        public static Int32 OnExecuteBegin(UInt32 numPackages)
        {
            Int32 continueInstall = (Int32)CommandID.IDOK;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = (Int32)CommandID.IDABORT;
            }
            else
            {
                continueInstall =
                    (Int32)IManagedSetupUXSingleton.Instance.OnExecuteBegin(numPackages);
            }

            return continueInstall;
        }

        public static Int32 OnExecutePackageBegin(String packageID,
                                                 bool installing)
        {
            Int32 continueInstall = (Int32)CommandID.IDOK;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = (Int32)CommandID.IDABORT;
            }
            else
            {
                continueInstall =
                    (Int32)IManagedSetupUXSingleton.Instance.OnExecutePackageBegin(packageID,
                                                                         installing);
            }

            return continueInstall;
        }

        public static bool OnExecutePackageComplete(String packageID,
                                                   Int32 hrStatus)
        {
            bool continueInstall = false;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = false;
            }
            else
            {
                continueInstall =
                    IManagedSetupUXSingleton.Instance.OnExecutePackageComplete(packageID,
                                                                            hrStatus);
            }

            return continueInstall;
        }

        public static bool OnExecuteComplete(Int32 hrStatus)
        {
            bool continueInstall = false;

            if (IManagedSetupUXSingleton.Instance == null)
            {
                continueInstall = false;
            }
            else
            {
                continueInstall =
                    IManagedSetupUXSingleton.Instance.OnExecuteComplete(hrStatus);
            }

            return continueInstall;
        }

        public static Int32 OnError(UInt32 code,
                                   String errMsg,
                                   UInt32 mbflag)
        {
            if (IManagedSetupUXSingleton.Instance != null)
            {
                return IManagedSetupUXSingleton.Instance.OnError(code, errMsg, mbflag);
            }
            else
            {
                return (Int32)DialogResult.Abort;
            }
        }

        private static ConstructorInfo GetSetupConstructorInfo(string fullPathToAssembly, string setupClass)
        {
            ConstructorInfo setupConstructorInfo = null;
            try
            {                
                Assembly assem = Assembly.LoadFile(fullPathToAssembly);
                
                if (assem == null)
                {
                    return null;
                }

                Type setupType = assem.GetType(setupClass);
                
                if (setupType == null)
                {
                    return null;
                }
                Type[] types = new Type[1];
                types[0] = typeof(Int32);                
                setupConstructorInfo = setupType.GetConstructor(types);                
            }
            catch (Exception)
            {
                setupConstructorInfo = null;
            }

            return setupConstructorInfo;
        }

        private static String FullPath(string workingDir, string fileName)
        {
            String fullPath;

            if (Path.DirectorySeparatorChar == workingDir[workingDir.Length - 1])
            {
                fullPath = String.Concat(workingDir, fileName);
            }
            else
            {
                fullPath = String.Concat(workingDir, Path.DirectorySeparatorChar, fileName);
            }

            return fullPath;
        }
    }
}