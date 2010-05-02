//-------------------------------------------------------------------------------------------------
// <copyright file="IUserExperienceFactory.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Class interface for the UserExperienceFactory class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Runtime.InteropServices;

    [ComVisible(true)]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [Guid("2965A12F-AC7B-43A0-85DF-E4B2168478A4")]
    public interface IBootstrapperUserExperienceFactory
    {
        IBurnUserExperience Create(ref Command command);
    }

    /// <summary>
    /// Command information passed from the engine for the user experience to perform.
    /// </summary>
    [Serializable]
    [StructLayout(LayoutKind.Sequential)]
    public struct Command
    {
        [MarshalAs(UnmanagedType.U4)] private readonly LaunchAction action;
        [MarshalAs(UnmanagedType.U4)] private readonly Display display;
        [MarshalAs(UnmanagedType.U4)] private readonly Restart restart;
        [MarshalAs(UnmanagedType.Bool)] private readonly bool resumed;

        /// <summary>
        /// Gets the action for the user experience to perform.
        /// </summary>
        public LaunchAction Action
        {
            get { return this.action; }
        }

        /// <summary>
        /// Gets the display level for the user experience.
        /// </summary>
        public Display Display
        {
            get { return this.display; }
        }

        /// <summary>
        /// Gets the action to perform if a reboot is required.
        /// </summary>
        public Restart Restart
        {
            get { return this.restart; }
        }

        /// <summary>
        /// Gets whether the engine was resumed from a previous installation step.
        /// </summary>
        public bool Resumed
        {
            get { return this.resumed; }
        }
    }
}
