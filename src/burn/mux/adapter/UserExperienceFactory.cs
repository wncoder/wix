//-------------------------------------------------------------------------------------------------
// <copyright file="UserExperienceFactory.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Creates and returns the IBurnUserExperience implementation to the engine.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Configuration;
    using System.Reflection;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Class used by the UX host to create and return the IBurnUserExperience implementation to the engine.
    /// </summary>
    [ClassInterface(ClassInterfaceType.None)]
    public sealed class UserExperienceFactory : MarshalByRefObject, IBootstrapperUserExperienceFactory
    {
        /// <summary>
        /// Creates a new instance of the <see cref="UserExperienceFactory"/> class.
        /// </summary>
        public UserExperienceFactory()
        {
        }

        /// <summary>
        /// Locates the <see cref="UserExperienceAttribute"/> and returns the specified type.
        /// </summary>
        /// <param name="assemblyName">The assembly that defines the user experience class.</param>
        /// <returns>The user experience <see cref="Type"/>, or null if not found.</returns>
        /// <exception cref="MissingAttributeException">The assembly specified by <paramref name="assemblyName"/>
        /// does not define the <see cref="UserExperienceAttribute"/>.</exception>
        private Type GetUserExperienceTypeFromAssembly(string assemblyName)
        {
            // Load the requested assembly.
            if (!string.IsNullOrEmpty(assemblyName))
            {
                Assembly asm = AppDomain.CurrentDomain.Load(assemblyName);

                // If an assembly was loaded and is not the current assembly, check for the required attribute.
                // This is done to avoid using the UserExperienceAttribute which we use at build time
                // to specify the UX assembly in the manifest. This attribute is for custom UX assemblies.
                if (!Assembly.GetExecutingAssembly().Equals(asm))
                {
                    // There must be one and only one UserExperienceAttribute. The attribute prevents multiple declarations already.
                    UserExperienceAttribute[] attrs = (UserExperienceAttribute[])asm.GetCustomAttributes(typeof(UserExperienceAttribute), false);
                    if (null == attrs)
                    {
                        throw new MissingAttributeException();
                    }

                    return attrs[0].Type;
                }
            }

            return null;
        }

        /// <summary>
        /// Creates the <see cref="UserExperience"/> child class and returns it to the engine.
        /// </summary>
        /// <param name="command">The command information to modify the user experience.</param>
        /// <returns>The <see cref="UserExperience"/> child class that the engine will use for business logic
        /// and to communicate with the user interface.</returns>
        /// <exception cref="InvalidUserExperienceException">The specified class does not extend <see cref="UserExperience"/>.</exception>
        private UserExperience Create(ref Command command)
        {
            Type uxType = null;

            // Get the wix.burn section group to load the mux handler.
            MuxSection section = ConfigurationManager.GetSection("wix.burn/mux") as MuxSection;
            if (null != section)
            {
                uxType = GetUserExperienceTypeFromAssembly(section.AssemblyName);
            }

            // Use the default UX if not specified.
            if (null == uxType)
            {
                uxType = typeof(UserExperience);
            }

            // Create the UX and make sure it extends UserExperience.
            UserExperience ux = Activator.CreateInstance(uxType) as UserExperience;
            if (null == ux)
            {
                throw new InvalidUserExperienceException();
            }

            ux.Command = command;
            return ux;
        }

        IBurnUserExperience IBootstrapperUserExperienceFactory.Create(ref Command command)
        {
            // Create the AppDomain to load the UX assemblies.
            AppDomain muxDomain = AppDomain.CreateDomain("MUX");

            // Create the UX factory in the new AppDomain and create the UX class.
            Type factoryType = this.GetType();
            UserExperienceFactory factory = muxDomain.CreateInstanceAndUnwrap(
                factoryType.Assembly.FullName,
                factoryType.FullName,
                false,
                BindingFlags.CreateInstance,
                null,
                null,
                null,
                null,
                null) as UserExperienceFactory;

            // Call Create in the new AppDomain to load and create the UX.
            return factory.Create(ref command);
        }
    }
}
