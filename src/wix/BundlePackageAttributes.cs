//-------------------------------------------------------------------------------------------------
// <copyright file="BundlePackageAttributes.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Bit flags for a bundle package in the Windows Installer Xml toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Attributes available for a bundle package.
    /// </summary>
    [Flags]
    internal enum BundlePackageAttributes : int
    {
        None = 0x0,
        Permanent = 0x1,
        Visible = 0x2,
        Slipstream = 0x4,
    }
}
