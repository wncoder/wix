//-------------------------------------------------------------------------------------------------
// <copyright file="BundlePackageAttributes.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
    }
}
