//-------------------------------------------------------------------------------------------------
// <copyright file="MsiPropertyInfo.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Utility class for Burn MsiProperty information.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Utility class for Burn MsiProperty information.
    /// </summary>
    internal class MsiPropertyInfo
    {
        public MsiPropertyInfo(Row row)
            : this((string)row[0], (string)row[1], (string)row[2])
        {
        }

        public MsiPropertyInfo(string packageId, string name, string value)
        {
            this.PackageId = packageId;
            this.Name = name;
            this.Value = value;
        }

        public string PackageId { get; private set; }
        public string Name { get; private set; }
        public string Value { get; set; }
    }
}
