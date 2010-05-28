//-------------------------------------------------------------------------------------------------
// <copyright file="PageBase.cs" company="Microsoft">
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
// Base class for setup wizard pages.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Windows;
    using System.Windows.Controls;

    /// <summary>
    /// Base class for setup wizard pages.
    /// </summary>
    public class PageBase : UserControl
    {
        private PageBase nextPage;
        private PageBase previousPage;

        /// <summary>
        /// Initializes a new instance of the <see cref="PageBase"/> class.
        /// </summary>
        public PageBase()
        {
        }

        /// <summary>
        /// Gets the setup wizard that this page belongs to.
        /// </summary>
        public SetupWizard SetupWizard
        {
            get { return SetupWizard.Instance; }
        }

        /// <summary>
        /// Gets or sets the next page in the setup wizard sequence.
        /// </summary>
        public virtual PageBase NextPage
        {
            get { return this.nextPage; }
            set { this.nextPage = value; }
        }

        /// <summary>
        /// Gets or sets the previous page in the setup wizard sequence.
        /// </summary>
        public virtual PageBase PreviousPage
        {
            get { return this.previousPage; }
            set { this.previousPage = value; }
        }

        /// <summary>
        /// Activates this page right before display.
        /// </summary>
        public virtual void Activate()
        {
        }

        /// <summary>
        /// Deactivates this page right before moving onto the next page.
        /// </summary>
        public virtual void Deactivate()
        {
        }
    }
}
