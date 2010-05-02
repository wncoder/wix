//-------------------------------------------------------------------------------------------------
// <copyright file="IBurnCore.cs" company="Microsoft">
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
// COM interop interface for IBurnCore.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Runtime.InteropServices;
    using System.Text;

    /// <summary>
    /// Allows calls into the installation engine.
    /// </summary>
    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [Guid("6480D616-27A0-44D7-905B-81512C29C2FB")]
    public interface IBurnCore
    {
        void GetPackageCount(
            [MarshalAs(UnmanagedType.U4)] out int pcPackages
            );

        [PreserveSig]
        int GetCommandLineParameters(
            [MarshalAs(UnmanagedType.LPWStr), Out] StringBuilder psczCommandLine,
            [MarshalAs(UnmanagedType.U4)] ref int pcchCommandLine
            );

        void GetVariableNumeric(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            out long pllValue
            );

        [PreserveSig]
        int GetVariableString(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.LPWStr), Out] StringBuilder wzValue,
            [MarshalAs(UnmanagedType.U4)] ref int pcchValue
            );

        void GetVariableVersion(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.U8)] out long pqwValue
            );

        // TODO: Need to define this as a tear-off object/interface instead.
        void OpenStore(
            [MarshalAs(UnmanagedType.LPWStr)] string wzBundleId
            );

        void GetPriorVariableNumeric(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            out long pllValue
            );

        void GetPriorVariableString(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.LPWStr), Out] StringBuilder wzValue,
            [MarshalAs(UnmanagedType.U4)] ref int pcchValue
            );

        void GetPriorVariableVersion(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.U8)] out long pqwValue
            );

        void CloseStore();

        void SetVariableNumeric(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            long llValue
            );

        void SetVariableString(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.LPWStr)] string wzValue
            );

        void SetVariableVersion(
            [MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.U8)] long qwValue
            );

        [PreserveSig]
        int FormatString(
            [MarshalAs(UnmanagedType.LPWStr)] string wzIn,
            [MarshalAs(UnmanagedType.LPWStr), Out] StringBuilder wzOut,
            [MarshalAs(UnmanagedType.U4)] ref int pcchOut
            );

        [PreserveSig]
        int EscapeString(
            [MarshalAs(UnmanagedType.LPWStr)] string wzIn,
            [MarshalAs(UnmanagedType.LPWStr), Out] StringBuilder wzOut,
            [MarshalAs(UnmanagedType.U4)] ref int pcchOut
            );

        void EvaluateCondition(
            [MarshalAs(UnmanagedType.LPWStr)] string wzCondition,
            [MarshalAs(UnmanagedType.Bool)] out bool pf
            );

        void Log(
            [MarshalAs(UnmanagedType.U4)] LogLevel level,
            [MarshalAs(UnmanagedType.LPWStr)] string wzMessage
            );

        [PreserveSig]
        int Elevate(
            IntPtr hwndParent
            );

        void Detect();

        void Plan(
            [MarshalAs(UnmanagedType.U4)] LaunchAction action
            );

        void Apply(
            IntPtr hwndParent
            );

        void Suspend();

        void Reboot();

        void Shutdown(
            [MarshalAs(UnmanagedType.U4)] int dwExitCode
            );

        void SetSource(
            [MarshalAs(UnmanagedType.LPWStr)] string wzSourcePath
            );
    }

    /// <summary>
    /// The installation action for the bundle or current package.
    /// </summary>
    public enum ActionState
    {
        None,
        Uninstall,
        Install,
        AdminInstall,
        Maintenance,
        Recache,
        MinorUpgrade,
        MajorUpgrade,
        Patch,
    }

    /// <summary>
    /// The action for the UX to perform.
    /// </summary>
    public enum LaunchAction
    {
        Unknown,
        Help,
        Uninstall,
        Install,
        Modify,
        Repair,
    }

    /// <summary>
    /// The message log level.
    /// </summary>
    public enum LogLevel
    {
        /// <summary>
        /// No logging level (generic).
        /// </summary>
        None,

        /// <summary>
        /// User messages.
        /// </summary>
        Standard,

        /// <summary>
        /// Verbose messages.
        /// </summary>
        Verbose,

        /// <summary>
        /// Messages for debugging.
        /// </summary>
        Debug,

        /// <summary>
        /// Error messages.
        /// </summary>
        Error,
    }

    /// <summary>
    /// Describes the state of an installation package.
    /// </summary>
    public enum PackageState
    {
        Unknown,
        Absent,
        Cached,
        Present,
    }

    /// <summary>
    /// Indicates the state desired for an installation package.
    /// </summary>
    public enum RequestState
    {
        None,
        Absent,
        Cache,
        Present,
        Repair,
    }
}
