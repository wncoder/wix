//-----------------------------------------------------------------------
// <copyright file="DownloaderFixture.cs" company="Microsoft">
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
//     - Test Fixture for Burn downloader feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Downloader
{
    using System.Diagnostics;
    using System.IO;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain;
    using Microsoft.Tools.WindowsInstallerXml.Test.BurnFileMetrics;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;

    /// <summary>
    /// Fixture for testing download scenarios
    /// </summary>
    public class DownloaderFixture : BurnCommonTestFixture
    {
        public enum Scenario
        {
            DownloadSucceeds,
            CancelWhileDownloading,
            DownloadAndRetriesFailBadUrl,
            DownloadAndRetriesFailBadSignature,
            DownloadFailsAndRetrySucceeds
        }

        public enum DownloadProtocol
        {
            Bits,
            WinHttp,
            UrlMon,
            undefined
        }

        public enum IgnoreDownloadFailure
        {
            False,
            True,
            undefined
        }

        public enum SerialDownloadConfiguration
        {
            False,
            True,
            undefined
        }

        public enum SerialDownloadSwitch
        {
            False,
            True,
            undefined
        }

        public enum DownloadCacheState
        {
            FilesExist,
            FilesNotExist
        }

        public enum ItemNames
        {
            FileExt,
            PathFileExt,
            ManyPathFileExt
        }


        private Scenario m_Scenario;
        private DownloadProtocol m_DownloadProtocol;
        private IgnoreDownloadFailure m_IgnoreDownloadFailure;
        private SerialDownloadConfiguration m_SerialDownloadConfiguration;
        private SerialDownloadSwitch m_SerialDownloadSwitch;
        private DownloadCacheState m_DownloadCacheState;
        private ItemNames m_ItemNames;

        private string fileName = "TestFile.exe";

        public DownloaderFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());
        }

        public void RunTest(Scenario scenario,
            DownloadProtocol protocol,
            IgnoreDownloadFailure ignoreDlFailure,
            SerialDownloadConfiguration serialDlConfiguration,
            SerialDownloadSwitch serialDlSwitch,
            DownloadCacheState dlCacheState,
            ItemNames itemNames,
            UserType userType
            )
        {
            m_Scenario = scenario;
            m_DownloadProtocol = protocol;
            m_IgnoreDownloadFailure = ignoreDlFailure;
            m_SerialDownloadConfiguration = serialDlConfiguration;
            m_SerialDownloadSwitch = serialDlSwitch;
            m_DownloadCacheState = dlCacheState;
            m_ItemNames = itemNames;
            UserTypeToUse = userType;

            BuildLayout();
            InitializeMachineState();
            VerifiableProperties = this.RunScenario(
                InstallMode.install,
                UiMode.Passive,
                userType);
        }

        public void RunCreatelayoutTest(
            ItemNames itemNames,
            UserType userType)
        {
            m_ItemNames = itemNames;
            UserTypeToUse = userType;

            BuildLayout();
            InitializeMachineState();
            VerifiableProperties = this.RunScenario( InstallMode.createLayout,
                UiMode.Passive,
                UserTypeToUse);
        }

        private void BuildLayout()
        {
            this.Layout.Wix.Bundle.Compressed = "no";

            // add the file to be downloaded
            this.Layout.AddExe(testExeFile, fileName, testExeUrl, false);
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.PackagesArray[0]).InstallCommand = " /ec 000 /log %temp%\\TestExeLog.txt ";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.PackagesArray[0]).RepairCommand = " /ec 0000 /log %temp%\\TestExeLog.txt ";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.PackagesArray[0]).UninstallCommand = " /ec 00000 /log %temp%\\TestExeLog.txt ";

            if (UserTypeToUse == UserType.NormalUser)
            {
                ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.PackagesArray[0]).PerMachine = "no";
                this.Layout.Wix.Bundle.Chain.PackagesArray[0].PerMachineT = false;
                this.Layout.Wix.Bundle.PerMachineT = false;
            }
            this.Layout.BuildBundle();
        }

        private void InitializeMachineState()
        {
            if (m_DownloadCacheState == DownloadCacheState.FilesExist)
            {
                string dest = Path.Combine(this.Layout.GetDownloadCachePath(), fileName);

                Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.CopyFile(testFileFile, dest);
            }

            switch (this.m_DownloadProtocol)
            {
                case DownloadProtocol.Bits:
                    System.Environment.SetEnvironmentVariable("BurnDownloadProtocol", "bits");
                    break;
                case DownloadProtocol.WinHttp:
                    System.Environment.SetEnvironmentVariable("BurnDownloadProtocol", "http");
                    break;
                case DownloadProtocol.UrlMon:
                    System.Environment.SetEnvironmentVariable("BurnDownloadProtocol", "urlmon");
                    break;
                case DownloadProtocol.undefined:
                    System.Environment.SetEnvironmentVariable("BurnDownloadProtocol", "");
                    break;
                default:
                    System.Environment.SetEnvironmentVariable("BurnDownloadProtocol", "");
                    break;
            }
        }

        /// <summary>
        /// Each test fixture that inherits from the CommonTestFixture should override this
        /// TestPasses method to make it determine if the test passed or failed.
        /// </summary>
        /// <returns>true if the test passes, false otherwise</returns>
        public override bool TestPasses()
        {
            bool retVal = true;
            string traceCategory = "Verify TestPasses";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            retVal &= ExitCodeMatchesExpectation(ErrorCodes.ERROR_SUCCESS);
            retVal &= DownloadCacheContainsExpectedFiles();
            retVal &= PayloadCacheIsAccurate();
            retVal &= BundleCacheExists(this.Layout);

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);
            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        /// <summary>
        /// Verifies the extracted folder contains the expected packages
        /// </summary>
        /// <returns>true if the test passes, false otherwise</returns>
        public bool TestPassesCreateLayout()
        {
            bool retVal = true;
            string traceCategory = "Verify TestPasses";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            retVal &= ExitCodeMatchesExpectation(ErrorCodes.ERROR_SUCCESS);
            retVal &= DownloadCacheContainsExpectedFiles();
            retVal &= PayloadCacheNotExist();
            retVal &= BundleCacheNotExist(this.Layout);
            retVal &= ExtractedLayoutFolderExists();

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);
            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        private bool DownloadCacheContainsExpectedFiles()
        {
            bool retVal = true;
            string traceCategory = "Verify DownloadCacheContainsExpectedFiles";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            string downloadCachePath = this.Layout.GetDownloadCachePath();
            if (UserTypeToUse == UserType.NormalUser)
            {
                downloadCachePath = this.Layout.GetDownloadCachePath(BurnstubLauncher.NormalUserName);
            }

            // No files should exist, they should be moved to the payload cache before being run
            // make sure no extra files exist
            bool foundExtraFile = false;
            if (Directory.Exists(downloadCachePath))
            {
                foreach (string file in Directory.GetFiles(downloadCachePath))
                {
                    foundExtraFile = true;
                    Trace.WriteLine("Error: unexpected file found in DL cache: " + file, traceCategory);
                }
            }
            retVal &= !foundExtraFile;

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        private bool ExtractedLayoutFolderExists()
        {
            bool retVal = true;
            string traceCategory = "Verify ExtractedLayoutFolderExists";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            foreach (Package package in this.Layout.Wix.Bundle.Chain.Packages)
            {
                string extractedPackageFile = Path.Combine(this.testExtractedLayoutFolder, package.Name);
                if (File.Exists(extractedPackageFile))
                {
                    Trace.WriteLine("File exists: " + extractedPackageFile, traceCategory);
                }
                else
                {
                    Trace.WriteLine("ERROR - File not exist: " + extractedPackageFile, traceCategory);
                    retVal = false;
                }
                foreach (PayloadElement subFile in package.Payloads)
                {
                    string extractedPackageSubFile = Path.Combine(this.testExtractedLayoutFolder, subFile.Name);
                    if (File.Exists(extractedPackageSubFile))
                    {
                        Trace.WriteLine("File exists: " + extractedPackageSubFile, traceCategory);
                    }
                    else
                    {
                        Trace.WriteLine("ERROR - File not exist: " + extractedPackageSubFile, traceCategory);
                        retVal = false;
                    }
                }

            }

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }
    }
}
