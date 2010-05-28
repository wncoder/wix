//-----------------------------------------------------------------------
// <copyright file="BurnMetricsDataUserEnvironment.cs" company="Microsoft">
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

    // Common BurnMetrics data based on the users environment
    // All of these datapoints should be collected for every session

    public partial class BurnMetricsData
    {
        public class BurnMetricsOsFullVersion : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsOsFullVersion(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetStringValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_OSFullVersion);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsOsFullVersion Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsOS : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsOS(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_Setup_OS);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsOS Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsOSLocale : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsOSLocale(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_OSLocale);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsOSLocale Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsProcFamily : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsProcFamily(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_PerfProcFamily);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsProcFamily Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsProcFrequency : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsProcFrequency(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_PerfProcFrequency);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsProcFrequency Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsCpuCount : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsCpuCount(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_PerfCpuCount);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsCpuCount Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsMemory : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsMemory(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_PerfMemory);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsMemory Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsSystemFreeDiskSpace : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsSystemFreeDiskSpace(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_SystemFreeDiskSpace);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsSystemFreeDiskSpace Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsSetupFlags : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsSetupFlags(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SetupFlags);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsSetupFlags Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsWindowsInstallerVersion : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsWindowsInstallerVersion(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetStringValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DB_WindowsInstallerVersion);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsWindowsInstallerVersion Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }
    }
}
