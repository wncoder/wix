//-------------------------------------------------------------------------------------------------
// <copyright file="ExitCodeInfo.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// Utility class for Burn ExitCode information.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Utility class for Burn ExitCode information.
    /// </summary>
    internal class ExitCodeInfo
    {
        public ExitCodeInfo(Row row)
            : this((string)row[0], (int)row[1], (string)row[2])
        {
        }

        public ExitCodeInfo(string packageId, int value, string behavior)
        {
            this.PackageId = packageId;
            // null value means wildcard
            if (CompilerCore.IntegerNotSet == value)
            {
                this.Code = "*";
            }
            else
            {
                this.Code = value.ToString();
            }
            this.Type = behavior;
        }

        public string PackageId { get; private set; }
        public string Code { get; private set; }
        public string Type { get; private set; }
    }
}
