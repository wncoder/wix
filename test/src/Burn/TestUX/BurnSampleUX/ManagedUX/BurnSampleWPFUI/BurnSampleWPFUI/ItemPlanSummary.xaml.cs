//-----------------------------------------------------------------------
// <copyright file="ItemPlanSummary.xaml.cs" company="Microsoft">
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
// <summary>window showing the list of items for install or uninstall</summary>
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
    /// Interaction logic for ItemPlanSummary.xaml
    /// </summary>
    public partial class ItemPlanSummary : UserControl
    {
        private BurnSetup m_MainWindow;       

        public ItemPlanSummary(BurnSetup mainWindow)
        {
            InitializeComponent();
            m_MainWindow = mainWindow;
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            m_MainWindow.ShowCancelWindow(ManagedSetupUX.UIPage.Welcome);
        }

        private void btnBack_Click(object sender, RoutedEventArgs e)
        {
            m_MainWindow.LoadWelcomePage();
        }

        public void Proceed()
        {
            m_MainWindow.PostActionMessage();
        }

        private void btnAction_Click(object sender, RoutedEventArgs e)
        {
            this.Proceed();
        }

        private void gridItemPlanSummary_Loaded(object sender, RoutedEventArgs e)
        {
            if (m_MainWindow != null)
            {
                string lblActionText = string.Empty;

                if (m_MainWindow.m_ItemSummaryList.Count > 1)
                {
                    lblActionText = "Following payloads will be {0}:";
                }
                else if (m_MainWindow.m_ItemSummaryList.Count > 0)
                {
                    lblActionText = "Following payload will be {0}:";
                }

                if (m_MainWindow.m_Mode == ManagedSetupUX.INSTALL_MODE.INSTALL_FULL_DISPLAY ||
                    m_MainWindow.m_Mode == ManagedSetupUX.INSTALL_MODE.INSTALL_MIN_DISPLAY)
                {
                    btnAction.Content = "Install";
                    lblAction.Content = string.Format(lblActionText, "installed");
                }

                if (m_MainWindow.m_Mode == ManagedSetupUX.INSTALL_MODE.REPAIR_FULL_DISPLAY ||
                    m_MainWindow.m_Mode == ManagedSetupUX.INSTALL_MODE.REPAIR_MIN_DISPLAY)
                {
                    btnAction.Content = "Repair";
                    lblAction.Content = string.Format(lblActionText, "repaired"); ;
                }

                if (m_MainWindow.m_Mode == ManagedSetupUX.INSTALL_MODE.UNINSTALL_FULL_DISPLAY ||
                    m_MainWindow.m_Mode == ManagedSetupUX.INSTALL_MODE.UNINSTALL_MIN_DISPLAY)
                {
                    btnAction.Content = "Uninstall";
                    lblAction.Content = string.Format(lblActionText, "uninstalled"); ;
                }

                UIElementCollection controls = gridItemPlanSummary.Children;
                foreach (UIElement control in controls)
                {
                    if (control.ToString() == "System.Windows.Controls.StackPanel")
                    {
                        control.Visibility = Visibility.Collapsed;
                    }
                }

                int stackPanelHeight = 71;
                int count = 0;

                foreach (string packageId in m_MainWindow.m_ItemSummaryList)
                {
                    string packageName = m_MainWindow.GetPackageName_IconSrc(packageId)[0];

                    StackPanel stp = new StackPanel();
                    stp.Orientation = Orientation.Horizontal;
                    stp.VerticalAlignment = VerticalAlignment.Top;
                    stp.HorizontalAlignment = HorizontalAlignment.Left;

                    stackPanelHeight += 27;

                    stp.Margin = new Thickness(12, stackPanelHeight, 13, 46);

                    Label lblNumber = new Label();
                    lblNumber.Height = 30;
                    lblNumber.Width = 6;
                    lblNumber.VerticalAlignment = VerticalAlignment.Top;
                    lblNumber.HorizontalAlignment = HorizontalAlignment.Left;
                    lblNumber.VerticalContentAlignment = VerticalAlignment.Top;
                    lblNumber.FontSize = 15;
                    lblNumber.Foreground = new SolidColorBrush(Colors.White);
                    lblNumber.Content = (++count).ToString();

                    TextBlock tb = new TextBlock();
                    tb.Height = 30;
                    tb.Width = 200;
                    tb.VerticalAlignment = VerticalAlignment.Center;
                    tb.HorizontalAlignment = HorizontalAlignment.Left;
                    tb.FontSize = 15;
                    tb.Margin = new Thickness(5);
                    tb.Foreground = new SolidColorBrush(Colors.White);

                    tb.Text = packageName;

                    stp.Children.Add(lblNumber);
                    stp.Children.Add(tb);

                    gridItemPlanSummary.Children.Add(stp);


                }
            }
        }        
    }
}
