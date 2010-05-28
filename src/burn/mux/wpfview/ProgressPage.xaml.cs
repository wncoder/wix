//-------------------------------------------------------------------------------------------------
// <copyright file="ProgressPage.xaml.cs" company="Microsoft">
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
