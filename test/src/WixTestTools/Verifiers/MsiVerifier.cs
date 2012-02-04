//-------------------------------------------------------------------------------------------------
// <copyright file="MsiVerifier.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
//      Contains methods for verification of MSIs.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Verifiers
{
    using System;
    using System.Linq;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;

    /// <summary>
    /// The MsiVerifier contains methods for verification of MSIs.
    /// </summary>
    public class MsiVerifier
    {
        /// <summary>
        /// Gets whether the product defined by the package <paramref name="path"/> is installed.
        /// </summary>
        /// <param name="path">The path to the package to test.</param>
        /// <returns>True if the package is installed; otherwise, false.</returns>
        public static bool IsPackageInstalled(string path)
        {
            string productCode = MsiUtils.GetMSIProductCode(path);
            return MsiUtils.IsProductInstalled(productCode);
        }
    }
}
