//-----------------------------------------------------------------------
// <copyright file="WixTools.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Contains an enumeration of the Wix Tools</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;

    /// <summary>
    /// Contains an enumeration of the Wix Tools
    /// </summary>
    public abstract partial class WixTool : TestTool
    {
        /// <summary>
        /// The Wix Tools
        /// </summary>
        public enum WixTools
        {
            Any,
            Candle,
            Dark,
            Heat,
            Insignia,
            Lit,
            Light,
            Melt,
            Pyro,
            Smoke,
            Torch,
            Wixunit
        }
    }
}
