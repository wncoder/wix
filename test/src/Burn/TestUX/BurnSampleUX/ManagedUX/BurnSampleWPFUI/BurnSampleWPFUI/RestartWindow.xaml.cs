//-----------------------------------------------------------------------
// <copyright file="RestartWindow.xaml.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
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
