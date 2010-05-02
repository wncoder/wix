//-----------------------------------------------------------------------
// <copyright file="EndToEndSuccessScenariosFixture.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
    using System.Text.RegularExpressions;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;

    class EndToEndSuccessScenariosFixture : EndToEndBaseFixture
    {
        protected PayloadType m_PayloadType;
        protected PayloadLocation m_PayloadLocation;

        public EndToEndSuccessScenariosFixture()
            : base()
        {
        }

        public void RunTest(PayloadType payloadType,
            PayloadLocation payloadLocation,
            InstallMode installMode,
            UiMode uiMode,
            UserType userType)
        {
            RunTest(payloadType,
                payloadLocation,
                installMode,
                uiMode,
                userType,
                false);
        }

        public void RunTest(PayloadType payloadType,
            PayloadLocation payloadLocation,
            InstallMode installMode,
            UiMode uiMode,
            UserType userType,
            bool useABundle)
        {
            m_PayloadType = payloadType;
            m_PayloadLocation = payloadLocation;
            InstallModeToUse = installMode;
            UiModeToUse = uiMode;
            UserTypeToUse = userType;
            UseABundle = useABundle;

            BuildLayout();
            InitializeMachineState();
            VerifiableProperties = this.RunScenario(
                installMode,
                uiMode,
                userType);
        }

        private void BuildLayout()
        {
            switch (m_PayloadType)
            {
                case PayloadType.Exe:
                    this.Layout.AddExe(testExeFile, null, testExeUrl, (m_PayloadLocation == PayloadLocation.InLocalLayout));
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[0]).InstallArguments = " /ec 000 /log %temp%\\TestExeLog.txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[0]).RepairArguments = " /ec 0000 /log %temp%\\TestExeLog.txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[0]).UninstallArguments = " /ec 00000 /log %temp%\\TestExeLog.txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[0]).InstallCommandLine = " /ec 000 /log %temp%\\TestExeLog.txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[0]).RepairCommandLine = " /ec 0000 /log %temp%\\TestExeLog.txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[0]).UninstallCommandLine = " /ec 00000 /log %temp%\\TestExeLog.txt ";
                    this.Layout.ParameterInfo.Items.Items[0].ActionTable.InstallAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.install;
                    this.Layout.ParameterInfo.Items.Items[0].ActionTable.InstallAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.install;
                    this.Layout.ParameterInfo.Items.Items[0].ActionTable.RepairAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.repair;
                    this.Layout.ParameterInfo.Items.Items[0].ActionTable.RepairAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.repair;
                    this.Layout.ParameterInfo.Items.Items[0].ActionTable.UninstallAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.uninstall;
                    this.Layout.ParameterInfo.Items.Items[0].ActionTable.UninstallAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.uninstall;
                    if (InstallModeToUse == InstallMode.uninstall ||
                        InstallModeToUse == InstallMode.repair ||
                        InstallModeToUse == InstallMode.maintenanceMode)
                    {
                        // make the Exe appear to be present so it can be repaired or uninstalled by the engine
                        this.Layout.ParameterInfo.Items.Items[0].IsPresent.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();
                    }
                    if (UserTypeToUse == UserType.NormalUser)
                    {
                        ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[0]).PerMachine = false;
                        ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[0]).PerMachine = "no";
                    }
                    break;
                case PayloadType.MsiPerMachine:
                    this.Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, (m_PayloadLocation == PayloadLocation.InLocalLayout));
                    break;
                case PayloadType.MsiPerUser:
                    this.Layout.AddMsi(testMsiPerUserFile, null, testMsiPerUserUrl, (m_PayloadLocation == PayloadLocation.InLocalLayout));
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem)this.Layout.ParameterInfo.Items.Items[0]).PerMachine = false;
                    break;
                case PayloadType.MsiPerUserExtCab:
                    List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.ExternalFile> extFiles = new List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.ExternalFile>();
                    Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.ExternalFile extFile = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.ExternalFile();
                    extFile.File = testMsiPerUserExtCabCabFile;
                    extFile.Url = testMsiPerUserExtCabCabUrl;
                    extFiles.Add(extFile);
                    this.Layout.AddMsiAndExternalFiles(testMsiPerUserExtCabMsiFile, null, testMsiPerUserExtCabMsiUrl, (m_PayloadLocation == PayloadLocation.InLocalLayout), extFiles);
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem)this.Layout.ParameterInfo.Items.Items[0]).PerMachine = false;
                    break;
                case PayloadType.Msp:
                    this.Layout.AddMsp(testMspFile, (m_PayloadLocation == PayloadLocation.InLocalLayout));
                    break;
                case PayloadType.FileAndExe:
                    // BUGBUG may need to update this.Layout.BurnManifest when files are supported by burn.exe
                    this.Layout.AddFile(testFileFile, null, testFileUrl, (m_PayloadLocation == PayloadLocation.InLocalLayout));
                    this.Layout.ParameterInfo.Items.Items[0].ActionTable.InstallAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.install;
                    this.Layout.ParameterInfo.Items.Items[0].ActionTable.InstallAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.install;
                    this.Layout.ParameterInfo.Items.Items[0].ActionTable.RepairAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.repair;
                    this.Layout.ParameterInfo.Items.Items[0].ActionTable.RepairAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.repair;
                    this.Layout.ParameterInfo.Items.Items[0].ActionTable.UninstallAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.uninstall;
                    this.Layout.ParameterInfo.Items.Items[0].ActionTable.UninstallAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.uninstall;
                    if (InstallModeToUse == InstallMode.uninstall ||
                        InstallModeToUse == InstallMode.repair ||
                        InstallModeToUse == InstallMode.maintenanceMode)
                    {
                        // make the File appear to be present so it can be repaired or uninstalled by the engine
                        this.Layout.ParameterInfo.Items.Items[0].IsPresent.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();
                    }
                    this.Layout.AddExe(testExeFile, true);
                    // BUGBUG enable this after the File item above is added to the burn manifest.  Currently File's aren't supported in the burnmanifest generator so this won't work (index is off).
                    //((Microsoft.Tools.WindowsInstallerXml.Test.Burn.BurnManifestGenerator.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[1]).InstallArguments = " /ec 000 /log %temp%\\TestExeLog.txt ";
                    //((Microsoft.Tools.WindowsInstallerXml.Test.Burn.BurnManifestGenerator.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[1]).RepairArguments = " /ec 0000 /log %temp%\\TestExeLog.txt ";
                    //((Microsoft.Tools.WindowsInstallerXml.Test.Burn.BurnManifestGenerator.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[1]).UninstallArguments = " /ec 00000 /log %temp%\\TestExeLog.txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[1]).InstallCommandLine = " /ec 000 /log %temp%\\TestExeLog.txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[1]).RepairCommandLine = " /ec 0000 /log %temp%\\TestExeLog.txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[1]).UninstallCommandLine = " /ec 00000 /log %temp%\\TestExeLog.txt ";
                    this.Layout.ParameterInfo.Items.Items[1].ActionTable.InstallAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.install;
                    this.Layout.ParameterInfo.Items.Items[1].ActionTable.InstallAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.install;
                    this.Layout.ParameterInfo.Items.Items[1].ActionTable.RepairAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.repair;
                    this.Layout.ParameterInfo.Items.Items[1].ActionTable.RepairAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.repair;
                    this.Layout.ParameterInfo.Items.Items[1].ActionTable.UninstallAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.uninstall;
                    this.Layout.ParameterInfo.Items.Items[1].ActionTable.UninstallAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.uninstall;
                    if (InstallModeToUse == InstallMode.uninstall ||
                        InstallModeToUse == InstallMode.repair ||
                        InstallModeToUse == InstallMode.maintenanceMode)
                    {
                        // make the Exe appear to be present so it can be repaired or uninstalled by the engine
                        this.Layout.ParameterInfo.Items.Items[1].IsPresent.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();
                    }

                    break;
            }

            if (UserTypeToUse == UserType.NormalUser) this.Layout.ParameterInfo.Registration.PerMachine="no";

            if (UseABundle)
            {
                this.Layout.GenerateBundle();
            }
            else
            {
                this.Layout.GenerateLayout();
            }
        }

        private void InitializeMachineState()
        {
            if ((m_PayloadType == PayloadType.MsiPerMachine) &&
                (InstallModeToUse != InstallMode.install))
            {
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.InstallMSI(testMsiPerMachineFile);
            }
            else if ((m_PayloadType == PayloadType.MsiPerUserExtCab) &&
                (InstallModeToUse != InstallMode.install))
            {
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.InstallMSI(testMsiPerUserExtCabMsiFile);
            }
            else if ((m_PayloadType == PayloadType.Msp) &&
                (InstallModeToUse == InstallMode.install))
            {
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.InstallMSI(testMsiFile);
            }
            else if ((m_PayloadType == PayloadType.Msp) &&
                (InstallModeToUse != InstallMode.install))
            {
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.InstallMSI(testMsiFile);
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.InstallMSPOnProduct(testMspFile,
                    Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.GetMSIProductCode(testMsiFile));
            }

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

            retVal &= ExitCodeMatchesExpectation(0);
            retVal &= DhtmlLogContainsPerformerSucceededMessage();
            retVal &= ItemStateMatchesExpectation();
            if (this.InstallModeToUse != InstallMode.uninstall) retVal &= PayloadCacheIsAccurate();
            if (validateMetricsData) { retVal &= MetricsDataIsAccurate(); }

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        private bool DhtmlLogContainsPerformerSucceededMessage()
        {
            bool foundStringInAnyDhtmlLog = false;
            string traceCategory = "Verify DhtmlLogContainsPerformerSucceededMessage";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            string logStringToVerifyExists = "";
            Trace.WriteLine("PayloadType=" + this.m_PayloadType.ToString(), traceCategory);
            switch (this.m_PayloadType)
            {
                case PayloadType.Exe:
                    logStringToVerifyExists = System.IO.Path.GetFileName(testExeFile);
                    break;
                case PayloadType.MsiPerMachine:
                    logStringToVerifyExists = System.IO.Path.GetFileName(testMsiPerMachineFile);
                    break;
                case PayloadType.MsiPerUser:
                    logStringToVerifyExists = System.IO.Path.GetFileName(testMsiPerUserFile);
                    break;
                case PayloadType.MsiPerUserExtCab:
                    logStringToVerifyExists = System.IO.Path.GetFileName(testMsiPerUserExtCabMsiFile);
                    break;
                case PayloadType.Msp:
                    logStringToVerifyExists = System.IO.Path.GetFileName(testMspFile);
                    break;
                case PayloadType.FileAndExe:
                    logStringToVerifyExists = System.IO.Path.GetFileName(testFileFile);
                    break;
            }

            if (this.VerifiableProperties.BurnDhtmlLogs.Count < 1)
            {
                // no logs to verify
            }
            else
            {
                foreach (BurnLog bl in this.VerifiableProperties.BurnDhtmlLogs)
                {
                    foundStringInAnyDhtmlLog |= bl.LogContainsRegExMatch(logStringToVerifyExists, System.Text.RegularExpressions.RegexOptions.None);
                }
            }
            Trace.WriteLineIf(foundStringInAnyDhtmlLog, "PASS: performer logged", traceCategory);
            Trace.WriteLineIf(!foundStringInAnyDhtmlLog, "FAIL: performer not logged", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return foundStringInAnyDhtmlLog;
        }

        private bool ItemStateMatchesExpectation()
        {
            bool retVal = false;
            string ecVal = "ec 000 ";
            string traceCategory = "Verify ItemStateMatchesExpectation";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();
            Trace.WriteLine("PayloadType=" + this.m_PayloadType.ToString(), traceCategory);

            if (this.InstallModeToUse == InstallMode.repair) ecVal = "ec 0000 ";
            if (this.InstallModeToUse == InstallMode.uninstall) ecVal = "ec 00000 ";

            switch (this.m_PayloadType)
            {
                case PayloadType.Exe:
                    retVal = FileExistsWithValidTimestampAndContents("%temp%\\TestExeLog.txt", ecVal, RegexOptions.IgnoreCase);
                    break;
                case PayloadType.MsiPerMachine:
                    if (this.InstallModeToUse == InstallMode.uninstall)
                    {
                        retVal = !Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsProductInstalled(
                        Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.GetMSIProductCode(testMsiPerMachineFile));
                    }
                    else
                    {
                        retVal = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsProductInstalled(
                            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.GetMSIProductCode(testMsiPerMachineFile));
                    }
                    break;
                case PayloadType.MsiPerUser:
                    if (this.InstallModeToUse == InstallMode.uninstall)
                    {
                        retVal = !Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsProductInstalled(
                            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.GetMSIProductCode(testMsiPerUserFile));
                    }
                    else
                    {
                        retVal = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsProductInstalled(
                            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.GetMSIProductCode(testMsiPerUserFile));
                    }
                    break;
                case PayloadType.MsiPerUserExtCab:
                    if (this.InstallModeToUse == InstallMode.uninstall)
                    {
                        retVal = !Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsProductInstalled(
                            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.GetMSIProductCode(testMsiPerUserExtCabMsiFile));
                    }
                    else
                    {
                        retVal = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsProductInstalled(
                            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.GetMSIProductCode(testMsiPerUserExtCabMsiFile));
                    }
                    break;
                case PayloadType.Msp:
                    if (this.InstallModeToUse == InstallMode.uninstall)
                    {
                        retVal = !Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsPatchInstalled(testMspFile);
                    }
                    else
                    {
                        retVal = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsPatchInstalled(testMspFile);
                    }
                    break;
                case PayloadType.FileAndExe:
                    // FILE does not install so nothing to verify with it, but check the EXE.
                    retVal = FileExistsWithValidTimestampAndContents("%temp%\\TestExeLog.txt", ecVal, RegexOptions.IgnoreCase);
                    break;
            }

            Trace.WriteLineIf(retVal, "PASS: item did install/repair/uninstall as expected", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL: item did NOT install/repair/uninstall as expected", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        private bool FileExistsWithValidTimestampAndContents(string FileName, string RegEx, RegexOptions RegexOptions)
        {
            bool retVal = false;
            string expandedFileName = System.Environment.ExpandEnvironmentVariables(FileName);

            // look in the correct %temp% folder
            if (this.UserTypeToUse == UserType.NormalUser)
            {
                string adminUsername = System.Environment.ExpandEnvironmentVariables("%USERNAME%");
                string normalUsername = BurnstubLauncher.NormalUserName;
                expandedFileName = expandedFileName.Replace(adminUsername, normalUsername);
            }

            try
            {
                DateTime lastWriteTime = File.GetLastWriteTime(expandedFileName);
                if (lastWriteTime > this.VerifiableProperties.BurnStartTime &&
                    lastWriteTime < this.VerifiableProperties.BurnEndTime)
                {
                    TextReader reader = new StreamReader(expandedFileName);
                    string logFileContent = reader.ReadToEnd();
                    Match m = Regex.Match(logFileContent, RegEx, RegexOptions);
                    if (m.Success)
                    {
                        retVal = true;
                    }
                }
                else
                {
                    Trace.WriteLine(String.Format("File: {0} lastWriteTime: {1}  BurnStartTime: {2} BurnEndTime: {3}", FileName, lastWriteTime.ToString(), this.VerifiableProperties.BurnStartTime.ToString(), this.VerifiableProperties.BurnEndTime.ToString()), "FileExistsWithValidTimestampAndContents");
                }
            }
            catch
            {
            }

            return retVal;
        }

        protected override bool MetricsDataIsAccurate()
        {
            bool metricsDataIsAccurate = true;
            string traceCategory = "Verify MetricsDataIsAccurate";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            // perform the standard verifications that should apply to all Metrics scenarios 
            metricsDataIsAccurate &= base.MetricsDataIsAccurate();

            // TODO: add additional verifications specific to this fixture's scenarios.

            Trace.WriteLineIf(metricsDataIsAccurate, "PASS ", traceCategory);
            Trace.WriteLineIf(!metricsDataIsAccurate, "FAIL ", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return metricsDataIsAccurate;
        }
    }
}
