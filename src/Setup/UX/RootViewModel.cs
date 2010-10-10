//-------------------------------------------------------------------------------------------------
// <copyright file="RootViewModel.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// The model of the view for the WixUX.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;
    using System.Diagnostics;
    using System.Reflection;
    using System.Windows;
    using System.Windows.Input;
    using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;

    /// <summary>
    /// The model of the root view in WixUX.
    /// </summary>
    public class RootViewModel : PropertyNotifyBase
    {
        private ICommand closeCommand;
        private ICommand refreshCommand;

        /// <summary>
        /// Creates a new model of the root view.
        /// </summary>
        public RootViewModel()
        {
            this.Title = String.Concat("Windows Installer XML Toolset v", WixUX.Model.Version.ToString());

            this.ApplicationBarViewModel = new ApplicationBarViewModel(this);
            this.InstallationViewModel = new InstallationViewModel(this);
            this.NewsViewModel = new NewsViewModel();
            this.UpdateViewModel = new UpdateViewModel();
        }

        public ApplicationBarViewModel ApplicationBarViewModel { get; private set; }

        public InstallationViewModel InstallationViewModel { get; private set; }

        public NewsViewModel NewsViewModel { get; private set; }

        public UpdateViewModel UpdateViewModel { get; private set; }

        public ICommand CloseCommand
        {
            get
            {
                if (this.closeCommand == null)
                {
                    this.closeCommand = new RelayCommand(param => WixUX.View.Close());
                }

                return this.closeCommand;
            }
        }

        public ICommand RefreshCommand
        {
            get
            {
                if (this.refreshCommand == null)
                {
                    this.refreshCommand = new RelayCommand(param => this.Refresh(), param => false); // TODO: enable this command
                }

                return this.refreshCommand;
            }
        }

        /// <summary>
        /// Gets the title for the application.
        /// </summary>
        public string Title { get; private set; }

        /// <summary>
        /// Instructs the various child models to refresh. Called directly via
        /// the UX *once* to initialize all the models. After that, only called
        /// when the RefreshCommand is executed.
        /// </summary>
        public void Refresh()
        {
            this.InstallationViewModel.Refresh();
            this.UpdateViewModel.Refresh();
            this.NewsViewModel.Refresh();
        }
    }
}
