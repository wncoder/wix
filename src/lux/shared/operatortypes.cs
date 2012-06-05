//-------------------------------------------------------------------------------------------------
// <copyright file="operatortypes.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// The Lux unit-test framework test operator types enum.
// </summary>
//-------------------------------------------------------------------------------------------------



namespace Microsoft.Tools.WindowsInstallerXml.Lux
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using Microsoft.Deployment.WindowsInstaller;

    /// <summary>
    /// The allowed operators for the LuxUnitTest MSI table 'op' column.
    /// </summary>
    public enum LuxOperator
    {
        /// <summary>No value specified.</summary>
        NotSet = 0,

        /// <summary>Equality comparison.</summary>
        Equal,

        /// <summary>Inequality comparison.</summary>
        NotEqual,

        /// <summary>Case-insensitive equality comparison.</summary>
        CaseInsensitiveEqual,

        /// <summary>Case-insensitive inequality comparison.</summary>
        CaseInsensitiveNotEqual,
    }    
}
