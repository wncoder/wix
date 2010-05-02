//-------------------------------------------------------------------------------------------------
// <copyright file="PageBase.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
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
