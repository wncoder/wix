//-------------------------------------------------------------------------------------------------
// <copyright file="FinishPage.xaml.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Setup wizard page containing final setup status.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Globalization;

    /// <summary>
    /// Interaction logic for Finished page.
    /// </summary>
    public partial class FinishPage : PageBase
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="FinishPage"/> class.
        /// </summary>
        public FinishPage()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Activates this page right before it displays
        /// </summary>
        public override void Activate()
        {
            this.UpdateFinishMessage(this.SetupWizard.CompletionStatus);
        }

        /// <summary>
        /// Updates the finished message with the appropriate setup status and user action
        /// </summary>
        /// <param name="status">Value indicating the status of the overall setup</param>
        private void UpdateFinishMessage(int status)
        {
            string finishMessage = string.Empty;

            switch (this.SetupWizard.UserAction)
            {
                case LaunchAction.Install:
                    finishMessage = UserInterfaceResources.InstallationHeading;
                    break;

                case LaunchAction.Uninstall:
                    finishMessage = UserInterfaceResources.UninstallationHeading;
                    break;

                case LaunchAction.Repair:
                    finishMessage = UserInterfaceResources.RepairHeading;
                    break;

                case LaunchAction.Modify:
                    finishMessage = UserInterfaceResources.ChangeHeading;
                    break;

                default:
                    throw new InvalidOperationException("Unsupported LaunchAction");
            }

            finishStatus.Content = String.Format(
                CultureInfo.CurrentUICulture, 
                (status == 0) ? UserInterfaceResources.ExecutionSuccess : UserInterfaceResources.ExecutionFailure, 
                finishMessage);
        }
    }
}
