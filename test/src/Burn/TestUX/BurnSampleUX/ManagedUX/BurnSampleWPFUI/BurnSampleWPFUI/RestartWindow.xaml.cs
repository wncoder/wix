//-----------------------------------------------------------------------
// <copyright file="RestartWindow.xaml.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>window to take user choice to restart system</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace BurnSampleWPFUI
{
    /// <summary>
    /// Interaction logic for RestartWindow.xaml
    /// </summary>
    public partial class RestartWindow : Window
    {
        public RestartWindow()
        {
            InitializeComponent();
        }

        public void DoModal()
        {
            ShowDialog();
        }
    }
}
