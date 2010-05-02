//-----------------------------------------------------------------------
// <copyright file="MaintenanceModePage.xaml.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>window for uninstall, repair and change</summary>
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

namespace BurnSampleWPFUI
{
    /// <summary>
    /// Interaction logic for MaintenanceModePage.xaml
    /// </summary>
    public partial class MaintenanceModePage : UserControl
    {
        private BurnSetup m_MainWindow;

        public MaintenanceModePage(BurnSetup mainWindow)
        {
            InitializeComponent();
            m_MainWindow = mainWindow;
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            m_MainWindow.ShowCancelWindow(ManagedSetupUX.UIPage.Welcome);
        }

        private void btnRepair_Click(object sender, RoutedEventArgs e)
        {
            m_MainWindow.m_Mode = ManagedSetupUX.INSTALL_MODE.REPAIR_FULL_DISPLAY;

            if (File.Exists(m_MainWindow.m_ItemPlanFilePath))
                File.Delete(m_MainWindow.m_ItemPlanFilePath);

            m_MainWindow.PostActionMessage();
        }

        private void btnUninstall_Click(object sender, RoutedEventArgs e)
        {
            m_MainWindow.m_Mode = ManagedSetupUX.INSTALL_MODE.UNINSTALL_FULL_DISPLAY;

            if (File.Exists(m_MainWindow.m_ItemPlanFilePath))
                File.Delete(m_MainWindow.m_ItemPlanFilePath);

            m_MainWindow.PostActionMessage();
        }

        private void btnChange_Click(object sender, RoutedEventArgs e)
        {
            m_MainWindow.m_Mode = ManagedSetupUX.INSTALL_MODE.UNINSTALL_FULL_DISPLAY;
            m_MainWindow.LoadWelcomePage();
        }       
    }
}
