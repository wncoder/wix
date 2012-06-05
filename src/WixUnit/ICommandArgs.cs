//-------------------------------------------------------------------------------------------------
// <copyright file="ICommandArgs.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Interface for passing command line arguments to tool classes.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Unit
{
    using System;

    /// <summary>
    /// Interface for passing command line arguments to tool classes.
    /// </summary>
    public interface ICommandArgs
    {
        /// <summary>
        /// Whether to remove intermediate build files.
        /// </summary>
        bool NoTidy { get; }
    }
}
