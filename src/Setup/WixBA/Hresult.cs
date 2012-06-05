//-------------------------------------------------------------------------------------------------
// <copyright file="Hresult.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Utility class to work with HRESULTs
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;

    /// <summary>
    /// Utility class to work with HRESULTs
    /// </summary>
    internal class Hresult
    {
        /// <summary>
        /// Determines if an HRESULT was a success code or not.
        /// </summary>
        /// <param name="status">HRESULT to verify.</param>
        /// <returns>True if the status is a success code.</returns>
        public static bool Succeeded(int status)
        {
            return status >= 0;
        }
    }
}
