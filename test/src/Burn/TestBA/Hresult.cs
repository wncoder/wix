//-------------------------------------------------------------------------------------------------
// <copyright file="Hresult.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Utility class to work with HRESULTs
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.BA
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
