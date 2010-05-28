//-----------------------------------------------------------------------
// <copyright file="BurnSetup.xaml.cs" company="Microsoft">
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
// <summary>wpf main window. It acts as a container for different setup pages like welcome, eula, progress etc</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using ManagedSetupUX;
using System.Windows.Interop;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters.Binary;
using System.Collections.Specialized;
using System.IO;
using System.Xml;

namespace BurnSampleWPFUI
{
    public struct StructPackageInfo
    {
        public string packageId;
        public string packageName;
        public PACKAGE_STATE packageState;
    }

    /// <summary>
    /// Interaction logic for BurnSetup.xaml
    /// </summary>
    public partial class BurnSetup : Window
    {
        private WelcomePage m_WelcomePage;
        private EulaPage m_EulaPage;
        private ProgressPage m_ProgressPage;
        private FinishPage m_FinishPage;
        private ItemPlanSummary m_ItemPlanSummaryPage;
        private MaintenanceModePage m_MaintenanceModePage;
        private static volatile IntPtr m_ControllingHwnd = IntPtr.Zero;
        private static volatile IntPtr m_LaunchWndHwnd = IntPtr.Zero;
        private XmlDocument m_XmlDocItemMapping;
        private bool cancelled;
        public INSTALL_MODE m_Mode;
        public string m_ItemPlanFilePath;
        public string m_PackageName;
        public List<string> m_ItemSummaryList;

        public ListDictionary m_packageInfo;

        public const int WM_NCLBUTTONDOWN = 0xA1;
        public const int HT_CAPTION = 0x2;

        [DllImportAttribute("user32.dll")]
        public static extern int SendMessage(IntPtr hWnd, int Msg, int wParam, int lParam);
        [DllImportAttribute("user32.dll")]
        public static extern bool ReleaseCapture();

        public void move_window(object sender, MouseButtonEventArgs e)
        {
            ReleaseCapture();
            SendMessage(new WindowInteropHelper(this).Handle, WM_NCLBUTTONDOWN, HT_CAPTION, 0);
        }

        public BurnSetup(IntPtr controllingHwnd, IntPtr launchWindowHandle, INSTALL_MODE mode)
        {
            //MessageBox.Show("DEBUG C# - BurnSetup() launchWindowHandle=  " + launchWindowHandle.ToInt32() + " mode=" + mode.ToString());
            InitializeComponent();

            m_WelcomePage = new WelcomePage(this);
            m_EulaPage = new EulaPage(this);
            m_ProgressPage = new ProgressPage(this);
            m_FinishPage = new FinishPage(this);
            m_ItemPlanSummaryPage = new ItemPlanSummary(this);
            m_MaintenanceModePage = new MaintenanceModePage(this);
           
            m_ControllingHwnd = controllingHwnd;
            m_LaunchWndHwnd = launchWindowHandle;

            m_Mode = mode;

            m_packageInfo = new ListDictionary();

            CreateItemPlanXMLFile();
        }

