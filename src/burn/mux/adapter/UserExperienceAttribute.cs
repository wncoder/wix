//-------------------------------------------------------------------------------------------------
// <copyright file="UserExperienceAttribute.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
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
// Identifies the user experience class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;

    /// <summary>
    /// Identifies the user experience class.
    /// </summary>
    /// <remarks>
    /// This required assembly attribute identifies the user experience class.
    /// </remarks>
    [AttributeUsage(AttributeTargets.Assembly, AllowMultiple = false)]
    public sealed class UserExperienceAttribute : Attribute
    {
        private Type type;

        /// <summary>
        /// Creates a new instance of the <see cref="UxFactoryAttribute"/> class.
        /// </summary>
        /// <param name="type">The <see cref="Type"/> of the user experience, or null for the default user experience.</param>
        public UserExperienceAttribute(Type type)
        {
            this.type = type;
        }

        /// <summary>
        /// Gets the type of the user experience class to create.
        /// </summary>
        public Type Type
        {
            get { return this.type; }
        }
    }
}
