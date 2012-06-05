//-------------------------------------------------------------------------------------------------
// <copyright file="BootstrapperSectionGroup.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Handler for the wix.bootstrapper configuration section group.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Configuration;

    /// <summary>
    /// Handler for the wix.bootstrapper configuration section group.
    /// </summary>
    public class BootstrapperSectionGroup : ConfigurationSectionGroup
    {
        /// <summary>
        /// Creates a new instance of the <see cref="BootstrapperSectionGroup"/> class.
        /// </summary>
        public BootstrapperSectionGroup()
        {
        }

        /// <summary>
        /// Gets the <see cref="HostSection"/> handler for the mux configuration section.
        /// </summary>
        [ConfigurationProperty("host")]
        public HostSection Host
        {
            get { return (HostSection)base.Sections["host"]; }
        }
    }
}
