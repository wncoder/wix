//---------------------------------------------------------------------
// <copyright file="CustomActionAttribute.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>
// Part of the Deployment Tools Foundation project.
// </summary>
//---------------------------------------------------------------------

namespace Microsoft.Deployment.WindowsInstaller
{
    using System;
    using System.Reflection;

    /// <summary>
    /// Marks a method as a custom action entry point.
    /// </summary>
    /// <remarks><p>
    /// A custom action method must be defined as public and static,
    /// take a single <see cref="Session"/> object as a parameter,
    /// and return an <see cref="ActionResult"/> enumeration value.
    /// </p></remarks>
    [Serializable, AttributeUsage(AttributeTargets.Method)]
    public sealed class CustomActionAttribute : Attribute
    {
        /// <summary>
        /// Name of the custom action entrypoint, or null if the same as the method name.
        /// </summary>
        private string name;

        /// <summary>
        /// Marks a method as a custom action entry point.
        /// </summary>
        public CustomActionAttribute()
            : this(null)
        {
        }

        /// <summary>
        /// Marks a method as a custom action entry point.
        /// </summary>
        /// <param name="name">Name of the function to be exported,
        /// defaults to the name of this method</param>
        public CustomActionAttribute(string name)
        {
            this.name = name;
        }

        /// <summary>
        /// Gets or sets the name of the custom action entrypoint. A null
        /// value defaults to the name of the method.
        /// </summary>
        /// <value>name of the custom action entrypoint, or null if none was specified</value>
        public string Name
        {
            get
            {
                return this.name;
            }
        }
    }
}
