//-------------------------------------------------------------------------------------------------
// <copyright file="MsiVerifier.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
//      Contains methods for verification of MSIs.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace WixTest.Verifiers
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using WixTest.Utilities;

    /// <summary>
    /// The MsiVerifier contains methods for verification of MSIs.
    /// </summary>
    public class MsiVerifier
    {
        private static Dictionary<string, string> PathsToProductCodes = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

        /// <summary>
        /// Gets whether the product defined by the package <paramref name="path"/> is installed.
        /// </summary>
        /// <param name="path">The path to the package to test.</param>
        /// <returns>True if the package is installed; otherwise, false.</returns>
        public static bool IsPackageInstalled(string path)
        {
            string productCode;
            if (!PathsToProductCodes.TryGetValue(path, out productCode))
            {
                productCode = MsiUtils.GetMSIProductCode(path);
                PathsToProductCodes.Add(path, productCode);
            }

            return MsiUtils.IsProductInstalled(productCode);
        }

        /// <summary>
        /// Gets whether the product defined by the package <paramref name="productCode"/> is installed.
        /// </summary>
        /// <param name="path">The product code of the package to test.</param>
        /// <returns>True if the package is installed; otherwise, false.</returns>
        public static bool IsProductInstalled(string productCode)
        {
            return MsiUtils.IsProductInstalled(productCode);
        }

        /// <summary>
        /// Resets the verifier to remove any cached results from previous tests.
        /// </summary>
        public static void Reset()
        {
            PathsToProductCodes.Clear();
        }
    }
}
