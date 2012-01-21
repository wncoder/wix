//-----------------------------------------------------------------------
// <copyright file="ErrorCodes.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Common windows error codes
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    /// <summary>
    /// Common windows error codes (from winerror.h).  This is a static class of constants rather than an enum so it can be inherited and extended for custom errors.
    /// </summary>
    public static class ErrorCodes
    {
        public const Int32 ERROR_SUCCESS = 0;
        public const Int32 ERROR_INVALID_FUNCTION = 1;
        public const Int32 ERROR_SUCCESS_REBOOT_INITIATED = 1641;
        public const Int32 ERROR_SUCCESS_REBOOT_REQUIRED = 3010;
        public const Int32 ERROR_INSTALL_FAILURE = 1603;
    }
}
