//-------------------------------------------------------------------------------------------------
// <copyright file="WixUX.cs" company="Microsoft">
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
// The WiX toolset user experience.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;
    using System.Diagnostics;
    using System.Windows;
    using System.Windows.Input;
    using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;

    /// <summary>
    /// The WiX toolset user experience.
    /// </summary>
    public class WixUX : BootstrapperApplication
    {
        /// <summary>
        /// Gets the global model.
        /// </summary>
        static public Model Model { get; private set; }

        /// <summary>
        /// Gets the root view.
        /// </summary>
        static public RootView View { get; private set; }

        /// <summary>
        /// Launches the default web browser to the provided URI.
        /// </summary>
        /// <param name="uri">URI to open the web browser.</param>
        public static void LaunchUrl(string uri)
        {
            // Switch the wait cursor since shellexec can take a second or so.
            Cursor cursor = WixUX.View.Cursor;
            WixUX.View.Cursor = Cursors.Wait;

            try
            {
                Process process = new Process();
                process.StartInfo.FileName = uri;
                process.StartInfo.UseShellExecute = true;
                process.StartInfo.Verb = "open";

                process.Start();
            }
            finally
            {
                WixUX.View.Cursor = cursor; // back to the original cursor.
            }
        }

        /// <summary>
        /// UI Thread entry point for WiX Toolset UX.
        /// </summary>
        protected override void Run()
        {
            WixUX.Model = new Model(this);

            RootViewModel viewModel = new RootViewModel();
            WixUX.View = new RootView(viewModel);

            // Populate the view models with data then run the view..
            viewModel.Refresh();
            WixUX.View.Run();

            this.Engine.Quit(0);
        }
    }
}
