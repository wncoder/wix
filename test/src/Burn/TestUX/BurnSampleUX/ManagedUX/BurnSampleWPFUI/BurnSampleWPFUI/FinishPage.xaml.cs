//-----------------------------------------------------------------------
// <copyright file="FinishPage.xaml.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>window showing the summary of setup</summary>
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
using System.IO;
using System.Diagnostics;

namespace BurnSampleWPFUI
{
    /// <summary>
    /// Interaction logic for FinishPage.xaml
    /// </summary>
    public partial class FinishPage : UserControl
    {
        private BurnSetup m_mainWindow;

        public FinishPage(BurnSetup mainWindow)
        {
            InitializeComponent();
            m_mainWindow = mainWindow;
        }

        public void UpdateFinishMessage(int status, int suspend, bool cancelled)
        {
            string finishMessage = string.Empty;

            if (cancelled)
            {
                finishMessage = "{0} was cancelled";
            }
            else if (suspend != 0)
            {
                finishMessage = "{0} was suspended";
            }
            else if (status == 0)
            {
                finishMessage = "{0} was successful";
            }
            else if (status == 3010 || status == -2147021886)
            {
                finishMessage = "{0} was successful but requires a reboot";
                // BUGBUG TODO, hook up a "reboot now/later" dialog or additional buttons on the existing finish page.
            }
            else
            {
                finishMessage = "{0} failed";
            }

            switch (m_mainWindow.m_Mode)
            {
                case ManagedSetupUX.INSTALL_MODE.INSTALL_FULL_DISPLAY:
                case ManagedSetupUX.INSTALL_MODE.INSTALL_MIN_DISPLAY:
                    finishMessage = string.Format(finishMessage, "Installation");
                    break;
                case ManagedSetupUX.INSTALL_MODE.UNINSTALL_FULL_DISPLAY:
                case ManagedSetupUX.INSTALL_MODE.UNINSTALL_MIN_DISPLAY:
                    finishMessage = string.Format(finishMessage, "Uninstall");
                    break;
                case ManagedSetupUX.INSTALL_MODE.REPAIR_FULL_DISPLAY:
                case ManagedSetupUX.INSTALL_MODE.REPAIR_MIN_DISPLAY:
                    finishMessage = string.Format(finishMessage, "Repair");
                    break;
            }
            lblFinishMsg.Content = finishMessage;

            string logFilePath = GetBurnLogFilePath();

            if (!string.IsNullOrEmpty(logFilePath))
            {
                hyLinkLogFile.NavigateUri = new Uri(logFilePath);
            }
        }

        private string GetBurnLogFilePath()
        {
            string dhtmlLogFile = string.Empty;
            try
            {
                string[] files = Directory.GetFiles(Environment.ExpandEnvironmentVariables("%temp%"), "*.html", SearchOption.TopDirectoryOnly);
                DateTime fileCreatedDatetime = DateTime.Today;

                foreach (string file in files)
                {
                    if (file.Contains(m_mainWindow.m_PackageName))
                    {
                        if (File.GetLastWriteTime(file) > fileCreatedDatetime)
                        {
                            fileCreatedDatetime = File.GetLastWriteTime(file);
                            dhtmlLogFile = System.IO.Path.Combine(Environment.ExpandEnvironmentVariables("%temp%"), file);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

            return dhtmlLogFile;
        }

        public void Proceed()
        {
            // Post message cancel message to native and launch window
            m_mainWindow.PostCloseMessage();
            System.Threading.Thread.Sleep(2000);  // THIS IS A HACK... if I don't do this, there is some kind of timing issue and System.Windows.Threading.Dispatcher.Run() gets a null reference cause the UI is gone before the message queue is empty (I think)
            m_mainWindow.Close();
        }

        private void btnFinish_Click(object sender, RoutedEventArgs e)
        {
            this.Proceed();
        }

       

        private void hyLinkLogFile_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            Process.Start(new ProcessStartInfo(e.Uri.AbsoluteUri));

            e.Handled = true;
        }
    }
}
