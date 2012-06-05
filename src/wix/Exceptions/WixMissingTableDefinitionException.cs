//-------------------------------------------------------------------------------------------------
// <copyright file="WixMissingTableDefinitionException.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Exception thrown when a table definition is missing.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Exception thrown when a table definition is missing.
    /// </summary>
    [Serializable]
    public class WixMissingTableDefinitionException : WixException
    {
        /// <summary>
        /// Instantiate new WixMissingTableDefinitionException.
        /// </summary>
        /// <param name="error">Localized error information.</param>
        public WixMissingTableDefinitionException(WixErrorEventArgs error)
            : base(error)
        {
        }
    }
}
