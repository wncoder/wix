//-------------------------------------------------------------------------------------------------
// <copyright file="Exceptions.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Exceptions used by the managed user experience classes.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;

    /// <summary>
    /// Base class for exception returned to the user experience host.
    /// </summary>
    public abstract class BootstrapperException : Exception
    {
        /// <summary>
        /// Creates an instance of the <see cref="BootstrapperException"/> base class with the given HRESULT.
        /// </summary>
        /// <param name="hr">The HRESULT for the exception that is used by the user experience host.</param>
        protected BootstrapperException(int hr)
        {
            this.HResult = hr;
        }
    }

    /// <summary>
    /// The user experience loaded by the host does not contain exactly one instance of the
    /// <see cref="UserExperienceAttribute"/> class.
    /// </summary>
    /// <seealso cref="UserExperienceAttribute"/>
    public sealed class MissingAttributeException : BootstrapperException
    {
        /// <summary>
        /// Creates a new instance of the <see cref="MissingAttributeException"/> class.
        /// </summary>
        internal MissingAttributeException()
            : base(NativeMethods.E_NOTFOUND)
        {
        }
    }

    /// <summary>
    /// The user experience specified by the <see cref="UserExperienceAttribute"/> does not extend the
    /// <see cref="UserExperience"/> base class.
    /// </summary>
    /// <seealso cref="UserExperience"/>
    /// <seealso cref="UserExperienceAttribute"/>
    public sealed class InvalidUserExperienceException : BootstrapperException
    {
        /// <summary>
        /// Creates a new instance of the <see cref="InvalidUserExperienceException"/> class.
        /// </summary>
        internal InvalidUserExperienceException()
            : base(NativeMethods.E_UNEXPECTED)
        {
        }
    }
}
