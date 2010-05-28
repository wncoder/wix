//-----------------------------------------------------------------------
// <copyright file="BurnMetricsData.cs" company="Microsoft">
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
    using System.Collections.Generic;
    using System.IO;


    public interface IBurnMetricsDataVerification
    {
        bool IsValidValue();
    }

    /// <summary>
    /// OM containing BurnMetrics data for each of the datapoints, strings, and streams that Burn collects
    /// List of what is collected with corresponding IDs is listed here: http://mswikis/dtg/Wiki%20Pages/SQM%20Datapoints.aspx
    /// </summary>
    public partial class BurnMetricsData
    {
        public string BurnMetricsXmlFilename;

        public BurnMetricsOsFullVersion OsFullVersion;
        public BurnMetricsOS OS;
        public BurnMetricsOSLocale OSLocale;
        public BurnMetricsProcFamily ProcFamily;
        public BurnMetricsProcFrequency ProcFrequency;
        public BurnMetricsCpuCount CpuCount;
        public BurnMetricsMemory Memory;
        public BurnMetricsSystemFreeDiskSpace SystemFreeDiskSpace;
        public BurnMetricsSetupFlags SetupFlags;
        public BurnMetricsAppId AppId;
        public BurnMetricsPackageName PackageName;
        public BurnMetricsPackageVersion PackageVersion;
        public BurnMetricsApplicationVersion ApplicationVersion;
        public BurnMetricsItemStream ItemStream;

        public BurnMetricsOperationRequested OperationRequested;
        public BurnMetricsCurrentItem CurrentItem;
        public BurnMetricsCurrentFlags CurrentFlags;
        public BurnMetricsReturnCode ReturnCode;
        public BurnMetricsResultDetail ResultDetail;
        public BurnMetricsCurrentItemStep CurrentItemStep;

        public BurnMetricsOperationUI OperationUI;
        public BurnMetricsDisplayLCID DisplayLCID;
        public BurnMetricsUserType UserType;
        public BurnMetricsIsInternal IsInternal;
        public BurnMetricsIsAdmin IsAdmin;
        public BurnMetricsChainingPackage ChainingPackage;
        public BurnMetricsRebootCount RebootCount;

        public BurnMetricsTimeToFirstWindow TimeToFirstWindow;
        public BurnMetricsApplicableIfTime ApplicableIfTime;
        public BurnMetricsSystemRequirementCheckTime SystemRequirementCheckTime;
        public BurnMetricsInstallTime InstallTime;
        public BurnMetricsCancelPage CancelPage;

        public BurnMetricsBlockerStream BlockerStream;
        public BurnMetricsApplicableSKUStream ApplicableSKUStream;

        public BurnMetricsPatchStream PatchStream;
        public BurnMetricsPatchType PatchType;

        public BurnMetricsWindowsInstallerVersion WindowsInstallerVersion;
        public BurnMetricsSku Sku;

        /// <summary>
        /// OM containing BurnMetrics data for each of the datapoints, strings, and streams that IronSpigot collects.
        /// </summary>
        public BurnMetricsData()
        {
        }

        /// <summary>
        /// OM containing BurnMetrics data for each of the datapoints, strings, and streams that IronSpigot collects.
        /// </summary>
        /// <param name="BurnMetricsXmlFile">BurnMetrics Xml file to get data from</param>
        public BurnMetricsData(string BurnMetricsXmlFile)
        {
            PopulateDataFromBurnMetricsXml(BurnMetricsXmlFile);
        }

        /// <summary>
        /// OM containing BurnMetrics data for each of the datapoints, strings, and streams that IronSpigot collects.
        /// Gets Data from the newest IronSpigot BurnMetrics log in %temp% that falls within the given time range
        /// </summary>
        /// <param name="StartTime"></param>
        /// <param name="FinishTime"></param>
        public BurnMetricsData(DateTime StartTime, DateTime FinishTime)
        {
            PopulateDataFromBurnMetricsXml(StartTime, FinishTime, 1);
        }

        /// <summary>
        /// OM containing BurnMetrics data for each of the datapoints, strings, and streams that IronSpigot collects.
        /// Gets Data from the newest IronSpigot BurnMetrics log in %temp% that falls within the given time range
        /// </summary>
        /// <param name="StartTime"></param>
        /// <param name="FinishTime"></param>
        /// <param name="xNewest">x newest 1 = newest, 2 = 2nd newest, 0 = oldest, -1 = 2nd oldest, etc.</param>
        public BurnMetricsData(DateTime StartTime, DateTime FinishTime, int xNewest)
        {
            PopulateDataFromBurnMetricsXml(StartTime, FinishTime, xNewest);
        }

        /// <summary>
        /// OM containing BurnMetrics data for each of the datapoints, strings, and streams that IronSpigot collects.
        /// Gets Data from the newest IronSpigot BurnMetrics log in %temp% that falls within the given time range
        /// </summary>
        /// <param name="StartTime"></param>
        /// <param name="FinishTime"></param>
        /// <param name="xNewest">x newest 1 = newest, 2 = 2nd newest, 0 = oldest, -1 = 2nd oldest, etc.</param>
        /// <param name="pathToMetricsFolder"></param>
        public BurnMetricsData(DateTime StartTime, DateTime FinishTime, int xNewest, string pathToMetricsFolder)
        {
            PopulateDataFromBurnMetricsXml(StartTime, FinishTime, xNewest, pathToMetricsFolder);
        }

        public void PopulateDataFromBurnMetricsXml(DateTime StartTime, DateTime FinishTime, int xNewest)
        {
            string BurnMetricsXmlFile = GetLatestBurnMetricsXmlFile(StartTime, FinishTime, xNewest);

            PopulateDataFromBurnMetricsXml(BurnMetricsXmlFile);
        }

        public void PopulateDataFromBurnMetricsXml(DateTime StartTime, DateTime FinishTime, int xNewest, string pathToMetricsFile)
        {
            string BurnMetricsXmlFile = GetLatestBurnMetricsXmlFile(StartTime, FinishTime, xNewest, pathToMetricsFile);

            PopulateDataFromBurnMetricsXml(BurnMetricsXmlFile);
        }

        class BurnMetricsFileInfo : IComparable
        {
            public string Filename;
            public DateTime LastWriteTime;

            public BurnMetricsFileInfo(string filename, DateTime lastWriteTime)
            {
                Filename = filename;
                LastWriteTime = lastWriteTime;
            }

            #region IComparable Members

            public int CompareTo(object obj)
            {
                return this.LastWriteTime.CompareTo(((BurnMetricsFileInfo)obj).LastWriteTime);
            }

            #endregion
        }

        /// <summary>
        /// Gets the full path to the latest %temp%\Metrics_*.xml file that exists within the time range specified by StartTime/FinishTime
        /// </summary>
        /// <returns>full path to the latest %temp%\Metrics_*.xml</returns>
        public string GetLatestBurnMetricsXmlFile(DateTime StartTime, DateTime FinishTime, int xNewest)
        {
            return GetLatestBurnMetricsXmlFile(StartTime, FinishTime, xNewest, "%temp%");
        }

        /// <summary>
        /// Gets the full path to the latest %temp%\Metrics_*.xml file that exists within the time range specified by StartTime/FinishTime
        /// </summary>
        /// <returns>full path to the latest %temp%\Metrics_*.xml</returns>
        public string GetLatestBurnMetricsXmlFile(DateTime StartTime, DateTime FinishTime, int xNewest, string pathToMetricsFile)
        {
            string retVal = "";
            string[] burnMetricsFiles = Directory.GetFiles(Environment.ExpandEnvironmentVariables(pathToMetricsFile), "Metrics_*.xml");
            List<BurnMetricsFileInfo> BurnMetricsFilesAndTimes = new List<BurnMetricsFileInfo>();

            foreach (string file in burnMetricsFiles)
            {
                if ((Directory.GetLastWriteTime(file) > StartTime) && (Directory.GetLastWriteTime(file) < FinishTime))
                {
                    BurnMetricsFilesAndTimes.Add(new BurnMetricsFileInfo(file, Directory.GetLastWriteTime(file)));
                }
            }

            BurnMetricsFilesAndTimes.Sort();

            if (BurnMetricsFilesAndTimes.Count > 0)
            {
                if (xNewest > 0)
                {
                    retVal = BurnMetricsFilesAndTimes[BurnMetricsFilesAndTimes.Count - xNewest].Filename;
                }
                else
                {
                    // negative means use the oldest and work back from there. 0 = oldest, -1 = 2nd oldest, etc.
                    retVal = BurnMetricsFilesAndTimes[0 - xNewest].Filename;
                }
            }
            return retVal;
        }

        public void PopulateDataFromBurnMetricsXml(string BurnMetricsXmlFile)
        {
            BurnMetricsXmlFilename = BurnMetricsXmlFile;
            OsFullVersion = new BurnMetricsOsFullVersion(BurnMetricsXmlFile);
            OS = new BurnMetricsOS(BurnMetricsXmlFile);
            OSLocale = new BurnMetricsOSLocale(BurnMetricsXmlFile);
            ProcFamily = new BurnMetricsProcFamily(BurnMetricsXmlFile);
            ProcFrequency = new BurnMetricsProcFrequency(BurnMetricsXmlFile);
            CpuCount = new BurnMetricsCpuCount(BurnMetricsXmlFile);
            Memory = new BurnMetricsMemory(BurnMetricsXmlFile);
            SystemFreeDiskSpace = new BurnMetricsSystemFreeDiskSpace(BurnMetricsXmlFile);
            SetupFlags = new BurnMetricsSetupFlags(BurnMetricsXmlFile);
            AppId = new BurnMetricsAppId(BurnMetricsXmlFile);
            PackageName = new BurnMetricsPackageName(BurnMetricsXmlFile);
            PackageVersion = new BurnMetricsPackageVersion(BurnMetricsXmlFile);
            ApplicationVersion = new BurnMetricsApplicationVersion(BurnMetricsXmlFile);
            ItemStream = new BurnMetricsItemStream(BurnMetricsXmlFile);
            OperationRequested = new BurnMetricsOperationRequested(BurnMetricsXmlFile);
            CurrentItem = new BurnMetricsCurrentItem(BurnMetricsXmlFile);
            CurrentFlags = new BurnMetricsCurrentFlags(BurnMetricsXmlFile);
            ReturnCode = new BurnMetricsReturnCode(BurnMetricsXmlFile);
            ResultDetail = new BurnMetricsResultDetail(BurnMetricsXmlFile);
            CurrentItemStep = new BurnMetricsCurrentItemStep(BurnMetricsXmlFile);
            OperationUI = new BurnMetricsOperationUI(BurnMetricsXmlFile);
            DisplayLCID = new BurnMetricsDisplayLCID(BurnMetricsXmlFile);
            UserType = new BurnMetricsUserType(BurnMetricsXmlFile);
            IsInternal = new BurnMetricsIsInternal(BurnMetricsXmlFile);
            IsAdmin = new BurnMetricsIsAdmin(BurnMetricsXmlFile);
            ChainingPackage = new BurnMetricsChainingPackage(BurnMetricsXmlFile);
            RebootCount = new BurnMetricsRebootCount(BurnMetricsXmlFile);
            TimeToFirstWindow = new BurnMetricsTimeToFirstWindow(BurnMetricsXmlFile);
            ApplicableIfTime = new BurnMetricsApplicableIfTime(BurnMetricsXmlFile);
            SystemRequirementCheckTime = new BurnMetricsSystemRequirementCheckTime(BurnMetricsXmlFile);
            InstallTime = new BurnMetricsInstallTime(BurnMetricsXmlFile);
            CancelPage = new BurnMetricsCancelPage(BurnMetricsXmlFile);
            BlockerStream = new BurnMetricsBlockerStream(BurnMetricsXmlFile);
            ApplicableSKUStream = new BurnMetricsApplicableSKUStream(BurnMetricsXmlFile);
            PatchStream = new BurnMetricsPatchStream(BurnMetricsXmlFile);
            PatchType = new BurnMetricsPatchType(BurnMetricsXmlFile);
            WindowsInstallerVersion = new BurnMetricsWindowsInstallerVersion(BurnMetricsXmlFile);
            Sku = new BurnMetricsSku(BurnMetricsXmlFile);
        }

        public bool ContainsExpectedDatapoint(string DatapointName)
        {
            bool retVal = true;

            switch (DatapointName)
            {
                case "AppId":
                    if (!this.AppId.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "ApplicableIfTime":
                    if (!this.ApplicableIfTime.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "ApplicableSKUStream":
                    if (!this.ApplicableSKUStream.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "ApplicationVersion":
                    if (!this.ApplicationVersion.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "BlockerStream":
                    if (!this.BlockerStream.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "ChainingPackage":
                    if (!this.ChainingPackage.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "CpuCount":
                    if (!this.CpuCount.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "CurrentFlags":
                    if (!this.CurrentFlags.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "CurrentItem":
                    if (!this.CurrentItem.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "DisplayLCID":
                    if (!this.DisplayLCID.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "InstallTime":
                    if (!this.InstallTime.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "IsAdmin":
                    if (!this.IsAdmin.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "ItemStream":
                    if (!this.ItemStream.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "Memory":
                    if (!this.Memory.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "OperationRequested":
                    if (!this.OperationRequested.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "OperationUI":
                    if (!this.OperationUI.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "OS":
                    if (!this.OS.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "OsFullVersion":
                    if (!this.OsFullVersion.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "OSLocale":
                    if (!this.OSLocale.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "PackageName":
                    if (!this.PackageName.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "PackageVersion":
                    if (!this.PackageVersion.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "ProcFamily":
                    if (!this.ProcFamily.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "ProcFrequency":
                    if (!this.ProcFrequency.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "RebootCount":
                    if (!this.RebootCount.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "ResultDetail":
                    if (!this.ResultDetail.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "ReturnCode":
                    if (!this.ReturnCode.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "SystemFreeDiskSpace":
                    if (!this.SystemFreeDiskSpace.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "SystemRequirementCheckTime":
                    if (!this.SystemRequirementCheckTime.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "TimeToFirstWindow":
                    if (!this.TimeToFirstWindow.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;
                case "UserType":
                    if (!this.UserType.IsValidValue())
                    {
                        retVal = false;
                    }
                    break;

                default:
                    Console.WriteLine("Unknown datapoint.  Could not verify '" + DatapointName + "'");
                    retVal = false;
                    break;
            }


            return retVal;
        }

        public bool ContainsExpectedDatapoints(List<string> expectedDatapoints)
        {
            bool retVal = true;

            foreach (string dp in expectedDatapoints)
            {
                if (!ContainsExpectedDatapoint(dp))
                {
                    Console.WriteLine("Expected datapoint is missing or contains invalid data: {0}", dp);
                    retVal = false;
                }
            }

            return retVal;
        }

        public bool NotContainsUnexpectedDatapoints(List<string> unexpectedDatapoints)
        {
            bool retVal = true;

            foreach (string dp in unexpectedDatapoints)
            {
                if (ContainsExpectedDatapoint(dp))
                {
                    Console.WriteLine("Unexpected datapoint is present: {0}", dp);
                    retVal = false;
                }
            }

            return retVal;
        }
    }

}
