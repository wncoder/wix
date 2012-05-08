//-------------------------------------------------------------------------------------------------
// <copyright file="MsiPropertyInfo.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
