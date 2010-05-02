//-----------------------------------------------------------------------
// <copyright file="DownloaderFixture.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Test Fixture for Burn downloader feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Downloader
{
    using System.Diagnostics;
    using System.IO;
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
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux.SampleUx());
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

        private void BuildLayout()
        {
            // add the file to be downloaded
            this.Layout.AddExe(testExeFile, fileName, testExeUrl, false);
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[0]).InstallCommandLine = " /ec 000 /log %temp%\\TestExeLog.txt ";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[0]).RepairCommandLine = " /ec 0000 /log %temp%\\TestExeLog.txt ";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[0]).UninstallCommandLine = " /ec 00000 /log %temp%\\TestExeLog.txt ";
            this.Layout.ParameterInfo.Items.Items[0].ActionTable.InstallAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.install;
            this.Layout.ParameterInfo.Items.Items[0].ActionTable.InstallAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.install;
            this.Layout.ParameterInfo.Items.Items[0].ActionTable.RepairAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.repair;
            this.Layout.ParameterInfo.Items.Items[0].ActionTable.RepairAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.repair;
            this.Layout.ParameterInfo.Items.Items[0].ActionTable.UninstallAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.uninstall;
            this.Layout.ParameterInfo.Items.Items[0].ActionTable.UninstallAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.uninstall;

            if (UserTypeToUse == UserType.NormalUser)
            {
                this.Layout.ParameterInfo.Registration.PerMachine = "no";
                ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[0]).PerMachine = false;
            }
            this.Layout.GenerateLayout();
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

            retVal &= ExitCodeMatchesExpectation(0);
            retVal &= DownloadCacheContainsExpectedFiles();
            retVal &= PayloadCacheIsAccurate();

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

    }
}
