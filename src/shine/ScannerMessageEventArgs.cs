//-------------------------------------------------------------------------------------------------
// <copyright file="ScannerMessageEventArgs.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

using System;

namespace Microsoft.Tools.WindowsInstallerXml
{
    public enum ScannerMessageType
    {
        Normal,
        Verbose,
        Warning,
        Error,
    }

    public delegate void ScannerMessageEventHandler(object sender, ScannerMessageEventArgs e);

    public class ScannerMessageEventArgs : EventArgs
    {
        public string Message { get; set; }

        public ScannerMessageType Type { get; set; }
    }
}
