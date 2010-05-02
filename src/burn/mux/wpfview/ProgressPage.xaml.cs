//-------------------------------------------------------------------------------------------------
// <copyright file="ProgressPage.xaml.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Setup wizard page containing setup progress.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;

    /// <summary>
    /// Interaction logic for the Progress page.
    /// </summary>
    public partial class ProgressPage : PageBase
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="ProgressPage"/> class.
        /// </summary>
        public ProgressPage()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Activates this page right before display.
        /// </summary>
        public override void Activate()
        {
            // Start the execution phase
            this.SetupWizard.Apply();
        }
    }
}
