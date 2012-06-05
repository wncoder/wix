//-------------------------------------------------------------------------------------------------
// <copyright file="BinderExtensionEx.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// The extended base binder extension.  Any of these methods can be overridden to perform binding tasks at
// various stages during the binding process.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Base class for creating an extended binder extension.
    /// </summary>
    public abstract class BinderExtensionEx : BinderExtension
    {
        /// <summary>
        /// Called after database variable resolution occurs.
        /// </summary>
        public virtual void DatabaseAfterResolvedFields(Output output)
        {
        }
    }
}