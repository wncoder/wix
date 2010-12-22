//-------------------------------------------------------------------------------------------------
// <copyright file="BootstrapperApplicationFactory.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Creates and returns the IBootstrapperApplication implementation to the engine.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Configuration;
    using System.Reflection;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Class used by the MUX host to create and return the IBootstrapperApplication implementation to the engine.
    /// </summary>
    [ClassInterface(ClassInterfaceType.None)]
    public sealed class BootstrapperApplicationFactory : MarshalByRefObject, IBootstrapperApplicationFactory
    {
        /// <summary>
        /// Creates a new instance of the <see cref="BootstrapperApplicationFactory"/> class.
        /// </summary>
        public BootstrapperApplicationFactory()
        {
        }

        /// <summary>
        /// Locates the <see cref="BootstrapperApplicationAttribute"/> and returns the specified type.
        /// </summary>
        /// <param name="assemblyName">The assembly that defines the user experience class.</param>
        /// <returns>The user experience <see cref="Type"/>, or null if not found.</returns>
        /// <exception cref="MissingAttributeException">The assembly specified by <paramref name="assemblyName"/>
        /// does not define the <see cref="BootstrapperApplicationAttribute"/>.</exception>
        private static Type GetBootstrapperApplicationTypeFromAssembly(string assemblyName)
        {
            // Load the requested assembly.
            if (!String.IsNullOrEmpty(assemblyName))
            {
                Assembly asm = AppDomain.CurrentDomain.Load(assemblyName);

                // If an assembly was loaded and is not the current assembly, check for the required attribute.
                // This is done to avoid using the BootstrapperApplicationAttribute which we use at build time
                // to specify the BootstrapperApplication assembly in the manifest. This attribute is for custom
                // BootstrapperApplication assemblies.
                if (!Assembly.GetExecutingAssembly().Equals(asm))
                {
                    // There must be one and only one BootstrapperApplicationAttribute. The attribute prevents multiple declarations already.
                    BootstrapperApplicationAttribute[] attrs = (BootstrapperApplicationAttribute[])asm.GetCustomAttributes(typeof(BootstrapperApplicationAttribute), false);
                    if (null != attrs)
                    {
                        return attrs[0].BootstrapperApplicationType;
                    }
                }
            }

            return null;
        }

        IBootstrapperApplication IBootstrapperApplicationFactory.Create(IBootstrapperEngine pEngine, ref Command command)
        {
            Type baType = null;

            // Get the wix.boostrapper section group to load the mux handler.
            HostSection section = ConfigurationManager.GetSection("wix.bootstrapper/host") as HostSection;
            if (null != section)
            {
                baType = BootstrapperApplicationFactory.GetBootstrapperApplicationTypeFromAssembly(section.AssemblyName);
            }

            // A derived managed BootstrapperApplication is required.
            if (null == baType)
            {
                throw new MissingAttributeException();
            }

            // Create the UX and make sure it extends BootstrapperApplication.
            BootstrapperApplication ba = Activator.CreateInstance(baType) as BootstrapperApplication;
            if (null == ba)
            {
                throw new InvalidBootstrapperApplicationException();
            }

            ba.Engine = new Engine(pEngine);
            ba.Command = command;
            return ba;
        }
    }
}
