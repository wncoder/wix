//-----------------------------------------------------------------------
// <copyright file="EndToEndSuccessScenariosFixture.cs" company="Microsoft">
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
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;

    public class EndToEndSuccessScenariosFixture : EndToEndBaseFixture
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
            m_PayloadType = payloadType;
            m_PayloadLocation = payloadLocation;
            InstallModeToUse = installMode;
            UiModeToUse = uiMode;
            UserTypeToUse = userType;

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
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[0]).InstallCommand = " /ec 000 /log %temp%\\TestExeLog.txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[0]).RepairCommand = " /ec 0000 /log %temp%\\TestExeLog.txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[0]).UninstallCommand = " /ec 00000 /log %temp%\\TestExeLog.txt ";
                    if (InstallModeToUse == InstallMode.uninstall ||
                        InstallModeToUse == InstallMode.repair ||
                        InstallModeToUse == InstallMode.maintenanceMode)
                    {
                        // BUGBUG TODO author a real detection mechanism (i.e. a reg key) and set/unset it appropriately to make it look like the Exe is installed or not.  Currently this isn't possible in Wix.
                        // Probably should update TestExe.exe to be able to create reg keys like this when they are run.  It can already do files, maybe use files...
                        ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[0]).DetectCondition = "1=1";
                    }
                    if (UserTypeToUse == UserType.NormalUser)
                    {
                        ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[0]).PerMachine = "no";
                        this.Layout.Wix.Bundle.Chain.Packages[0].PerMachineT = false;
                    }
                    break;
                case PayloadType.MsiPerMachine:
                    this.Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, (m_PayloadLocation == PayloadLocation.InLocalLayout));
                    break;
                case PayloadType.MsiPerUser:
                    this.Layout.AddMsi(testMsiPerUserFile, null, testMsiPerUserUrl, (m_PayloadLocation == PayloadLocation.InLocalLayout));
                    this.Layout.Wix.Bundle.Chain.Packages[0].PerMachineT = false;
                    this.Layout.Wix.Bundle.PerMachineT = false;
                    break;
                case PayloadType.MsiPerUserExtCab:
                    List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.ExternalFile> extFiles = new List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.ExternalFile>();
                    Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.ExternalFile extFile = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.ExternalFile();
                    extFile.File = testMsiPerUserExtCabCabFile;
                    extFile.Url = testMsiPerUserExtCabCabUrl;
                    extFiles.Add(extFile);
                    this.Layout.AddMsiAndExternalFiles(testMsiPerUserExtCabMsiFile, null, testMsiPerUserExtCabMsiUrl, (m_PayloadLocation == PayloadLocation.InLocalLayout), extFiles);
                    this.Layout.Wix.Bundle.Chain.Packages[0].PerMachineT = false;
                    this.Layout.Wix.Bundle.PerMachineT = false;
                    break;
                case PayloadType.Msp:
                    this.Layout.AddMsp(testMspFile, (m_PayloadLocation == PayloadLocation.InLocalLayout));
                    break;

                default:
                    throw new Exception("ERROR: Unsupported PayloadType.  This test fixture doesn't support PayloadType." + m_PayloadType.ToString());
            }

            if (UserTypeToUse == UserType.NormalUser)
            {
                //this.Layout.ExpectPerMachineBundle = false;
                this.Layout.Wix.Bundle.PerMachineT = false;
            }

            this.Layout.BuildBundle();
        }

        private void InitializeMachineState()
        {
            if ((m_PayloadType == PayloadType.MsiPerMachine) &&
                (InstallModeToUse != InstallMode.install))
            {
                MsiUtils.InstallMSI(testMsiPerMachineFile);
            }
            else if ((m_PayloadType == PayloadType.MsiPerUserExtCab) &&
                (InstallModeToUse != InstallMode.install))
            {
                MsiUtils.InstallMSI(testMsiPerUserExtCabMsiFile);
            }
            else if ((m_PayloadType == PayloadType.Msp) &&
                (InstallModeToUse == InstallMode.install))
            {
                MsiUtils.InstallMSI(testMsiFile);
            }
            else if ((m_PayloadType == PayloadType.Msp) &&
                (InstallModeToUse != InstallMode.install))
            {
                MsiUtils.InstallMSI(testMsiFile);
                MsiUtils.InstallMSPOnProduct(testMspFile,
                    MsiUtils.GetMSIProductCode(testMsiFile));
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

            retVal &= ExitCodeMatchesExpectation(ErrorCodes.ERROR_SUCCESS);
            //retVal &= DhtmlLogContainsPerformerSucceededMessage();  // turning off this validation for now since DHTML logging is disabled
            retVal &= ItemStateMatchesExpectation();
            if (this.InstallModeToUse != InstallMode.uninstall)
            {
                retVal &= PayloadCacheIsAccurate();
                retVal &= BundleCacheExists(this.Layout);
            }
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

            if (this.VerifiableProperties.BurnLogFiles.Count < 1)
            {
                // no logs to verify
            }
            else
            {
                foreach (BurnLog bl in this.VerifiableProperties.BurnLogFiles)
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
                        retVal = !MsiUtils.IsProductInstalled(
                        MsiUtils.GetMSIProductCode(testMsiPerMachineFile));
                    }
                    else
                    {
                        retVal = MsiUtils.IsProductInstalled(
                            MsiUtils.GetMSIProductCode(testMsiPerMachineFile));
                    }
                    break;
                case PayloadType.MsiPerUser:
                    if (this.InstallModeToUse == InstallMode.uninstall)
                    {
                        retVal = !MsiUtils.IsProductInstalled(
                            MsiUtils.GetMSIProductCode(testMsiPerUserFile));
                    }
                    else
                    {
                        retVal = MsiUtils.IsProductInstalled(
                            MsiUtils.GetMSIProductCode(testMsiPerUserFile));
                    }
                    break;
                case PayloadType.MsiPerUserExtCab:
                    if (this.InstallModeToUse == InstallMode.uninstall)
                    {
                        retVal = !MsiUtils.IsProductInstalled(
                            MsiUtils.GetMSIProductCode(testMsiPerUserExtCabMsiFile));
                    }
                    else
                    {
                        retVal = MsiUtils.IsProductInstalled(
                            MsiUtils.GetMSIProductCode(testMsiPerUserExtCabMsiFile));
                    }
                    break;
                case PayloadType.Msp:
                    if (this.InstallModeToUse == InstallMode.uninstall)
                    {
                        retVal = !MsiUtils.IsPatchInstalled(testMspFile);
                    }
                    else
                    {
                        retVal = MsiUtils.IsPatchInstalled(testMspFile);
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
