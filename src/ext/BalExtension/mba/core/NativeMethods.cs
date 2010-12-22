//-------------------------------------------------------------------------------------------------
// <copyright file="NativeMethods.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Container class for the IBootstrapperEngine interface passed to the IBootstrapperApplication.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;

    /// <summary>
    /// Contains native constants, functions, and structures for this assembly.
    /// </summary>
    internal static class NativeMethods
    {
        #region Error Constants
        internal const int S_OK = 0;
        internal const int E_MOREDATA = unchecked((int)0x800700ea);
        internal const int E_INSUFFICIENT_BUFFER = unchecked((int)0x8007007a);
        internal const int E_CANCELLED = unchecked((int)0x800704c7);
        internal const int E_ALREADYINITIALIZED = unchecked((int)0x800704df);
        internal const int E_NOTFOUND = unchecked((int)0x80070490);
        internal const int E_UNEXPECTED = unchecked((int)0x8000ffff);
        #endregion
    }
}
