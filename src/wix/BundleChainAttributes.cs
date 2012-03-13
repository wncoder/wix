//-------------------------------------------------------------------------------------------------
// <copyright file="BundleChainAttributes.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// Bit flags for a bundle chain in the Windows Installer Xml toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Attributes available for a bundle chain.
    /// </summary>
    [Flags]
    internal enum BundleChainAttributes : int
    {
        None = 0x0,
        DisableRollback = 0x1,
        DisableSystemRestore = 0x2,
        ParallelCache = 0x4,
    }
}
