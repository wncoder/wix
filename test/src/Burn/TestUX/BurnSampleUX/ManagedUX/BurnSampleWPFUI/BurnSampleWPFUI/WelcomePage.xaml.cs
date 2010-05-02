//-----------------------------------------------------------------------
// <copyright file="WelcomePage.xaml.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary></summary>
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
using System.Collections.Specialized;
using System.Collections;

namespace BurnSampleWPFUI
{
    /// <summary>
    /// Interaction logic for WelcomePage.xaml
    /// </summary>
    public partial class WelcomePage : UserControl
    {
        private BurnSetup m_MainWindow;
        private static int m_StackPanelHeight = 0;
        private static ListDictionary packageOrginalState;
        private static bool m_IsWelcomePageLoaded = false;

        public WelcomePage(BurnSetup mainWindow)
        {
            InitializeComponent();
            m_MainWindow = mainWindow;
        }

        public void Proceed()
        {
            try
            {
                m_MainWindow.RemoveRequestStateAttribute();

                m_MainWindow.m_ItemSummaryList = new List<string>();

                UIElementCollection controls = gridWelcome.Children;
                foreach (UIElement control in controls)
                {
                    if (control.ToString() == "System.Windows.Controls.StackPanel")
                    {
                        System.Windows.Controls.StackPanel sp = (System.Windows.Controls.StackPanel)control;
                        UIElementCollection stackPanelChilds = sp.Children;

                        foreach (UIElement spChild in stackPanelChilds)
                        {
                            if (spChild.GetType().ToString() == "System.Windows.Controls.CheckBox")
                            {
                                System.Windows.Controls.CheckBox cb = (System.Windows.Controls.CheckBox)spChild;
                                string packageId = (string)cb.Tag;

                                if (m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_FULL_DISPLAY ||
                                    m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY)
                                {
                                    if (cb.IsChecked == false)
                                    {
                                        m_MainWindow.UpdateItemPlan(packageId, REQUEST_STATE.NONE);
                                    }
                                }
                                else if (m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_FULL_DISPLAY ||
                                    m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY ||
                                    m_MainWindow.m_Mode == INSTALL_MODE.REPAIR_FULL_DISPLAY ||
                                    m_MainWindow.m_Mode == INSTALL_MODE.REPAIR_MIN_DISPLAY)
                                {
                                    if (cb.IsChecked == true && (PACKAGE_STATE)packageOrginalState[packageId] == PACKAGE_STATE.PRESENT)
                                    {
                                        m_MainWindow.UpdateItemPlan(packageId, REQUEST_STATE.NONE);
                                    }
                                    else if (cb.IsChecked == true && (PACKAGE_STATE)packageOrginalState[packageId] == PACKAGE_STATE.ABSENT)
                                    {
                                        m_MainWindow.UpdateItemPlan(packageId, REQUEST_STATE.PRESENT);
                                    }
                                }

                                if ((m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY) && cb.IsChecked == true)
                                {
                                    m_MainWindow.m_ItemSummaryList.Add(packageId);
                                }
                                else if ((m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY) && cb.IsChecked == false
                                    && (PACKAGE_STATE)packageOrginalState[packageId] == PACKAGE_STATE.PRESENT)
                                {
                                    m_MainWindow.m_ItemSummaryList.Add(packageId);
                                }
                                else if ((m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY) && cb.IsChecked == true
                                    && (PACKAGE_STATE)packageOrginalState[packageId] == PACKAGE_STATE.ABSENT)
                                {
                                    m_MainWindow.m_ItemSummaryList.Add(packageId); // BugBug
                                }
                            }
                        }
                    }
                }

                m_MainWindow.LoadItemPlanSummaryPage();
            }

            catch (Exception ex)
            {
                MessageBox.Show(ex.StackTrace);
            }
        }

        private void btnNext_Click(object sender, RoutedEventArgs e)
        {
            this.Proceed();
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            m_MainWindow.ShowCancelWindow(ManagedSetupUX.UIPage.Welcome);
        }

