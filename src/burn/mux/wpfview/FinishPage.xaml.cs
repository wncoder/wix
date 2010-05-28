//-------------------------------------------------------------------------------------------------
// <copyright file="FinishPage.xaml.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
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
