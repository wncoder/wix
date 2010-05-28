//-------------------------------------------------------------------------------------------------
// <copyright file="BurnSectionGroup.cs" company="Microsoft">
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
// Handler for the wix.burn configuration section group.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Configuration;

    /// <summary>
    /// Handler for the wix.burn configuration section group.
    /// </summary>
    public class BurnSectionGroup : ConfigurationSectionGroup
    {
        /// <summary>
        /// Creates a new instance of the <see cref="BurnSectionGroup"/> class.
        /// </summary>
        public BurnSectionGroup()
        {
        }

        /// <summary>
        /// Gets the <see cref="MuxSection"/> handler for the mux configuration section.
        /// </summary>
        [ConfigurationProperty("mux")]
        public MuxSection Mux
        {
            get { return (MuxSection)base.Sections["mux"]; }
        }
    }
}
