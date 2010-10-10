//-----------------------------------------------------------------------
// <copyright file="EndToEndBundleFixture.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>
//     - Test Fixture for Pri-0 end-to-end scenarios.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.EndToEnd
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Collections;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Variable;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;

    public class EndToEndBundleFixture : EndToEndBaseFixture
    {
        public EndToEndBundleFixture()
            : base()
        {
        }

        public void RunTest(InstallMode installMode,
            UiMode uiMode,
            UserType userType,
            PayloadLocation payloadLocation)
        {
            RunTest(installMode,
                uiMode,
                userType,
                payloadLocation,
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());
        }

        public void RunTest(InstallMode installMode,
            UiMode uiMode,
            UserType userType,
            PayloadLocation payloadLocation,
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.UxBase ux)
        {
            InstallModeToUse = installMode;
            UiModeToUse = uiMode;
            UserTypeToUse = userType;
            PayloadLocationToUse = payloadLocation;
            UxToUse = ux;

            BuildLayout();
            InitializeMachineState();
            VerifiableProperties = this.RunScenario(
                installMode,
                uiMode,
                userType);
        }

        public void RunRollbackTest(InstallMode installMode,
            UiMode uiMode,
            UserType userType,
            PayloadLocation payloadLocation,
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.UxBase ux)
        {
            RunRollbackOrStopTest(false,
                 installMode,
                 uiMode,
                 userType,
                 payloadLocation,
                 ux);
        }
        public void RunStopTest(InstallMode installMode,
            UiMode uiMode,
            UserType userType,
            PayloadLocation payloadLocation,
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.UxBase ux)
        {
            RunRollbackOrStopTest(true,
                 installMode,
                 uiMode,
                 userType,
                 payloadLocation,
                 ux);
        }

        private void RunRollbackOrStopTest(bool rollbackDisabled,
            InstallMode installMode,
            UiMode uiMode,
            UserType userType,
            PayloadLocation payloadLocation,
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.UxBase ux)
        {
            InstallModeToUse = installMode;
            UiModeToUse = uiMode;
            UserTypeToUse = userType;
            PayloadLocationToUse = payloadLocation;
            UxToUse = ux;

            CreateRollbackLayout(rollbackDisabled);
            InitializeMachineState();
            VerifiableProperties = this.RunScenario(
                installMode,
                uiMode,
                userType);
        }

        public void RunUninstallFromArpTest(
            UiMode uiMode,
            UserType userType)
        {
            InstallModeToUse = InstallMode.uninstall;
            UiModeToUse = uiMode;
            UserTypeToUse = userType;

            VerifiableProperties = this.RunUninstallFromArpScenario(
                UiModeToUse,
                UserTypeToUse);
        }

        private void BuildLayout()
        {
            this.Layout = new LayoutManager(UxToUse);

            bool includeInLayout;
            int i = 0;

            if (PayloadLocationToUse == PayloadLocation.InLocalLayout)
            {
                includeInLayout = true;
                this.Layout.Wix.Bundle.Compressed = "yes";
            }
            else
            {
                includeInLayout = false;
                this.Layout.Wix.Bundle.Compressed = "no";
            }

            // Add a variable
            VariableElement variable = new VariableElement();
            variable.Name = "Foo";
            variable.Type = VariableElement.VariableDataType.String;
            variable.Value = "Bar";
            this.Layout.Wix.Bundle.Variables.Add(variable);

            // Per-machine Exe
            AddTestExeToLayout("TestExe1.exe", 0, 0, 0, 0, includeInLayout, true);
            i++;

            // Per-user Exe
            AddTestExeToLayout("TestExe2.exe", 0, 0, 0, 0, includeInLayout, false);
            i++;

            // Per-machine MSI
            this.Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, includeInLayout);
            i++;

            // Per-user MSI
            this.Layout.AddMsi(testMsiPerUserFile, null, testMsiPerUserUrl, includeInLayout);
            this.Layout.Wix.Bundle.Chain.Packages[i].PerMachineT = false;
            this.Layout.Wix.Bundle.PerMachineT = false; // if any per-user package is included, the bundle itself is per-user
            i++;

            // Per-user MSI with external Cab
            List<LayoutManager.ExternalFile> extFiles = new List<LayoutManager.ExternalFile>();
            LayoutManager.ExternalFile extFile = new LayoutManager.ExternalFile();
            extFile.File = testMsiPerUserExtCabCabFile;
            extFile.Url = testMsiPerUserExtCabCabUrl;
            extFiles.Add(extFile);
            this.Layout.AddMsiAndExternalFiles(testMsiPerUserExtCabMsiFile, null, testMsiPerUserExtCabMsiUrl, includeInLayout, extFiles);
            this.Layout.Wix.Bundle.Chain.Packages[i].PerMachineT = false;
            this.Layout.Wix.Bundle.PerMachineT = false; // if any per-user package is included, the bundle itself is per-user
            i++;

            this.Layout.BuildBundle();
        }

        /// <summary>
        /// Create a layout to test rollback scenario. Its contains 2 msi and an exe which returns 1603 to trigger rollback
        /// </summary>
        public void CreateRollbackLayout(bool rollbackDisabled)
        {
            this.Layout = new LayoutManager(UxToUse);

            int i = 0;
            // Add per-user MSI
            this.Layout.AddMsi(testMsiPerUserFile, null, null, true);
            this.Layout.Wix.Bundle.Chain.Packages[i].PerMachineT = false;
            this.Layout.Wix.Bundle.PerMachineT = false;
            ((MsiPackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).Vital = "yes";
            i++;

            // Add per-machine MSI
            this.Layout.AddMsi(testMsiPerMachineFile, null, null, true);
            ((MsiPackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).Vital = "yes";
            i++;

            // Add exe which return 1603 at install time

            // Per-machine Exe
            this.Layout.AddExe(testExeFile, "TestExe1.exe", testExeUrl, true);
            ((ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).InstallCommand = " /ec 1603 /log %temp%\\TestExe1-TestExeLog.txt ";
            ((ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).RepairCommand = " /ec 0000 /log %temp%\\TestExe1-TestExeLog.txt ";
            ((ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).UninstallCommand = " /ec 00000 /log %temp%\\TestExe1-TestExeLog.txt ";
            ((ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).Vital = "yes";

            if (rollbackDisabled) this.Layout.Wix.Bundle.Chain.DisableRollback = "yes";

            this.Layout.BuildBundle();
        }

        /// <summary>
        /// To verify that msi payloads are installed before rollback happened because of error returned by exe payload
        /// </summary>
        /// <returns></returns>
        public bool VerifyMsiInstallBeforeRollback()
        {
            bool result = false;

            string traceCategory = "VerifyMsiInstallBeforeRollback";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            string perUserMsiName = Path.GetFileName(testMsiPerUserFile);
            string perMachineMsiName = Path.GetFileName(testMsiPerMachineFile);

            // Get the latest per-user msi log file
            string perUserMsiInstallLogFile = GetLatestLogFilePath(perUserMsiName);

            result = VerifyLogContent(perUserMsiInstallLogFile, "MainEngineThread is returning 0");

            if (!result)
            {
                Trace.WriteLine(string.Format("MSI: {0} was not installed successfully before rollback"));
                Trace.WriteLine(string.Format("Test failed to find text: {0} in log file {1}", "MainEngineThread is returning 0"
                    , perUserMsiInstallLogFile), traceCategory);

                return result;
            }
            else
            {
                Trace.WriteLine(string.Format("Test found text: {0} in log file {1}", "MainEngineThread is returning 0"
                    , perUserMsiInstallLogFile), traceCategory);
            }

            // Get the latest per-machine log file
            string perMachineMsiInstallLogFile = GetLatestLogFilePath(perMachineMsiName);

            result = VerifyLogContent(perMachineMsiInstallLogFile, "MainEngineThread is returning 0");

            if (!result)
            {
                Trace.WriteLine(string.Format("MSI: {0} was not installed successfully before rollback"), traceCategory);
                Trace.WriteLine(string.Format("Test failed to find text: {0} in log file {1}", "MainEngineThread is returning 0"
                    , perMachineMsiInstallLogFile), traceCategory);

                return result;
            }
            else
            {
                Trace.WriteLine(string.Format("Test found text: {0} in log file {1}", "MainEngineThread is returning 0"
                    , perMachineMsiInstallLogFile), traceCategory);
            }

            Trace.WriteLineIf(result, "PASS", traceCategory);
            Trace.WriteLineIf(!result, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return result;
        }

        private bool VerifyLogContent(string logFilename, string logLookupText)
        {
            string fileText = File.ReadAllText(logFilename);

            return fileText.Contains(logLookupText);
        }

        private string GetLatestLogFilePath(string logFilename)
        {
            string logFilePath = string.Empty;

            string[] files = Directory.GetFiles(Environment.ExpandEnvironmentVariables("%temp%"), string.Format("*{0}*.txt", logFilename)
                , SearchOption.AllDirectories);

            ArrayList fileCreatedDateList = new ArrayList();

            foreach (string file in files)
            {
                FileInfo fileInfo = new FileInfo(file);

                fileCreatedDateList.Add(fileInfo.CreationTime);
            }

            fileCreatedDateList.Sort();

            foreach (string file in files)
            {
                FileInfo fileInfo = new FileInfo(file);

                if (fileCreatedDateList[fileCreatedDateList.Count - 2].ToString() == fileInfo.CreationTime.ToString())
                    logFilePath = file;
            }

            return logFilePath;
        }

        /// <summary>
        /// To verify msi payloads installed before rollback are uninstalled at rollback
        /// </summary>
        /// <returns></returns>
        public bool VerifyRollbackUninstall()
        {
            bool result = true;
            string traceCategory = "VerifyRollbackUninstall";

            result &= !VerifyMsiIsInstalled(testMsiPerUserFile, traceCategory);
            result &= !VerifyMsiIsInstalled(testMsiPerMachineFile, traceCategory);

            return result;
        }

        private void InitializeMachineState()
        {
            // Nothing to do here.  Once MSPs are supported, we will need to pre-install the MSI the MSP targets
        }

        public override bool TestPasses()
        {
            return TestPasses(true);
        }


        public bool TestPasses(bool validateMetricsData)
        {
            bool retVal = true;
            string traceCategory = "Verify TestPasses";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            retVal &= ExitCodeMatchesExpectation(ErrorCodes.ERROR_SUCCESS);
            retVal &= NoRunKeysSet();
            //retVal &= DhtmlLogContainsPerformerSucceededMessage();
            if (this.InstallModeToUse == InstallMode.uninstall)
            {
                retVal &= AllPackagesUninstalled();
                retVal &= ArpEntryDoesNotExist();
                retVal &= BundleCacheNotExist(this.Layout);
                retVal &= PackageApplyOrderIsAccurate(false);
            }
            else
            {
                retVal &= AllPackagesInstalled();
                retVal &= PayloadCacheIsAccurate();
                retVal &= BundleCacheExists(this.Layout);
                retVal &= ArpEntryIsAccurate();
                retVal &= PackageApplyOrderIsAccurate(true);
            }
            if (validateMetricsData) { retVal &= MetricsDataIsAccurate(); }

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        public bool TestPassesRollbackScenario()
        {
            return TestPassesRollbackOrStopScenario(false, "TestPassesRollbackScenario");
        }
        
        public bool TestPassesStopScenario()
        {
            return TestPassesRollbackOrStopScenario(true, "TestPassesStopScenario");
        }

        public bool TestPassesRollbackOrStopScenario(bool rollbackDisabled, string verificationMethodName)
        {
            bool retVal = true;
            string traceCategory = "Verify " + verificationMethodName;

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            retVal &= ExitCodeMatchesExpectation(ErrorCodes.ERROR_INSTALL_FAILURE);
            retVal &= NoRunKeysSet();
            if (rollbackDisabled)
            {
                // If rollback is disabled, the install will just stop.
                // It will not remove the ARP entry or bundle cache so verify they still exist.
                retVal &= ArpEntryIsAccurate();
                retVal &= BundleCacheExists(this.Layout);
            }
            else
            {
                //retVal &= VerifyMsiInstallBeforeRollback();  // This needs to be refactored.  Log file names have changed and it no longer works as is.  It should be able to get that info from the engine log (but that info is not currently logged).
                retVal &= VerifyRollbackUninstall();
                retVal &= AllPackagesUninstalled();
                retVal &= ArpEntryDoesNotExist();
                retVal &= BundleCacheNotExist(this.Layout);
                //retVal &= PayloadCacheNotExist();    // Not sure what the package cache state should be in a rollback scenario?
            }

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }
    }
}
