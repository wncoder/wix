//-------------------------------------------------------------------------------------------------
// <copyright file="ResolvedDirectory.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
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
