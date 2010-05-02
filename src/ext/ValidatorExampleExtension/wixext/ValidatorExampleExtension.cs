//-------------------------------------------------------------------------------------------------
// <copyright file="ValidatorExampleExtension.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// The Windows Installer XML Toolset Validator Example Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Reflection;

    /// <summary>
    /// The Windows Installer XML Toolset Validator Example Extension.
    /// </summary>
    public sealed class ValidatorExampleExtension : WixExtension
    {
        private ValidatorExtension extension;

        /// <summary>
        /// Gets the optional validator extension.
        /// </summary>
        /// <value>The optional validator extension.</value>
        public override ValidatorExtension ValidatorExtension
        {
            get
            {
                if (null == this.extension)
                {
                    this.extension = new ValidatorXmlExtension();
                }

                return this.extension;
            }
        }
    }
}
