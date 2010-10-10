//-----------------------------------------------------------------------
// <copyright file="EndToEndFailureScenariosFixture.cs" company="Microsoft">
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
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;

    public class EndToEndFailureScenariosFixture : EndToEndBaseFixture
    {
        protected PayloadType m_Payload1Type;
        protected PayloadType m_Payload2Type;
        protected PayloadOutcome m_Payload1Outcome;
        protected PayloadOutcome m_Payload2Outcome;
        protected PayloadLocation m_Payload1Location;
        protected PayloadLocation m_Payload2Location;

        public EndToEndFailureScenariosFixture()
            : base()
        {
        }

        public void RunTest(PayloadType payload1Type,
            PayloadOutcome payload1Outcome,
            PayloadType payload2Type,
            PayloadOutcome payload2Outcome, 
            InstallMode installMode, 
            UiMode uiMode,
            UserType userType)
        {
            m_Payload1Type = payload1Type;
            m_Payload1Outcome = payload1Outcome;
            m_Payload2Type = payload2Type;
            m_Payload2Outcome = payload2Outcome;
            m_Payload1Location = PayloadLocation.InLocalLayout;
            m_Payload2Location = PayloadLocation.InLocalLayout;
            InstallModeToUse = installMode;
            UiModeToUse = uiMode;
            UserTypeToUse = userType;

            if (m_Payload1Type == PayloadType.Msp ||
                m_Payload1Type == PayloadType.FileAndExe ||
                m_Payload1Type == PayloadType.File ||
                m_Payload2Type == PayloadType.Msp ||
                m_Payload2Type == PayloadType.FileAndExe ||
                m_Payload2Type == PayloadType.File)
            {
                throw new Exception("Fixture does not support testing that PayloadType yet");
            }

            if (InstallModeToUse != InstallMode.install)
            {
                throw new Exception("Fixture does not support testing that type of InstallMode yet");
            }

            BuildLayout();
            InitializeMachineState();
            VerifiableProperties = this.RunScenario(
                installMode,
                uiMode,
                userType);
        }

        class PayloadTypeAndLocation
        {
            public PayloadType myPayloadType;
            public PayloadOutcome myPayloadOutcome;
            public PayloadLocation myPayloadLocation;

            public PayloadTypeAndLocation(PayloadType payloadType, PayloadOutcome payloadOutcome, PayloadLocation payloadLocation)
            {
                myPayloadType = payloadType;
                myPayloadOutcome = payloadOutcome;
                myPayloadLocation = payloadLocation;
            }
        }

        private void BuildLayout()
        {
            List<PayloadTypeAndLocation> ptls = new List<PayloadTypeAndLocation>();
            ptls.Add(new PayloadTypeAndLocation(m_Payload1Type, m_Payload1Outcome, m_Payload1Location));
            ptls.Add(new PayloadTypeAndLocation(m_Payload2Type, m_Payload2Outcome, m_Payload2Location));

            int i = 0;
            foreach (PayloadTypeAndLocation pt in ptls)
            {
                switch (pt.myPayloadType)
                {
                    case PayloadType.Exe:
                        this.Layout.AddExe(testExeFile, i.ToString() + "_" + System.IO.Path.GetFileName(testExeFile), null, (pt.myPayloadLocation == PayloadLocation.InLocalLayout));
                        string exeExitCode = "0";
                        if (pt.myPayloadOutcome == PayloadOutcome.Fail) exeExitCode = "1603";
                        ((ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).InstallCommand = " /ec 00" + exeExitCode + " /log %temp%\\" + i.ToString() + "-TestExeLog.txt ";
                        ((ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).RepairCommand = " /ec 000" + exeExitCode + " /log %temp%\\" + i.ToString() + "-TestExeLog.txt ";
                        ((ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).UninstallCommand = " /ec 0000" + exeExitCode + " /log %temp%\\" + i.ToString() + "-TestExeLog.txt ";
                        this.Layout.Wix.Bundle.Chain.Packages[i].Vital = "yes";
                        if (this.UserTypeToUse == UserType.NormalUser)
                        {
                            this.Layout.Wix.Bundle.PerMachineT = false;
                            ((ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).PerMachine = "no";
                        }
                        // BUGBUG TODO: convert this to bundle.  Need to add a filesearch and condition on it.
                        //this.Layout.ParameterInfo.Items.Items[i].IsPresent.Expression = new Test.Burn.OM.ParameterInfoOM.BurnExpressions.Exists(new Test.Burn.OM.ParameterInfoOM.BurnOperands.Path("%temp%\\" + i.ToString() + "-TestExeLog.txt"));
                        i++;
                        break;
                    case PayloadType.MsiPerMachine:
                        string testMsiFileToUse = testMsiConditionalPass1File;
                        if (i == 1) testMsiFileToUse = testMsiConditionalPass2File;
                        this.Layout.AddMsi(testMsiFileToUse, i.ToString() + "_" + System.IO.Path.GetFileName(testMsiPerMachineFile), null, (pt.myPayloadLocation == PayloadLocation.InLocalLayout));
                        this.Layout.Wix.Bundle.Chain.Packages[i].Vital = "yes";
                        if (pt.myPayloadOutcome == PayloadOutcome.Succeed)
                        {
                            ((MsiPackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).MsiProperty = new MsiPropertyElement();
                            ((MsiPackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).MsiProperty.Name = "PASSME";
                            ((MsiPackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).MsiProperty.Value = "1";
                        }
                        i++; 
                        break;
                    case PayloadType.Msp:
                        this.Layout.AddMsp(testMspFile, i.ToString() + "_" + System.IO.Path.GetFileName(testMspFile), null, (pt.myPayloadLocation == PayloadLocation.InLocalLayout), null);
                        this.Layout.Wix.Bundle.Chain.Packages[i].Vital = "yes";
                        i++; 
                        break;
                }
            }

            this.Layout.BuildBundle();
        }

        private void InitializeMachineState()
        {
            if ((m_Payload1Type == PayloadType.MsiPerMachine) &&
                (InstallModeToUse != InstallMode.install))
            {
                MsiUtils.InstallMSI(testMsiPerMachineFile);
            }
            else if (m_Payload1Type == PayloadType.Msp)
            {
                MsiUtils.InstallMSI(testMsiPerMachineFile);
            }
            else if ((m_Payload1Type == PayloadType.Msp) &&
                (InstallModeToUse != InstallMode.install))
            {
                MsiUtils.InstallMSI(testMsiPerMachineFile);
                MsiUtils.InstallMSPOnProduct(testMspFile,
                    MsiUtils.GetMSIProductCode(testMsiPerMachineFile));
            }

        }

        public override bool TestPasses()
        {
            bool retVal = true;

            retVal &= ExitCodeMatchesExpectation(ErrorCodes.ERROR_INSTALL_FAILURE);
            retVal &= AllPackagesUninstalled();
            retVal &= ArpEntryDoesNotExist(); 
            retVal &= BundleCacheNotExist(this.Layout);
            retVal &= NoRunKeysSet();
            //retVal &= DhtmlLogContainsPerformerFailedMessage();
            retVal &= MetricsDataIsAccurate();

            return retVal;
        }

        private bool DhtmlLogContainsPerformerFailedMessage()
        {
            bool foundStringInAnyDhtmlLog = false;

            string logStringToVerifyExists = "";
            switch (this.m_Payload1Type)
            {
                case PayloadType.Exe:
                    logStringToVerifyExists = System.IO.Path.GetFileName(testExeFile);
                    break;
                case PayloadType.MsiPerMachine:
                    logStringToVerifyExists = System.IO.Path.GetFileName(testMsiPerMachineFile);
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

            return foundStringInAnyDhtmlLog;
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
