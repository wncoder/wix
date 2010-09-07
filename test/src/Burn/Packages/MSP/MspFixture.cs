//-----------------------------------------------------------------------
// <copyright file="MspFixture.cs" company="Microsoft">
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
//     - Test fixture for Burn MSP feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.RebootResume
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;

    public class MspFixture : BurnCommonTestFixture
    {
        public enum MsiInstallState
        {
            MsiA,
            MsiAMsiB,
            MsiB
        }

        public enum MspPayloadType
        {
            MspA,
            MspAB,
            PatchesMspA,
            PatchesMspAMspB,
            PatchesMspAMspBMspAB
        }
        protected MsiInstallState m_MsiInstallState;
        protected MspPayloadType m_MspPayloadType;

        protected string testMsiAFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\PerMachineMsisMsps\prodAv100.msi");
        protected string testMsiBFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\PerMachineMsisMsps\prodBv100.msi");

        protected string testMspAFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\PerMachineMsisMsps\PatchAv101.msp"); // MSP that will target prodAv100.msi
        protected string testMspBFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\PerMachineMsisMsps\PatchBv101.msp"); // MSP that will target prodBv100.msi
        protected string testMspABFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\PerMachineMsisMsps\PatchABv101.msp"); // MSP that will target prodAv100.msi and prodBv100.msi

        public MspFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());
        }

        public void RunTest(MsiInstallState msiInstallState,
            MspPayloadType mspPayloadType,
            InstallMode installMode,
            UiMode uiMode,
            UserType userType)
        {
            m_MsiInstallState = msiInstallState;
            m_MspPayloadType = mspPayloadType;
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
            switch (m_MspPayloadType)
            {
                case MspPayloadType.MspAB:
                    this.Layout.AddMsp(testMspABFile, true);
                    break;
                default:
                    throw new Exception("That MspPayloadType is not supported yet by this test fixture");
            }

            this.Layout.BuildBundle();
        }

        private void InitializeMachineState()
        {
            if (this.m_MsiInstallState == MsiInstallState.MsiA || this.m_MsiInstallState == MsiInstallState.MsiAMsiB)
            {
                MsiUtils.InstallMSI(testMsiAFile);
            }
            if (this.m_MsiInstallState == MsiInstallState.MsiB || this.m_MsiInstallState == MsiInstallState.MsiAMsiB)
            {
                MsiUtils.InstallMSI(testMsiBFile);
            }

        }

        public override void CleanUp()
        {
            MsiUtils.RemoveMSI(testMsiAFile);
            MsiUtils.RemoveMSI(testMsiBFile);

            base.CleanUp();
        }

        public override bool TestPasses()
        {
            bool retVal = true;
            string traceCategory = "Verify TestPasses";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            retVal = true;
            retVal &= ExitCodeMatchesExpectation(ErrorCodes.ERROR_SUCCESS);
            retVal &= ItemStateMatchesExpectation();
            retVal &= PayloadCacheIsAccurate();

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }


        private bool ItemStateMatchesExpectation()
        {
            bool retVal = true;
            string traceCategory = "Verify ItemStateMatchesExpectation";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();
            Trace.WriteLine("PayloadType=" + this.m_MspPayloadType.ToString(), traceCategory);


            List<string> prodCodes = null;
            List<string> patchCodes = new List<string>();
            if (this.m_MspPayloadType == MspPayloadType.MspA ||
                this.m_MspPayloadType == MspPayloadType.PatchesMspA ||
                this.m_MspPayloadType == MspPayloadType.PatchesMspAMspB ||
                this.m_MspPayloadType == MspPayloadType.PatchesMspAMspBMspAB)
            {
                patchCodes.Add(MsiUtils.GetPatchCode(testMspAFile));
            }
            if (this.m_MspPayloadType == MspPayloadType.PatchesMspAMspB ||
                this.m_MspPayloadType == MspPayloadType.PatchesMspAMspBMspAB)
            {
                patchCodes.Add(MsiUtils.GetPatchCode(testMspBFile));
            }
            if (this.m_MspPayloadType == MspPayloadType.MspAB ||
                this.m_MspPayloadType == MspPayloadType.PatchesMspAMspBMspAB)
            {
                patchCodes.Add(MsiUtils.GetPatchCode(testMspABFile));
            }

            foreach (string patchCode in patchCodes)
            {
                if (this.m_MsiInstallState == MsiInstallState.MsiA)
                {
                    prodCodes = new List<string>();
                    prodCodes.Add(MsiUtils.GetMSIProductCode(testMsiAFile));
                    if (!MsiUtils.IsPatchInstalled(patchCode, prodCodes.ToArray()))
                    {
                        retVal = false;
                        Trace.WriteLine("MspA not applied to prodAv100.msi", traceCategory);
                    }
                }
                if (this.m_MsiInstallState == MsiInstallState.MsiB)
                {
                    prodCodes = new List<string>();
                    prodCodes.Add(MsiUtils.GetMSIProductCode(testMsiBFile));
                    if (!MsiUtils.IsPatchInstalled(patchCode, prodCodes.ToArray()))
                    {
                        retVal = false;
                        Trace.WriteLine("MspB not applied to prodBv100.msi", traceCategory);
                    }
                }
                if (this.m_MsiInstallState == MsiInstallState.MsiAMsiB)
                {
                    prodCodes = new List<string>();
                    prodCodes.Add(MsiUtils.GetMSIProductCode(testMsiAFile));
                    if (!MsiUtils.IsPatchInstalled(patchCode, prodCodes.ToArray()))
                    {
                        retVal = false;
                        Trace.WriteLine("MspAB not applied to prodAv100.msi", traceCategory);
                    }
                    prodCodes = new List<string>();
                    prodCodes.Add(MsiUtils.GetMSIProductCode(testMsiBFile));
                    if (!MsiUtils.IsPatchInstalled(patchCode, prodCodes.ToArray()))
                    {
                        retVal = false;
                        Trace.WriteLine("MspAB not applied to prodBv100.msi", traceCategory);
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
