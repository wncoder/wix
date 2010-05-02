//-----------------------------------------------------------------------
// <copyright file="BurnMetricsConstants.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Contains methods used for getting Burn Metrics data.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.BurnFileMetrics
{
    public class BurnMetricsConstants
    {
        public class BurnMetricsIds
        {
            public const int sdpStartupAppid = 8;
            public const int DP_SPInstaller_PackageName = 439;
            public const int DP_SPInstaller_PackageVersion = 457;
            public const int DP_SPInstaller_ApplicationVersion = 440;
            public const int DP_SPInstaller_OperationRequested = 437;
            public const int DP_SPInstaller_CurrentItem = 461;
            public const int DP_Setup_CurrentFlags2 = 517;
            public const int DP_Setup_ResultCode = 515;
            public const int DP_Setup_ResultDetail = 501;
            public const int DP_SPInstaller_OSFullVersion = 453;
            public const int DP_Setup_OS = 493;
            public const int DP_OSLocale = 13;
            public const int DP_SPInstaller_OperationUI = 438;
            public const int DP_SPInstaller_DisplayedLcid2 = 450;
            public const int DP_UserType = 37;
            public const int sdpIsInternal = 426; 
            public const int DP_SPInstaller_IsAdmin = 427;
            public const int DP_Setup_ChainingPackage = 494;
            public const int DP_RebootCount = 500;
            public const int DP_PerfProcFamily = 16;
            public const int DP_PerfProcFrequency = 19;
            public const int DP_PerfCpuCount = 15;
            public const int DP_PerfMemory = 3;
            public const int DP_SPInstaller_TimeToFirstWindow = 423;
            public const int DP_SPInstaller_SystemFreeDiskSpace = 424;
            public const int DP_SPInstaller_ApplicableIfTime = 454;
            public const int DP_SPInstaller_SystemRequirementCheckTime = 455;
            public const int DP_SPInstaller_InstallTime = 434;
            public const int DP_Setup_ItemStream = 514; // used to be 498;
            public const int DP_Setup_BlockerStream = 499;
            public const int DP_SPInstaller_ApplicableSKU = 441;
            public const int DP_Setup_CancelPage = 530;  // Ironman only
            public const int DP_SetupFlags = 555;
            public const int DP_Setup_PatchStream = 570;
            public const int DP_Setup_PatchType = 571;
            public const int DB_WindowsInstallerVersion = 596;
            public const int DP_Setup_Sku = 599;
            public const int DP_CurrentItemStep = 642;
        }
    }
}
