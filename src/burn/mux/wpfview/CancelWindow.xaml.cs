//-------------------------------------------------------------------------------------------------
// <copyright file="CancelWindow.xaml.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Dialog to cancel setup wizard
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Windows;
    using System.Windows.Input;

    /// <summary>
    /// Interaction logic for CancelWindow.xaml
    /// </summary>
    public partial class CancelWindow : Window
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="CancelWindow"/> class.
        /// </summary>
        public CancelWindow()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Shows a modal dialog
        /// </summary>
        /// <returns>User response of whether to cancel the setup or not</returns>
        public bool? DoModal()
        {
            return this.ShowDialog();
        }

        /// <summary>
        /// Event handler for a 'no' user choice
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Routed event</param>
        private void CancelNo_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            this.Close();
        }

        /// <summary>
        /// Event handler for a 'yes' user choice
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Routed event</param>
        private void CancelYes_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
            this.Close();
        }
    }
}
