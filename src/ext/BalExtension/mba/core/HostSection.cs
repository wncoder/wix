//-------------------------------------------------------------------------------------------------
// <copyright file="HostSection.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Handler for the Host configuration section.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Configuration;

    /// <summary>
    /// Handler for the Host configuration section.
    /// </summary>
    public sealed class HostSection : ConfigurationSection
    {
        private static readonly ConfigurationProperty assemblyNameProperty = new ConfigurationProperty("assemblyName", typeof(string), null, ConfigurationPropertyOptions.IsRequired);

        /// <summary>
        /// Creates a new instance of the <see cref="HostSection"/> class.
        /// </summary>
        public HostSection()
        {
        }

        /// <summary>
        /// Gets the name of the assembly that contians the <see cref="BootstrapperApplication"/> child class.
        /// </summary>
        /// <remarks>
        /// The assembly specified by this name must contain the <see cref="BootstrapperApplicationAttribute"/> to identify
        /// the type of the <see cref="BootstrapperApplication"/> child class.
        /// </remarks>
        [ConfigurationProperty("assemblyName", IsRequired = true)]
        public string AssemblyName
        {
            get { return (string)base[assemblyNameProperty]; }
            set { base[assemblyNameProperty] = value; }
        }
    }
}
