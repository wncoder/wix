//-------------------------------------------------------------------------------------------------
// <copyright file="MsiFeature.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
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
        public long Size { get; set; }
        public string Parent { get; set; }
        public string Title { get; set; }
        public string Description { get; set; }
        public int Display { get; set; }
        public int Level { get; set; }
        public string Directory { get; set; }
        public int Attributes { get; set; }
    }
}
