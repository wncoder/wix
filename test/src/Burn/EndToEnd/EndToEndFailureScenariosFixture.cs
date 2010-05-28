//-----------------------------------------------------------------------
// <copyright file="EndToEndFailureScenariosFixture.cs" company="Microsoft">
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
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;

    class EndToEndFailureScenariosFixture : EndToEndBaseFixture
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
                        this.Layout.AddExe(testExeFile, i.ToString() + "-" + System.IO.Path.GetFileName(testExeFile), null, (pt.myPayloadLocation == PayloadLocation.InLocalLayout));
                        ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[i]).InstallCommandLine = " /ec 000 /log %temp%\\" + i.ToString() + "-TestExeLog.txt ";
                        ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[i]).RepairCommandLine = " /ec 0000 /log %temp%\\" + i.ToString() + "-TestExeLog.txt ";
                        ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[i]).UninstallCommandLine = " /ec 00000 /log %temp%\\" + i.ToString() + "-TestExeLog.txt ";
                        if (pt.myPayloadOutcome == PayloadOutcome.Fail)
                        {
                            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[i]).InstallCommandLine = " /ec 1603 /log %temp%\\" + i.ToString() + "-TestExeLog.txt ";
                            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[i]).RepairCommandLine = " /ec 01603 /log %temp%\\" + i.ToString() + "-TestExeLog.txt ";
                            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[i]).UninstallCommandLine = " /ec 001603 /log %temp%\\" + i.ToString() + "-TestExeLog.txt ";
                        }
                        if (this.UserTypeToUse == UserType.NormalUser)
                        {
                            this.Layout.ParameterInfo.Registration.PerMachine = "no";
                            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[i]).PerMachine= false;
                        }
                        this.Layout.ParameterInfo.Items.Items[i].IsPresent.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Exists(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.Path("%temp%\\" + i.ToString() + "-TestExeLog.txt"));
                        this.Layout.ParameterInfo.Items.Items[i].ActionTable.InstallAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.install;
                        this.Layout.ParameterInfo.Items.Items[i].ActionTable.InstallAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.install;
                        this.Layout.ParameterInfo.Items.Items[i].ActionTable.RepairAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.repair;
                        this.Layout.ParameterInfo.Items.Items[i].ActionTable.RepairAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.repair;
                        this.Layout.ParameterInfo.Items.Items[i].ActionTable.UninstallAction.IfAbsent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.uninstall;
                        this.Layout.ParameterInfo.Items.Items[i].ActionTable.UninstallAction.IfPresent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.uninstall;
                        i++;
                        break;
                    case PayloadType.MsiPerMachine:
                        string testMsiFileToUse = testMsiConditionalPass1File;
                        if (i == 1) testMsiFileToUse = testMsiConditionalPass2File;
                        this.Layout.AddMsi(testMsiFileToUse, i.ToString() + "-" + System.IO.Path.GetFileName(testMsiPerMachineFile), null, (pt.myPayloadLocation == PayloadLocation.InLocalLayout));
                        if (pt.myPayloadOutcome == PayloadOutcome.Succeed)
                        {
                            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem)this.Layout.ParameterInfo.Items.Items[i]).MSIOptions = "PASSME=1";
                        }
                        i++; 
                        break;
                    case PayloadType.Msp:
                        this.Layout.AddMsp(testMspFile, i.ToString() + "-" + System.IO.Path.GetFileName(testMspFile), null, (pt.myPayloadLocation == PayloadLocation.InLocalLayout));
                        i++; 
                        break;
                }
            }

            this.Layout.GenerateLayout();
        }

        private void InitializeMachineState()
        {
            if ((m_Payload1Type == PayloadType.MsiPerMachine) &&
                (InstallModeToUse != InstallMode.install))
            {
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.InstallMSI(testMsiPerMachineFile);
            }
            else if (m_Payload1Type == PayloadType.Msp)
            {
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.InstallMSI(testMsiPerMachineFile);
            }
            else if ((m_Payload1Type == PayloadType.Msp) &&
                (InstallModeToUse != InstallMode.install))
            {
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.InstallMSI(testMsiPerMachineFile);
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.InstallMSPOnProduct(testMspFile,
                    Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.GetMSIProductCode(testMsiPerMachineFile));
            }

        }

        public override bool TestPasses()
        {
            bool retVal = true;

            retVal &= ExitCodeMatchesExpectation(1603);
            retVal &= DhtmlLogContainsPerformerFailedMessage();
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