        private void gridWelcome_Loaded(object sender, RoutedEventArgs e)
        {
            try
            {

                switch (m_MainWindow.m_Mode)
                {
                    case INSTALL_MODE.INSTALL_FULL_DISPLAY:
                    case INSTALL_MODE.INSTALL_MIN_DISPLAY:
                        lblWelcomePageHeader.Content = "Install";
                        break;
                    case INSTALL_MODE.REPAIR_FULL_DISPLAY:
                    case INSTALL_MODE.REPAIR_MIN_DISPLAY:
                        lblWelcomePageHeader.Content = "Repair";
                        break;
                    case INSTALL_MODE.UNINSTALL_FULL_DISPLAY:
                    case INSTALL_MODE.UNINSTALL_MIN_DISPLAY:
                        lblWelcomePageHeader.Content = "Uninstall";
                        break;
                }

                if (m_MainWindow.m_packageInfo.Count > 0 && !m_IsWelcomePageLoaded)
                {
                    packageOrginalState = new ListDictionary();

                    m_IsWelcomePageLoaded = true;
                    bool isUninstallSelected = true;

                    IDictionaryEnumerator packageInfoEnumerator = m_MainWindow.m_packageInfo.GetEnumerator();

                    while (packageInfoEnumerator.MoveNext())
                    {
                        StructPackageInfo packageInfo = (StructPackageInfo)packageInfoEnumerator.Value;
                        string packageId = packageInfo.packageId;
                        string packageName = packageInfo.packageName;
                        PACKAGE_STATE state = packageInfo.packageState;

                        packageOrginalState.Add(packageId, state);

                        StackPanel stp = new StackPanel();
                        stp.Orientation = Orientation.Horizontal;
                        stp.VerticalAlignment = VerticalAlignment.Top;
                        stp.HorizontalAlignment = HorizontalAlignment.Left;

                        if (m_StackPanelHeight == 0)
                        {
                            m_StackPanelHeight = 80;
                        }
                        else
                        {
                            m_StackPanelHeight += 30;
                        }

                        stp.Margin = new Thickness(12, m_StackPanelHeight, 13, 46);

                        CheckBox cb = new CheckBox();
                        cb.Margin = new Thickness(2);
                        cb.Height = 30;
                        cb.VerticalAlignment = VerticalAlignment.Bottom;
                        cb.HorizontalAlignment = HorizontalAlignment.Left;
                        cb.Tag = (object)packageId;
                        cb.IsChecked = false;
                        cb.Click += new RoutedEventHandler(cb_Click);


                        TextBlock tb = new TextBlock();
                        tb.Height = 30;
                        tb.Width = 200;
                        tb.VerticalAlignment = VerticalAlignment.Center;
                        tb.HorizontalAlignment = HorizontalAlignment.Left;
                        tb.FontSize = 15;
                        tb.Margin = new Thickness(5);
                        tb.Foreground = new SolidColorBrush(Colors.White);

                        if (state == PACKAGE_STATE.ABSENT && (m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY))
                        {
                            cb.IsChecked = true;                            
                        }
                        else if (state == PACKAGE_STATE.PRESENT && (m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY))
                        {
                            continue;
                        }
                        else if (state == PACKAGE_STATE.PRESENT && (m_MainWindow.m_Mode == INSTALL_MODE.REPAIR_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.REPAIR_MIN_DISPLAY))
                        {
                            cb.IsChecked = true;
                        }
                        else if (state == PACKAGE_STATE.PRESENT && (m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY))
                        {
                            cb.IsChecked = true;
                            isUninstallSelected = false;
                        }
                        else if (state == PACKAGE_STATE.ABSENT && (m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY))
                        {
                            cb.IsChecked = false;
                        }

                        tb.Text = packageName;

                        stp.Children.Add(cb);
                        stp.Children.Add(tb);

                        gridWelcome.Children.Add(stp);
                    }

                    if (m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY)
                    {
                        if (m_MainWindow.m_packageInfo.Count > 1)
                        {
                            lblInstruction.Content = "Select the payload(s) to be installed:";
                        }
                        else if (m_MainWindow.m_packageInfo.Count > 0)
                        {
                            lblInstruction.Content = "Select the payload to be installed:";
                        }
                    }

                    EnableDisableNextButton();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void EnableDisableNextButton()
        {
            UIElementCollection controls = gridWelcome.Children;
            bool isPayloadSelected = false;
            bool isPayloadUnselected = false;

            foreach (UIElement control in controls)
            {
                if (control.ToString() == "System.Windows.Controls.StackPanel")
                {
                    System.Windows.Controls.StackPanel sp = (System.Windows.Controls.StackPanel)control;
                    UIElementCollection stackPanelChilds = sp.Children;

                    foreach (UIElement spChild in stackPanelChilds)
                    {
                        if (spChild.GetType().ToString() == "System.Windows.Controls.CheckBox")
                        {
                            System.Windows.Controls.CheckBox cb = (System.Windows.Controls.CheckBox)spChild;

                            if (cb.IsChecked == true)
                            {
                                 isPayloadSelected = true;                              
                            }
                            else
                            {
                                isPayloadUnselected = true;
                            }
                        }
                    }
                }
            }
           
            // In install mode if payload is selected then enable the Next button
            if ((m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY) && isPayloadSelected == true)
            {             
                btnNext.IsEnabled = true;
                btnNext.Opacity = 1;
            }
            else if ((m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.INSTALL_MIN_DISPLAY) && isPayloadSelected == false)
            {
                btnNext.IsEnabled = false;
                btnNext.Opacity = 0.2;
            }

            // In Change or modify mode enable the Next button if any of the payload is unselected
            if ((m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY) && isPayloadUnselected == true)
            {
                btnNext.IsEnabled = true;
                btnNext.Opacity = 1;
            }
            else if ((m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_FULL_DISPLAY || m_MainWindow.m_Mode == INSTALL_MODE.UNINSTALL_MIN_DISPLAY) && isPayloadUnselected == false)
            {
                btnNext.IsEnabled = true;
                btnNext.Opacity = 1; // BugBug change
            }
        }

        private void cb_Click(object sender, System.EventArgs e)
        {
            EnableDisableNextButton();
        }
    }
}
