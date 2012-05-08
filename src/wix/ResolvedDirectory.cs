//-------------------------------------------------------------------------------------------------
// <copyright file="ResolvedDirectory.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// Structure used for resolved directory information.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Structure used for resolved directory information.
    /// </summary>
    internal struct ResolvedDirectory
    {
        /// <summary>The directory parent.</summary>
        public string DirectoryParent;

        /// <summary>The name of this directory.</summary>
        public string Name;

        /// <summary>The path of this directory.</summary>
        public string Path;

        /// <summary>
        /// Constructor for ResolvedDirectory.
        /// </summary>
        /// <param name="directoryParent">Parent directory.</param>
        /// <param name="name">The directory name.</param>
        public ResolvedDirectory(string directoryParent, string name)
        {
            this.DirectoryParent = directoryParent;
            this.Name = name;
            this.Path = null;
        }
    }
}
