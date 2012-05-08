//-------------------------------------------------------------------------------------------------
// <copyright file="BinderExtensionEx.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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