//-------------------------------------------------------------------------------------------------
// <copyright file="WixBA.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
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
    using Threading = System.Windows.Threading;
    using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;

    /// <summary>
    /// The WiX toolset user experience.
    /// </summary>
    public class WixBA : BootstrapperApplication
    {
        /// <summary>
        /// Gets the global model.
        /// </summary>
        static public Model Model { get; private set; }

        /// <summary>
        /// Gets the global view.
        /// </summary>
        static public RootView View { get; private set; }
        // TODO: We should refactor things so we dont have a global View.

        /// <summary>
        /// Gets the global dispatcher.
        /// </summary>
        static public Threading.Dispatcher Dispatcher { get; private set; }

        /// <summary>
        /// Launches the default web browser to the provided URI.
        /// </summary>
        /// <param name="uri">URI to open the web browser.</param>
        public static void LaunchUrl(string uri)
        {
            // Switch the wait cursor since shellexec can take a second or so.
            Cursor cursor = WixBA.View.Cursor;
            WixBA.View.Cursor = Cursors.Wait;

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
                WixBA.View.Cursor = cursor; // back to the original cursor.
            }
        }

        /// <summary>
        /// Thread entry point for WiX Toolset UX.
        /// </summary>
        protected override void Run()
        {
            this.Engine.Log(LogLevel.Verbose, "Running the WiX BA.");
            WixBA.Model = new Model(this);
            WixBA.Dispatcher = Threading.Dispatcher.CurrentDispatcher;
            RootViewModel viewModel = new RootViewModel();

            // Populate the view models with the latest data. This is where Detect is called.
            viewModel.Refresh();

            // Create a Window to show UI.
            if (WixBA.Model.Command.Display == Display.Passive ||
                WixBA.Model.Command.Display == Display.Full)
            {
                this.Engine.Log(LogLevel.Verbose, "Creating a UI.");
                WixBA.View = new RootView(viewModel);
                WixBA.View.Show();
            }

            Threading.Dispatcher.Run();

            this.Engine.Quit(0);
        }
    }
}
