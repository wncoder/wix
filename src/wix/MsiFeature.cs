//-------------------------------------------------------------------------------------------------
// <copyright file="MsiFeature.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// MSI Feature Information.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Msi Feature Information.
    /// </summary>
    internal class MsiFeature
    {
        public string Name { get; set; }
        public int Size { get; set; }
    }
}
