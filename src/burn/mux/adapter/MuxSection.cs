//-------------------------------------------------------------------------------------------------
// <copyright file="MuxSection.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Handler for the mux configuration section.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Configuration;

    /// <summary>
    /// Handler for the mux configuration section.
    /// </summary>
    public sealed class MuxSection : ConfigurationSection
    {
        private static readonly ConfigurationProperty assemblyNameProperty;

        /// <summary>
        /// Initializes the configuration properties for this section.
        /// </summary>
        static MuxSection()
        {
            assemblyNameProperty = new ConfigurationProperty("assemblyName", typeof(string), null, ConfigurationPropertyOptions.IsRequired);
        }

        /// <summary>
        /// Creates a new instance of the <see cref="MuxSection"/> class.
        /// </summary>
        public MuxSection()
        {
        }

        /// <summary>
        /// Gets the name of the assembly that contians the <see cref="UserExperience"/> child class.
        /// </summary>
        /// <remarks>
        /// The assembly specified by this name must contain the <see cref="UserExperienceAttribute"/> to identify
        /// the type of the <see cref="UserExperience"/> child class.
        /// </remarks>
        [ConfigurationProperty("assemblyName", IsRequired = true)]
        public string AssemblyName
        {
            get { return (string)base[assemblyNameProperty]; }
            set { base[assemblyNameProperty] = value; }
        }
    }
}
