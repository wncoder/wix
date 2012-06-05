//-------------------------------------------------------------------------------------------------
// <copyright file="VotivePPReturnValue.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Enumerates all of the possible return values from the VotivePP application.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio.Tools
{
    using System;

    /// <summary>
    /// Enumerates all of the possible return values from the VotivePP application.
    /// </summary>
    public enum VotivePPReturnValue
    {
        Success = 0,
        UnknownError,
        InvalidParameters,
        InvalidPlaceholderParam,
        SourceFileNotFound,
        FileReadError,
    }
}
