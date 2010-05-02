//-------------------------------------------------------------------------------------------------
// <copyright file="BurnSectionGroup.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
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
