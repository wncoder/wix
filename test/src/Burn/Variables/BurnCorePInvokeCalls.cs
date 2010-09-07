//-----------------------------------------------------------------------
// <copyright file="BurnCorePInvokeCalls.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>
//     - Test fixture for Burn Variables feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn
{
    using System;
    using System.Runtime.InteropServices;
    using System.Text;

    # region Public Enum from IBurnCore.cpp

    public enum BURN_ACTION
    {
        BURN_ACTION_UNKNOWN,
        BURN_ACTION_HELP,
        BURN_ACTION_UNINSTALL,
        BURN_ACTION_INSTALL,
        BURN_ACTION_MODIFY,
        BURN_ACTION_REPAIR
    }

    public enum ACTION_STATE
    {
        ACTION_STATE_NONE,
        ACTION_STATE_UNINSTALL,
        ACTION_STATE_INSTALL,
        ACTION_STATE_ADMIN_INSTALL,
        ACTION_STATE_MAINTENANCE,
        ACTION_STATE_RECACHE,
        ACTION_STATE_MINOR_UPGRADE,
        ACTION_STATE_MAJOR_UPGRADE,
        ACTION_STATE_PATCH
    }

    public enum PACKAGE_STATE
    {
        PACKAGE_STATE_UNKNOWN,
        PACKAGE_STATE_ABSENT,
        PACKAGE_STATE_CACHED,
        PACKAGE_STATE_PRESENT
    }

    public enum REQUEST_STATE
    {
        REQUEST_STATE_NONE,
        REQUEST_STATE_ABSENT,
        REQUEST_STATE_CACHE,
        REQUEST_STATE_PRESENT,
        REQUEST_STATE_REPAIR
    }

    public enum BURN_LOG_LEVEL
    {
        BURN_LOG_LEVEL_NONE,      // turns off report (only valid for XXXSetLevel())
        BURN_LOG_LEVEL_STANDARD,  // written if reporting is on
        BURN_LOG_LEVEL_VERBOSE,   // written only if verbose reporting is on
        BURN_LOG_LEVEL_DEBUG,     // reporting useful when debugging code
        BURN_LOG_LEVEL_ERROR      // always gets reported, but can never be specified
    }

    #endregion

    public class BurnCorePInvokeCalls
    {
        # region BurnCore P\Invoke calls

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 GetVariableNumeric([MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            out long pllValue);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 GetVariableString([MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.LPWStr), Out] StringBuilder wzValue,
            [MarshalAs(UnmanagedType.U4)] ref int pcchValue);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 GetVariableVersion([MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
           [MarshalAs(UnmanagedType.U8)] out long pqwValue);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 SetVariableNumeric([MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            long llValue);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 SetVariableString([MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.LPWStr)] string wzValue);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 SetVariableVersion([MarshalAs(UnmanagedType.LPWStr)] string wzVariable,
            [MarshalAs(UnmanagedType.U8)] long qwValue);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 FormatString([MarshalAs(UnmanagedType.LPWStr)] string wzIn,
                                   [MarshalAs(UnmanagedType.LPWStr), Out] StringBuilder wzOut,
                                   [MarshalAs(UnmanagedType.U4)] ref int pcchOut);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 EscapeString([MarshalAs(UnmanagedType.LPWStr)] string wzIn,
                                    [MarshalAs(UnmanagedType.LPWStr), Out] StringBuilder wzOut,
                                    [MarshalAs(UnmanagedType.U4)] ref int pcchOut);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 EvaluateCondition([MarshalAs(UnmanagedType.LPWStr)] string wzCondition,
                                    [MarshalAs(UnmanagedType.Bool)] out bool pf);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 Log(BURN_LOG_LEVEL level, string wzMessage);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 Elevate(IntPtr hwndParent);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 Detect();

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 Plan(BURN_ACTION action);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 Apply(IntPtr hwndParent);

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 Suspend();

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 Reboot();

        [DllImport("TestProxyEntryPoint.dll", CharSet = CharSet.Unicode)]
        public static extern Int32 SetSource([MarshalAs(UnmanagedType.LPWStr)] string wzSourcePath);

        # endregion

    }
}
