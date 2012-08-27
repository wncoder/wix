//-------------------------------------------------------------------------------------------------
// <copyright file="ReturnValue.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Enumerates all of the possible return values from the ResIdGen application.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio.Tools
{
    using System;

    /// <summary>
    /// Enumerates all of the possible return values from the ResIdGen application.
    /// </summary>
    public enum ReturnValue
    {
        Success = 0,
        UnknownError,
        InvalidParameters,
        ResourceFileNotFound,
        SourceFileNotFound,
        FileOpenError,
        FileReadWriteError,
        NoStartAutoGenerateTagFound,
        NoEndAutoGenarateTagFound,
    }
}
