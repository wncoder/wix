//-------------------------------------------------------------------------------------------------
// <copyright file="MaintenancePage.xaml.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Setup wizard page containing maintenance options.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Windows;

    /// <summary>
    /// Interaction logic for the Maintenance page.
    /// </summary>
    public partial class MaintenancePage : PageBase
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="MaintenancePage"/> class.
        /// </summary>
        public MaintenancePage()
        {
            this.InitializeComponent();
        }

        /// <summary>
        /// Event handler to handle the "change" user action
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Routed event</param>
        private void Change_Click(object sender, RoutedEventArgs e)
        {
            this.SetupWizard.UserAction = LaunchAction.Modify;
            this.SetupWizard.GoToNextPage();
        }

        /// <summary>
        /// Event handler to handle the "repair" user action
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Routed event</param>
        private void Repair_Click(object sender, RoutedEventArgs e)
        {
            this.SetupWizard.UserAction = LaunchAction.Repair;
            this.SetupWizard.GoToNextPage();
        }

        /// <summary>
        /// Event handler to handle the "uninstall" user action
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Routed event</param>
        private void Uninstall_Click(object sender, RoutedEventArgs e)
        {
            this.SetupWizard.UserAction = LaunchAction.Uninstall;
            this.SetupWizard.GoToNextPage();
        }
    }
}