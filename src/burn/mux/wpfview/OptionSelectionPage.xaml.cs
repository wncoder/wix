//-------------------------------------------------------------------------------------------------
// <copyright file="OptionSelectionPage.xaml.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Setup wizard page containing user selectable options to install/uninstall/etc.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;

    /// <summary>
    /// Interaction logic for the Option Selection page.
    /// </summary>
    public partial class OptionSelectionPage : PageBase
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="OptionSelectionPage"/> class.
        /// </summary>
        public OptionSelectionPage()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Deactivates this page right before moving onto the next page
        /// </summary>
        public override void Deactivate()
        {
            // Start the planning stage
            this.SetupWizard.Plan();
        }
    }
}
