//-------------------------------------------------------------------------------------------------
// <copyright file="OptionSelectionPage.xaml.cs" company="Microsoft">
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
