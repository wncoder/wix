//-----------------------------------------------------------------------
// <copyright file="ProgressPage.xmal.cs" company="Microsoft">
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
// <summary>to display download and install progress</summary>
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
using System.Windows.Navigation;
using System.Windows.Shapes;
using ManagedSetupUX;

namespace BurnSampleWPFUI
{
    /// <summary>
    /// Interaction logic for ProgressPage.xaml
    /// </summary>
    public partial class ProgressPage : UserControl
    {
        private BurnSetup m_MainWindow;
        private bool m_IsInstallationComplete = false;
        public bool IsInstallationComplete
        {
            get { return m_IsInstallationComplete; }
        }

        public ProgressPage(BurnSetup mainWindow)
        {
            InitializeComponent();
            m_MainWindow = mainWindow;                   

            pbInstallation.Minimum = 0;
            pbInstallation.Maximum = 100;
            pbDownload.Minimum = 0;
            pbDownload.Maximum = 100;
        }

        // Update download progress
        public void UpdateDownloadProgress(int progressPercentage, int overallProgressPercentage)
        {
            pbDownload.Value = overallProgressPercentage;
        }

        // Update execution progress
        public void UpdateExecutionProgress(int progressPercentage, int overallProgressPercentage)
        {
            pbInstallation.Value = overallProgressPercentage;
            int executionPercentage = progressPercentage;            
            lblInstallationPercentage.Content = executionPercentage.ToString()+ "%";
        }

        public void UpdateExectionPackageName(string packageName)
        {
            lblInstallationPackageName.Content = packageName;
        }

        public void DownloadComplete()
        {
            string executionLabel = string.Empty;
            if (m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY)
            {
                executionLabel = "Installing ...";
            }
            else if (m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY)
            {
                executionLabel = "Uninstalling ...";
            }
            else if (m_MainWindow.m_Mode == INSTALL_MODE.REPAIR_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.REPAIR_MIN_DISPLAY)
            {
                executionLabel = "Repairing ...";
            }

            lblInstalltionProgress.Content = executionLabel;
        }

        public void ExecutionComplete()
        {
            switch (m_MainWindow.m_Mode)
            {
                case INSTALL_MODE.INSTALL_FULL_DISPLAY:
                case INSTALL_MODE.INSTALL_MIN_DISPLAY:
                    lblInstalltionProgress.Content = "Installation Completed";
                    break;
                case INSTALL_MODE.REPAIR_FULL_DISPLAY:
                case INSTALL_MODE.REPAIR_MIN_DISPLAY:
                    lblInstalltionProgress.Content = "Repair Completed";
                    break;
                case INSTALL_MODE.UNINSTALL_FULL_DISPLAY:
                case INSTALL_MODE.UNINSTALL_MIN_DISPLAY:
                    lblInstalltionProgress.Content = "Uninstallation Completed";
                    break;
            }
            m_IsInstallationComplete = true;
        }

        public void Proceed()
        {
            // do nothing, Finish page will automatically be loaded when the ApplyComplete message is received.
        }

        public void UpdateExecutionPackageName(string packageName)
        {
            lblInstallationPackageName.Content = packageName;
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            m_MainWindow.ShowCancelWindow(ManagedSetupUX.UIPage.Progress);
        }

        private void btnSuspend_Click(object sender, RoutedEventArgs e)
        {
            m_MainWindow.ShowSuspendWindow(ManagedSetupUX.UIPage.Progress);
        }
    }
}
