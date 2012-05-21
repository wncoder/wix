//-----------------------------------------------------------------------
// <copyright file="ErrorCodes.cs" company="Microsoft">
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
// <summary>
//     Common windows error codes
// </summary>
//-----------------------------------------------------------------------

namespace WixTest
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
        public const Int32 ERROR_INSTALL_USEREXIT = 1602;
        public const Int32 ERROR_INSTALL_FAILURE = 1603;

        public static int ToHresult(int errorCode)
        {
            return unchecked((int)(((uint)errorCode & 0x0000FFFF) | (7 << 16) | 0x80000000));
        }
    }
}
