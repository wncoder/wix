//-----------------------------------------------------------------------
// <copyright file="WixTools.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>Contains an enumeration of the Wix Tools</summary>
//-----------------------------------------------------------------------

namespace WixTest
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
