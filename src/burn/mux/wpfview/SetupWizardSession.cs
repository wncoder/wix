//-------------------------------------------------------------------------------------------------
// <copyright file="SetupWizardSession.cs" company="Microsoft">
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
// Top-level container for business logic shared between managed user experience components.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;

    /// <summary>
    /// The setup session for UX components.
    /// This is the entry point UX type created by the host.
    /// </summary>
    public class SetupWizardSession : SetupSession
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="SetupWizardSession"/> class.
        /// </summary>
        public SetupWizardSession()
        {
        }

        /// <summary>
        /// Called by the engine to run the user experience.
        /// </summary>
        protected void Run()
        {
            SetupWizard mainWindow = new SetupWizard(this);

            // Bind View to Model
            mainWindow.DataContext = this;

            // Show the application window
            mainWindow.ShowDialog();
        }

    }
}