        private void CreateItemPlanXMLFile()
        {
            try
            {
                m_ItemPlanFilePath = System.IO.Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "ItemPlan.xml");

                if (File.Exists(m_ItemPlanFilePath))
                    File.Delete(m_ItemPlanFilePath);

                XmlDocument xmlDoc = new XmlDocument();
                XmlNode parentNode = xmlDoc.CreateNode(XmlNodeType.Element, "ItemPlan", "");
                xmlDoc.AppendChild(parentNode);

                // Load PI.xml           
                string parameterInfoFilePath = new Uri(System.Reflection.Assembly.GetAssembly(typeof(BurnSetup)).CodeBase).LocalPath;
                parameterInfoFilePath = System.IO.Path.GetDirectoryName(parameterInfoFilePath);
                parameterInfoFilePath = System.IO.Path.Combine(parameterInfoFilePath, "ParameterInfo.xml");
               
                XmlDocument xmlDocPI = new XmlDocument();
                xmlDocPI.Load(parameterInfoFilePath);

                XmlNamespaceManager nameSpaceManager;
                nameSpaceManager = new XmlNamespaceManager(new NameTable());
                nameSpaceManager.AddNamespace("im", @"http://schemas.microsoft.com/Setup/2008/01/im");

                // Get the package name
                XmlNode uiNode = xmlDocPI.SelectSingleNode("//im:UI", nameSpaceManager);
                m_PackageName = uiNode.Attributes["Name"].Value;

                // Get item child nodes
                XmlNodeList itemNodes = xmlDocPI.SelectSingleNode("//im:Items", nameSpaceManager).ChildNodes;               
                foreach (XmlNode itemNode in itemNodes)
                {
                    // Read "Id" attribute
                    string itemIdValue = itemNode.Attributes["Id"].Value;
                    XmlNode itemPlanNode = xmlDoc.CreateNode(XmlNodeType.Element, "Item", "");

                    XmlAttribute itemIdAttribute = xmlDoc.CreateAttribute("Id");
                    itemIdAttribute.Value = itemIdValue;

                    XmlAttribute itemRequestStateAttribute = xmlDoc.CreateAttribute("RequestState");
                    itemRequestStateAttribute.Value = "";

                    itemPlanNode.Attributes.Append(itemIdAttribute);
                    itemPlanNode.Attributes.Append(itemRequestStateAttribute);

                    parentNode.AppendChild(itemPlanNode);
                }
                
                xmlDoc.Save(m_ItemPlanFilePath);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        public void UpdateItemPlan(string packageId, REQUEST_STATE requestState)
        {
            try
            {
                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load(m_ItemPlanFilePath);

                XmlNode node = xmlDoc.SelectSingleNode(string.Format("./ItemPlan/Item[@Id='{0}']", packageId));
                node.Attributes["RequestState"].Value = requestState.ToString();

                xmlDoc.Save(m_ItemPlanFilePath);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        public void RemoveRequestStateAttribute()
        {
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.Load(m_ItemPlanFilePath);

            XmlNodeList itemNodes = xmlDoc.SelectNodes("//Item");

            foreach (XmlNode node in itemNodes)
            {
                XmlAttribute attr = node.Attributes["RequestState"];

                if (attr != null)
                {
                    node.Attributes.Remove(attr);
                }
            }
        }

        // BUGBUG this is never called, delete it.  (leaving it for now as a reference, just in case...)
        //// Welcome page will be loaded when Detect of package begins
        //public CommandID OnDetectBegin()
        //{
        //    // Load Welcome page
        //    return LoadWelcomePage();
        //}

        private CommandID LoadInitialPage()
        {
            try
            {
                gridMain.Children.Clear();
                if (m_Mode == INSTALL_MODE.INSTALL_FULL_DISPLAY)
                {
                    LoadEulaPage();
                }
                else if (m_Mode == INSTALL_MODE.REPAIR_FULL_DISPLAY || m_Mode == INSTALL_MODE.UNINSTALL_FULL_DISPLAY)
                {
                    LoadMMPage();
                }
                else if (m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY ||
                    m_Mode == INSTALL_MODE.REPAIR_MIN_DISPLAY ||
                    m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY)
                {
                    LoadEulaPage();
                }
                
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
                return CommandID.IDABORT;
            }
            return CommandID.IDOK;
        }

        public CommandID LoadWelcomePage()
        {
            try
            {   
                gridMain.Children.Clear();                
                gridMain.Children.Add(m_WelcomePage);
              
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
                return CommandID.IDABORT;
            }

            if (this.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY ||
                this.m_Mode == INSTALL_MODE.REPAIR_MIN_DISPLAY ||
                this.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY)
            {
                m_WelcomePage.Proceed();
            }

            return CommandID.IDOK;
        }

        public CommandID LoadMMPage()
        {
            try
            {
                gridMain.Children.Clear();
                gridMain.Children.Add(m_MaintenanceModePage);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
                return CommandID.IDABORT;
            }
            return CommandID.IDOK;
        }

        public CommandID LoadItemPlanSummaryPage()
        {
            try
            {
                m_ItemPlanSummaryPage = new ItemPlanSummary(this);
                gridMain.Children.Clear();
                gridMain.Children.Add(m_ItemPlanSummaryPage);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
                return CommandID.IDABORT;
            }

            if (this.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY ||
                this.m_Mode == INSTALL_MODE.REPAIR_MIN_DISPLAY ||
                this.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY)
            {
                m_ItemPlanSummaryPage.Proceed();
            }

            return CommandID.IDOK;
        }

        public bool OnPlanPackageComplete(string packageName, Int32 hrStatus, PACKAGE_STATE state, REQUEST_STATE requested
            , ACTION_STATE execute, ACTION_STATE rollback)
        {
            bool result = false;



            return result;
        }

        // Load the Eula page after user clicks Next button in Welcome page
        public void LoadEulaPage()
        {
            gridMain.Children.Clear();
            gridMain.Children.Add(m_EulaPage);
            m_EulaPage.LoadEulaText();

            if (this.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY ||
                this.m_Mode == INSTALL_MODE.REPAIR_MIN_DISPLAY ||
                this.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY)
            {
                m_EulaPage.Proceed();
            }
        }      

        // Load the progres page after user accepts the EULA
        public void LoadProgressPage()
        {
            gridMain.Children.Clear();
            gridMain.Children.Add(m_ProgressPage);

            if (this.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY ||
                this.m_Mode == INSTALL_MODE.REPAIR_MIN_DISPLAY ||
                this.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY)
            {
                m_ProgressPage.Proceed();
            }
        }

        // Update the download progress 
        public CommandID OnDownloadProgress(int progressPercentage, int overallProgressPercentage)
        {
            CommandID result = CommandID.IDABORT;

            return result;
        }

        // Update the installation progress. After CacheComplete, progressbar would show the installation progress
        public CommandID OnInstallationProgress(int progressPercentage, int overallProgressPercentage)
        {
            CommandID result = CommandID.IDABORT;

            return result;
        }

        // Load Finish page after onApplyComplete event is fired
        public void LoadFinishPage()
        {
            gridMain.Children.Clear();
            gridMain.Children.Add(m_FinishPage);
        }

        public IntPtr GetWindowHandle()
        {
            try
            {
                IntPtr windowHandle = new WindowInteropHelper(this).Handle;                
                return windowHandle;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
            return IntPtr.Zero;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            WindowInteropHelper winHelper = new WindowInteropHelper(this);
            try 
            {
                HwndSource hwnd = HwndSource.FromHwnd(winHelper.Handle);
                hwnd.AddHook(new HwndSourceHook(this.MessageProc));
            }
            catch (Exception ex) 
            { 
                MessageBox.Show(ex.Message);
            }
        }

        public bool PostPlanMessage()
        {
            IntPtr installerAction = IntPtr.Zero;

            if (m_Mode == INSTALL_MODE.INSTALL_FULL_DISPLAY || m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY)
            {
                installerAction = (IntPtr)BURN_ACTION.INSTALL;
            }
            else if (m_Mode == INSTALL_MODE.UNINSTALL_FULL_DISPLAY || m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY)
            {
                installerAction = (IntPtr)BURN_ACTION.UNINSTALL;
            }
            else if (m_Mode == INSTALL_MODE.REPAIR_FULL_DISPLAY || m_Mode == INSTALL_MODE.REPAIR_MIN_DISPLAY)
            {
                installerAction = (IntPtr)BURN_ACTION.REPAIR;
            }

            if (ManagedSetupUXProxy.NativeParentHWND == IntPtr.Zero)
            {
                return false;
            }
            else
            {
                return NativeMethods.PostMessage(m_ControllingHwnd, (uint)ManagedSetupUX.WM_STDUX.PLAN_PACKAGES, installerAction, IntPtr.Zero);
            }
        }

        public bool PostApplyMessage()
        {
            if (ManagedSetupUXProxy.NativeParentHWND == IntPtr.Zero)
            {
                return false;
            }
            else
            {
                return NativeMethods.PostMessage(m_ControllingHwnd, (uint)ManagedSetupUX.WM_STDUX.APPLY_PACKAGES, GetWindowHandle(), IntPtr.Zero);
            }
        }

        public string[] GetPackageName_IconSrc(string packageId)
        {
            try
            {
                string[] packageName_IconSrc = new string[2];
                m_XmlDocItemMapping = new XmlDocument();

                string itemMappingFilePath = new Uri(System.Reflection.Assembly.GetAssembly(typeof(BurnSetup)).CodeBase).LocalPath;
                itemMappingFilePath = System.IO.Path.GetDirectoryName(itemMappingFilePath);
                itemMappingFilePath = System.IO.Path.Combine(itemMappingFilePath, "1033");
                itemMappingFilePath = System.IO.Path.Combine(itemMappingFilePath, "ItemMapping.xml");

                if (File.Exists(itemMappingFilePath))
                {

                    m_XmlDocItemMapping.Load(itemMappingFilePath);

                    if (m_XmlDocItemMapping != null)
                    {
                        XmlNode node = m_XmlDocItemMapping.SelectSingleNode(string.Format("./Items/Item[@Id='{0}']", packageId));
                        if (node != null)
                        {
                            packageName_IconSrc[0] = node.Attributes["Name"].Value;
                            packageName_IconSrc[1] = node.Attributes["Icon"].Value;
                            return packageName_IconSrc;
                        }
                    }
                }

                packageName_IconSrc[0] = packageId;
                packageName_IconSrc[1] = string.Empty;

                return packageName_IconSrc;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
                return null;
            }
        }

        public void ShowCancelWindow(ManagedSetupUX.UIPage uiPage)
        {
            CancelWindow cancelWindow = new CancelWindow(this);
            if (cancelWindow.DoModal() == true)
            {
                // Post Cancel message to launch window
                PostCancelMessage();

                if (uiPage == UIPage.Welcome || uiPage == UIPage.Eula)
                {
                    PostCloseMessage();
                    this.Close();
                }
            }           
        }

        public void ShowSuspendWindow(ManagedSetupUX.UIPage uiPage)
        {
            SuspendWindow suspendWindow = new SuspendWindow(this);
            if (suspendWindow.DoModal() == true)
            {
                // Post Suspend message to launch window
                PostSuspendMessage();

                if (uiPage == UIPage.Welcome || uiPage == UIPage.Eula)
                {
                    PostCloseMessage();
                    this.Close();
                }
            }
        }

        public void PostSuspendMessage()
        {
            NativeMethods.PostMessage(m_LaunchWndHwnd, (uint)UIAction.SendSuspendInstall, IntPtr.Zero, IntPtr.Zero);
        }

        public void PostCancelMessage()
        {
            this.cancelled = true;
            NativeMethods.PostMessage(m_LaunchWndHwnd, (uint)UIAction.SendCancelInstall, IntPtr.Zero, IntPtr.Zero);
        }

        public void PostCloseMessage()
        {
            NativeMethods.PostMessage(m_LaunchWndHwnd, (uint)UIAction.CloseLaunchWindow, IntPtr.Zero, IntPtr.Zero);
        }

        public void PostActionMessage()
        {
            PostPlanMessage();
            PostApplyMessage();
            LoadProgressPage();
        }

        private IntPtr MessageProc(IntPtr hwnd, int nMsgID, IntPtr wParam, IntPtr lParam, ref bool bHandled) 
        {
            ManagedSetupUX.UIAction uiAction = ManagedSetupUX.ConstantConversion.UIActionFromInt(nMsgID);

            switch (uiAction)
            {
                case UIAction.LoadWelcomePage:
                    LoadInitialPage();
                    break;

                case UIAction.PopulateItem:
                    // Add item list in welcome page
                    string packageId = Marshal.PtrToStringAnsi(wParam);                  
                    string[] packageName_IconSrc = GetPackageName_IconSrc(packageId);

                    StructPackageInfo packageInfo = new StructPackageInfo();
                    packageInfo.packageId = packageId;
                    packageInfo.packageName = packageName_IconSrc[0];
                    packageInfo.packageState = ManagedSetupUX.ConstantConversion.PackageStateFromInt(lParam.ToInt32());

                    m_packageInfo.Add(packageId, packageInfo);                   
                    break;

                case UIAction.SendDownloadProgress:
                    // Update download progress
                    m_ProgressPage.UpdateDownloadProgress(wParam.ToInt32(), lParam.ToInt32()); 
                    break;

                case UIAction.SendDownloadComplete:
                    // Update download complete message
                    m_ProgressPage.DownloadComplete();
                    break;

                case UIAction.SendExecutionPackageName:
                    // Update current execution package name
                    m_ProgressPage.UpdateExectionPackageName(GetPackageName_IconSrc(Marshal.PtrToStringAnsi(wParam))[0]);
                    break;

                case UIAction.SendExecutionProgress:
                    // Update execution progress
                    m_ProgressPage.UpdateExecutionProgress(wParam.ToInt32(), lParam.ToInt32());
                    break;

                case UIAction.SendExecutionComplete:                    
                    // Update execution complete message
                    m_ProgressPage.ExecutionComplete();                   
                    break;

                case UIAction.SendApplyComplete:
                   // Load Finish page and pass the return code so that Finish page can show appropiate message (Success or Failure)
                    LoadFinishPage();
                    m_FinishPage.UpdateFinishMessage(wParam.ToInt32(), lParam.ToInt32(), this.cancelled);

                    if (this.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY ||
                        this.m_Mode == INSTALL_MODE.REPAIR_MIN_DISPLAY ||
                        this.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY)
                    {
                        m_FinishPage.Proceed();
                    }
                    // BUGBUG where do we handle the reboot now/later dlg?

                    break;
            }

            return IntPtr.Zero;
        }
    }
}
