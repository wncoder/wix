//-------------------------------------------------------------------------------------------------
// <copyright file="ApplicationBarViewModel.cs" company="Microsoft">
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
// The model of the view for the application bar in the WixUX.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;
    using System.Windows.Input;

    /// <summary>
    /// The model of application bar view in the WixUX.
    /// </summary>
    public class ApplicationBarViewModel : PropertyNotifyBase
    {
        private RootViewModel root;

        private ICommand homeCommand;
        private ICommand helpCommand;
        private ICommand searchCommand;

        public ApplicationBarViewModel(RootViewModel root)
        {
            this.root = root;
        }

        public ICommand HomeCommand
        {
            get
            {
                if (this.homeCommand == null)
                {
                    this.homeCommand = new RelayCommand(param => WixUX.LaunchUrl("http://wixtoolset.org/"));
                }

                return this.homeCommand;
            }
        }

        public ICommand HelpCommand
        {
            get
            {
                if (this.helpCommand == null)
                {
                    this.helpCommand = new RelayCommand(param => WixUX.LaunchUrl("http://wixtoolset.org/help/"));
                }

                return this.helpCommand;
            }
        }

        public ICommand SearchCommand
        {
            get
            {
                if (this.searchCommand == null)
                {
                    this.searchCommand = new RelayCommand(param => WixUX.LaunchUrl("http://wixtoolset.org/search/"));
                }

                return this.searchCommand;
            }
        }

        public ICommand RefreshCommand
        {
            get { return this.root.RefreshCommand; }
        }
    }
}
