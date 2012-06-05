//-------------------------------------------------------------------------------------------------
// <copyright file="WixOutputType.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Contains the WixOutputType enum.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;
    using System.ComponentModel;

    /// <summary>
    /// Enumeration for the various output types for a Wix project.
    /// </summary>
    public enum WixOutputType
    {
        /// <summary>
        /// Wix project that builds an MSI file.
        /// </summary>
        Package,

        /// <summary>
        /// Wix project that builds an MSM file.
        /// </summary>
        Module,

        /// <summary>
        /// Wix project that builds a wixlib file.
        /// </summary>
        Library,

        /// <summary>
        /// Wix project that builds a exe file.
        /// </summary>
        Bundle,
    }
}
