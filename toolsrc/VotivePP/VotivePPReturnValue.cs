//-------------------------------------------------------------------------------------------------
// <copyright file="VotivePPReturnValue.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
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
