//-----------------------------------------------------------------------
// <copyright file="BurnMetricsDataPackageInfo.cs" company="Microsoft">
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
//     - Contains methods used for getting Burn Metrics data.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.BurnFileMetrics
{
    using System;

    // Common BurnMetrics data based on the package
    // All of these datapoints should be collected for every session

    public partial class BurnMetricsData
    {
        /// <summary>
        /// App Id that recorded this BurnMetrics data.  21=IronSpigot, 23=Ironman, 24=Cartman
        /// </summary>
        public class BurnMetricsAppId : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsAppId(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.sdpStartupAppid);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsAppId Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsPackageName : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsPackageName(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetStringValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_PackageName);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsPackageName Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsPackageVersion : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsPackageVersion(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetStringValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_PackageVersion);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsPackageVersion Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsApplicationVersion : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsApplicationVersion(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetStringValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_ApplicationVersion);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsApplicationVersion Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsPatchType : IBurnMetricsDataVerification
        {
            // 0 == none / not authored
            // 1 == GDR
            // 2 == LDR
            public string Value = "";

            public BurnMetricsPatchType(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_Setup_PatchType);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsPatchType Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsSku : IBurnMetricsDataVerification
        {
            // 0 == none
            // 1 == Local
            // 2 == Web
            public string Value = "";

            public BurnMetricsSku(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_Setup_Sku);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsSku Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }
    }
}
