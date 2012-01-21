//-------------------------------------------------------------------------------------------------
// <copyright file="RootView.xaml.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// View for WiX UX.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;
    using System.ComponentModel;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Threading;

    /// <summary>
    /// Interaction logic for View.xaml
    /// </summary>
    public partial class RootView : Window
    {
        /// <summary>
        /// Creates the view populated with it's model.
        /// </summary>
        /// <param name="viewModel">Model for the view.</param>
        public RootView(RootViewModel viewModel)
        {
            this.DataContext = viewModel;

            this.Loaded += (sender, e) => WixBA.Model.Engine.CloseSplashScreen();
            this.Closed += (sender, e) => this.Dispatcher.InvokeShutdown(); // shutdown dispatcher when the window is closed.

            this.InitializeComponent();
        }

        /// <summary>
        /// Allows the user to drag the window around by grabbing the background rectangle.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Background_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            this.DragMove();
        }
    }
}
