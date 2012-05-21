//-------------------------------------------------------------------------------------------------
// <copyright file="ExitCodeInfo.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
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
