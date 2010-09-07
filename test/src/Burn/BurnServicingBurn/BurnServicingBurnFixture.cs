//-----------------------------------------------------------------------
// <copyright file="BurnServicingBurnFixture.cs" company="Microsoft">
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
//     - Test Fixture for Burn servicing Burn scenarios.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.BurnServicingBurn
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;

    public class BurnServicingBurnFixture : BurnCommonTestFixture
    {
        protected BundleType m_BundleType;
        protected PayloadOutcome m_PayloadOutcome;
        protected UpgradeUpdateType m_UpgradeUpdateType;
        protected TestMode m_Mode;

        private int exePayloadExitCode;

        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager LayoutA_v2_FamA;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager LayoutB_v2_FamA;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager LayoutC_v2_FamB;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager LayoutD_v4_FamC;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager LayoutE_v4_FamD;
        private List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager> myStaticLayouts = new List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager>();
        
        public BurnServicingBurnFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());
        }

        public enum UpgradeUpdateType
        {
            /// <summary>
            /// v1 FamilyABundles, no Upgrade or Update data specified, same family but lower version causes it to not install
            /// </summary>
            LayoutBv1,
            /// <summary>
            /// v3 FamilyABundles, no Upgrade or Update data specified, but same family causes it to replace lower versions of same family
            /// </summary>
            LayoutBv3a,
            /// <summary>
            /// v3 FamilyReplacementBundles, Upgrade data specified to replace both FamilyABundles and FamilyBBundles
            /// </summary>
            LayoutBv3b_upgrade,
            /// <summary>
            /// v5 FamilyReplacementBundles, Upgrade data specified to replace both FamilyABundles and FamilyBBundles
            /// </summary>
            LayoutBv5d_upgrade,
            /// <summary>
            /// v3 FamilyUpdateBundles, Update data specified to update LayoutA
            /// </summary>
            UpdateV3_A,
            /// <summary>
            /// v5 FamilyUpdateBundles, Update data specified to update LayoutA, LayoutB, and notExist
            /// </summary>
            UpdateV3_A_B,
            /// <summary>
            /// v5 FamilyUpdateBundles, Update data specified to update LayoutA, LayoutD, and notExist (update cross families)
            /// </summary>
            UpdateV3_A_D_notExist,
            /// <summary>
            /// v5 FamilyUpdateBundles, Update data specified to update notExist (update doesn't apply to anything)
            /// </summary>
            UpdateV5_noTargetExists
        }

        public enum TestMode
        {
            Normal,
            UninstallUpdateTarget,
            Repair
        }

        public override void CleanUp()
        {
            // remove all the update/upgrade-able static bundles
            foreach (Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager myLayout in myStaticLayouts)
            {
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.EngineCleanup ec = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.EngineCleanup(myLayout);
                ec.CleanupEverything();
            }

            base.CleanUp();
        }

        public void RunTest(BundleType bundleType,
            PayloadOutcome payloadOutcome,
            UpgradeUpdateType upgradeUpdateType,
            InstallMode installMode,
            UiMode uiMode,
            UserType userType)
        {
            RunTest(bundleType,
                payloadOutcome,
                upgradeUpdateType,
                installMode,
                uiMode,
                userType,
                TestMode.Normal);
        }

        public void RunTest(BundleType bundleType,
            PayloadOutcome payloadOutcome,
            UpgradeUpdateType upgradeUpdateType,
            InstallMode installMode,
            UiMode uiMode,
            UserType userType,
            TestMode mode)
        {
            m_BundleType = bundleType;
            m_PayloadOutcome = payloadOutcome;
            m_UpgradeUpdateType = upgradeUpdateType;
            InstallModeToUse = installMode;
            UiModeToUse = uiMode;
            UserTypeToUse = userType;
            m_Mode = mode;

            exePayloadExitCode = 0;
            if (this.m_PayloadOutcome == PayloadOutcome.Fail) exePayloadExitCode = 1603;

            BuildStaticLayouts();
            BuildLayout();
            InitializeMachineState();
            if (InstallModeToUse == InstallMode.install)
            {
                VerifiableProperties = this.RunScenario(
                    installMode,
                    uiMode,
                    userType);
            }
            else if (InstallModeToUse == InstallMode.repair)
            {
                // pre-install the bundle
                VerifiableProperties = this.RunScenario(
                    InstallMode.install,
                    uiMode,
                    userType);
                // TODO verify it is applied as expected???

                // install another product that the bundle targets
                BurnstubLauncher burnstubLauncher = new BurnstubLauncher();
                if (UserTypeToUse == UserType.NormalUser)
                {
                    burnstubLauncher.UseNormalUser = true;
                }

                System.Diagnostics.Process burnProcess = burnstubLauncher.StartProcess(LayoutD_v4_FamC, " /q");
                burnProcess.WaitForExit();

                // repair the bundle
                VerifiableProperties = this.RunScenario(
                    InstallModeToUse,
                    uiMode,
                    userType);
            }
            else if (InstallModeToUse == InstallMode.uninstall && mode == TestMode.Normal)
            {
                // pre-install the bundle
                VerifiableProperties = this.RunScenario(
                    InstallMode.install,
                    uiMode,
                    userType);
                // TODO verify it is applied as expected???

                // uninstall the bundle
                VerifiableProperties = this.RunScenario(
                    InstallModeToUse,
                    uiMode,
                    userType);
            }
            else if (InstallModeToUse == InstallMode.uninstall && mode == TestMode.UninstallUpdateTarget)
            {
                // pre-install the bundle that will update targets
                VerifiableProperties = this.RunScenario(
                    InstallMode.install,
                    uiMode,
                    userType);
                // TODO verify it is applied as expected???

                // uninstall a target that was updated
                VerifiableProperties = this.RunScenario(
                    LayoutA_v2_FamA,
                    InstallModeToUse,
                    uiMode,
                    userType);
            }
            else
            {
                throw new Exception("Fixture does not support specified InstallMode.");
            }
        }

        private void BuildStaticLayouts()
        {
            LayoutA_v2_FamA = GenerateBundle("LayoutAv2", "2.0.0.0", "{AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA}");
            LayoutB_v2_FamA = GenerateBundle("LayoutBv2", "2.0.0.0", "{AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA}");
            LayoutC_v2_FamB = GenerateBundle("LayoutCv2", "2.0.0.0", "{BBBBBBBB-BBBB-BBBB-BBBB-BBBBBBBBBBBB}");
            LayoutD_v4_FamC = GenerateBundle("LayoutDv4", "4.0.0.0", "{CCCCCCCC-CCCC-CCCC-CCCC-CCCCCCCCCCCC}");
            LayoutE_v4_FamD = GenerateBundle("LayoutEv5", "4.0.0.0", "{DDDDDDDD-DDDD-DDDD-DDDD-DDDDDDDDDDDD}");

            myStaticLayouts.Add(LayoutA_v2_FamA);
            myStaticLayouts.Add(LayoutB_v2_FamA);
            myStaticLayouts.Add(LayoutC_v2_FamB);
            myStaticLayouts.Add(LayoutD_v4_FamC);
            myStaticLayouts.Add(LayoutE_v4_FamD);
        }

        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager GenerateBundle(
            string bundleName,
            string bundleVersion,
            string bundleFamily)
        {
            string bundleFilename = bundleName + ".exe";
            string yesOrNo = "yes";
            if (m_BundleType == BundleType.PerUser) yesOrNo = "no";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager myLayout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());
            myLayout.AddExe(testExeFile, true);
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)myLayout.Wix.Bundle.Chain.Packages[0]).PerMachine = yesOrNo;
            myLayout.Wix.Bundle.Name = myLayout.Wix.Bundle.Name + " " + bundleName + " v" + bundleVersion + " " + bundleFamily;
            myLayout.Wix.Bundle.Version = bundleVersion;
            myLayout.Wix.Bundle.UpgradeCode = bundleFamily;
            myLayout.SetupBundleFilename = bundleFilename;
            myLayout.BuildBundle(false);
            
            return myLayout;
        }

        private void BuildLayout()
        {
            int i = 0;

            this.Layout.AddExe(testExeFile, i.ToString() + "_" + System.IO.Path.GetFileName(testExeFile), null, true);
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).InstallCommand = "/s 1000 /ec " + exePayloadExitCode + " /log %temp%\\TestExeLog_" + i.ToString() + ".txt ";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).RepairCommand = "/s 1000 /ec " + exePayloadExitCode + " /log %temp%\\TestExeLog_" + i.ToString() + ".txt ";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[i]).UninstallCommand = "/s 1000 /ec " + exePayloadExitCode + " /log %temp%\\TestExeLog_" + i.ToString() + ".txt ";

            if (m_BundleType == BundleType.PerUser) 
            {
                ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain.ExePackageElement)this.Layout.Wix.Bundle.Chain.Packages[0]).PerMachine = "no";
            }

            string bundleName = this.m_UpgradeUpdateType.ToString();
            string bundleVersion = "";
            string bundleFamily = "";

            if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv1)
            {
                bundleVersion = "1.0.0.0";
                bundleFamily = "{AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA}";
            }
            if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv3a)
            {
                bundleVersion = "3.0.0.0";
                bundleFamily = "{AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA}";
            }
            if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv3b_upgrade)
            {
                bundleVersion = "3.0.0.0";
                bundleFamily = "{EEEEEEEE-EEEE-EEEE-EEEE-EEEEEEEEEEEE}";

                // BUGBUG This needs to be replace by the new WiX authoring that supports this.  It doesn't exist yet.  
                //Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpgradeElement upgrade1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpgradeElement();
                //upgrade1.FamilyId = "{AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA}";
                //Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpgradeElement upgrade2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpgradeElement();
                //upgrade2.FamilyId = "{BBBBBBBB-BBBB-BBBB-BBAA-BBBBBBBBBBBB}";
                //this.Layout.BurnManifest.Registration.UpgradeElements.Add(upgrade1);
                //this.Layout.BurnManifest.Registration.UpgradeElements.Add(upgrade2);
            }
            if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv5d_upgrade)
            {
                bundleVersion = "5.0.0.0";
                bundleFamily = "{EEEEEEEE-EEEE-EEEE-EEEE-EEEEEEEEEEEE}";

                // BUGBUG This needs to be replace by the new WiX authoring that supports this.  It doesn't exist yet.  
                //Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpgradeElement upgrade1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpgradeElement();
                //upgrade1.FamilyId = "{AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA}";
                //Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpgradeElement upgrade2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpgradeElement();
                //upgrade2.FamilyId = "{BBBBBBBB-BBBB-BBBB-BBAA-BBBBBBBBBBBB}";
                //this.Layout.BurnManifest.Registration.UpgradeElements.Add(upgrade1);
                //this.Layout.BurnManifest.Registration.UpgradeElements.Add(upgrade2);
            }

            if (this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV3_A)
            {
                bundleVersion = "3.0.0.0";
                bundleFamily = "{FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF}";

                // BUGBUG This needs to be replace by the new WiX authoring that supports this.  It doesn't exist yet.  
                //Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement update1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement();
                //update1.BundleId = LayoutA_v2_FamA.ParameterInfo.Registration.Id;
                //this.Layout.BurnManifest.Registration.UpdateElements.Add(update1);
            }
            if (this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV3_A_B)
            {
                bundleVersion = "3.0.0.0";
                bundleFamily = "{FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF}";

                // BUGBUG This needs to be replace by the new WiX authoring that supports this.  It doesn't exist yet.  
                //Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement update1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement();
                //update1.BundleId = LayoutA_v2_FamA.ParameterInfo.Registration.Id;
                //Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement update2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement();
                //update2.BundleId = LayoutB_v2_FamA.ParameterInfo.Registration.Id;
                //this.Layout.BurnManifest.Registration.UpdateElements.Add(update1);
                //this.Layout.BurnManifest.Registration.UpdateElements.Add(update2);
            }
            if (this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV3_A_D_notExist)
            {
                bundleVersion = "3.0.0.0";
                bundleFamily = "{FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF}";

                // BUGBUG This needs to be replace by the new WiX authoring that supports this.  It doesn't exist yet.  
                //Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement update1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement();
                //update1.BundleId = LayoutA_v2_FamA.ParameterInfo.Registration.Id;
                //Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement update2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement();
                //update2.BundleId = LayoutD_v4_FamC.ParameterInfo.Registration.Id;
                //Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement update3 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement();
                //update3.BundleId = "non-existant ID";
                //this.Layout.BurnManifest.Registration.UpdateElements.Add(update1);
                //this.Layout.BurnManifest.Registration.UpdateElements.Add(update2);
                //this.Layout.BurnManifest.Registration.UpdateElements.Add(update3);
            }

            if (this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV5_noTargetExists)
            {
                bundleVersion = "5.0.0.0";
                bundleFamily = "{FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF}";

                // BUGBUG This needs to be replace by the new WiX authoring that supports this.  It doesn't exist yet.  
                //Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement update1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement();
                //update1.BundleId = "non-existant ID";
                //this.Layout.BurnManifest.Registration.UpdateElements.Add(update1);
            }
            this.Layout.Wix.Bundle.Name = this.Layout.Wix.Bundle.Name + " " + bundleName + " v" + bundleVersion + " " + bundleFamily;
            this.Layout.Wix.Bundle.Version = bundleVersion;
            this.Layout.Wix.Bundle.UpgradeCode = bundleFamily;

            this.Layout.BuildBundle(false);
        }

        private void InitializeMachineState()
        {
            if (m_Mode == TestMode.Repair)
            {
                BurnstubLauncher burnstubLauncher = new BurnstubLauncher();
                if (UserTypeToUse == UserType.NormalUser)
                {
                    burnstubLauncher.UseNormalUser = true;
                }

                System.Diagnostics.Process burnProcess = burnstubLauncher.StartProcess(LayoutA_v2_FamA, " /q");
                burnProcess.WaitForExit();

                if (!ArpEntryIsAccurate(LayoutA_v2_FamA))
                {
                    throw new Exception("Machine state is not valid.  LayoutA_v2_FamA base product is not installed.");
                }
            }
            else
            {
                InstallStaticLayouts();
                bool machineStateIsValid = true;

                // Verify we are good to go.  If not, throw.  
                // No point in continuing with the test if the machine state isn't what we expect.
                foreach (Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager layout in myStaticLayouts)
                {
                    machineStateIsValid &= ArpEntryIsAccurate(layout);
                }
                if (!machineStateIsValid)
                {
                    throw new Exception("Machine state is not valid.  All base products are not installed.");
                }
            }
        }

        private void InstallStaticLayouts()
        {
            BurnstubLauncher burnstubLauncher = new BurnstubLauncher();
            if (UserTypeToUse == UserType.NormalUser)
            {
                burnstubLauncher.UseNormalUser = true;
            }

            foreach (Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager layout in myStaticLayouts)
            {
                System.Diagnostics.Process burnProcess = burnstubLauncher.StartProcess(layout, " /passive");
                burnProcess.WaitForExit();
            }
        }

        public override bool TestPasses()
        {
            bool retVal = true;
            string traceCategory = "Verify TestPasses";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            retVal = true;
            if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv1 ||
                this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV5_noTargetExists)
            {
                exePayloadExitCode = -2147418113; // BUGBUG TODO figure out what the correct "nothing applies" exit code is and look for it.
            }
            retVal &= ExitCodeMatchesExpectation(exePayloadExitCode);
            retVal &= ArpEntriesAreAccurate();
            retVal &= BundleCachesAreAccurate();

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        protected bool ArpEntriesAreAccurate()
        {
            bool retVal = true;
            string traceCategory = "Verify ArpEntriesAreAccurate";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv1 ||
                this.m_PayloadOutcome == PayloadOutcome.Fail)
            {
                retVal &= ArpEntryIsAccurate(LayoutA_v2_FamA);
                retVal &= ArpEntryIsAccurate(LayoutB_v2_FamA);
                retVal &= ArpEntryIsAccurate(LayoutC_v2_FamB);
                retVal &= ArpEntryIsAccurate(LayoutD_v4_FamC);
                retVal &= ArpEntryIsAccurate(LayoutE_v4_FamD);
                retVal &= ArpEntryDoesNotExist(this.Layout);
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv3a)
            {
                retVal &= ArpEntryDoesNotExist(LayoutA_v2_FamA);
                retVal &= ArpEntryDoesNotExist(LayoutB_v2_FamA);
                retVal &= ArpEntryIsAccurate(LayoutC_v2_FamB);
                retVal &= ArpEntryIsAccurate(LayoutD_v4_FamC);
                retVal &= ArpEntryIsAccurate(LayoutE_v4_FamD);
                retVal &= ArpEntryIsAccurate(this.Layout);
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv3b_upgrade)
            {
                retVal &= ArpEntryDoesNotExist(LayoutA_v2_FamA);
                retVal &= ArpEntryDoesNotExist(LayoutB_v2_FamA);
                retVal &= ArpEntryDoesNotExist(LayoutC_v2_FamB);
                retVal &= ArpEntryIsAccurate(LayoutD_v4_FamC);
                retVal &= ArpEntryIsAccurate(LayoutE_v4_FamD);
                retVal &= ArpEntryIsAccurate(this.Layout);
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv5d_upgrade)
            {
                retVal &= ArpEntryDoesNotExist(LayoutA_v2_FamA);
                retVal &= ArpEntryDoesNotExist(LayoutB_v2_FamA);
                retVal &= ArpEntryDoesNotExist(LayoutC_v2_FamB);
                retVal &= ArpEntryIsAccurate(LayoutD_v4_FamC);
                retVal &= ArpEntryIsAccurate(LayoutE_v4_FamD);
                retVal &= ArpEntryIsAccurate(this.Layout);
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV3_A)
            {
                retVal &= ArpEntryIsAccurate(LayoutA_v2_FamA);
                retVal &= ArpEntryIsAccurate(LayoutB_v2_FamA);
                retVal &= ArpEntryIsAccurate(LayoutC_v2_FamB);
                retVal &= ArpEntryIsAccurate(LayoutD_v4_FamC);
                retVal &= ArpEntryIsAccurate(LayoutE_v4_FamD);
                retVal &= ArpEntryIsAccurate(this.Layout);
                // TODO figure out all the ARP entries this thing targeted and make sure they are accurate
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV3_A_B)
            {
                retVal &= ArpEntryIsAccurate(LayoutA_v2_FamA);
                retVal &= ArpEntryIsAccurate(LayoutB_v2_FamA);
                retVal &= ArpEntryIsAccurate(LayoutC_v2_FamB);
                retVal &= ArpEntryIsAccurate(LayoutD_v4_FamC);
                retVal &= ArpEntryIsAccurate(LayoutE_v4_FamD);
                retVal &= ArpEntryIsAccurate(this.Layout);
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV3_A_D_notExist)
            {
                if (m_Mode == TestMode.UninstallUpdateTarget)
                {
                    retVal &= ArpEntryDoesNotExist(LayoutA_v2_FamA);
                }
                else
                {
                    retVal &= ArpEntryIsAccurate(LayoutA_v2_FamA);
                }
                if (m_Mode == TestMode.Repair)
                {
                    retVal &= ArpEntryDoesNotExist(LayoutB_v2_FamA);
                    retVal &= ArpEntryDoesNotExist(LayoutC_v2_FamB);
                    retVal &= ArpEntryIsAccurate(LayoutD_v4_FamC);
                    retVal &= ArpEntryDoesNotExist(LayoutE_v4_FamD);
                }
                else
                {
                    retVal &= ArpEntryIsAccurate(LayoutB_v2_FamA);
                    retVal &= ArpEntryIsAccurate(LayoutC_v2_FamB);
                    retVal &= ArpEntryIsAccurate(LayoutD_v4_FamC);
                    retVal &= ArpEntryIsAccurate(LayoutE_v4_FamD);
                }
                if (this.InstallModeToUse == InstallMode.uninstall && m_Mode != TestMode.UninstallUpdateTarget)
                {
                    retVal &= ArpEntryDoesNotExist(this.Layout);
                }
                else
                {
                    retVal &= ArpEntryIsAccurate(this.Layout);
                }
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV5_noTargetExists)
            {
                retVal &= ArpEntryIsAccurate(LayoutA_v2_FamA);
                retVal &= ArpEntryIsAccurate(LayoutB_v2_FamA);
                retVal &= ArpEntryIsAccurate(LayoutC_v2_FamB);
                retVal &= ArpEntryIsAccurate(LayoutD_v4_FamC);
                retVal &= ArpEntryIsAccurate(LayoutE_v4_FamD);
                retVal &= ArpEntryDoesNotExist(this.Layout);
            }
            else
            {
                Trace.WriteLine("Unknown UpgradeUpdateType, didn't verify anything", traceCategory);
                retVal = false;
            }

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }


        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        protected bool BundleCachesAreAccurate()
        {
            bool retVal = true;
            string traceCategory = "Verify BundleCachesAreAccurate";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv1 ||
                this.m_PayloadOutcome == PayloadOutcome.Fail)
            {
                retVal &= BundleCacheExists(LayoutA_v2_FamA);
                retVal &= BundleCacheExists(LayoutB_v2_FamA);
                retVal &= BundleCacheExists(LayoutC_v2_FamB);
                retVal &= BundleCacheExists(LayoutD_v4_FamC);
                retVal &= BundleCacheExists(LayoutE_v4_FamD);
                retVal &= BundleCacheNotExist(this.Layout);
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv3a)
            {
                retVal &= BundleCacheNotExist(LayoutA_v2_FamA);
                retVal &= BundleCacheNotExist(LayoutB_v2_FamA);
                retVal &= BundleCacheExists(LayoutC_v2_FamB);
                retVal &= BundleCacheExists(LayoutD_v4_FamC);
                retVal &= BundleCacheExists(LayoutE_v4_FamD);
                retVal &= BundleCacheExists(this.Layout);
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv3b_upgrade)
            {
                retVal &= BundleCacheNotExist(LayoutA_v2_FamA);
                retVal &= BundleCacheNotExist(LayoutB_v2_FamA);
                retVal &= BundleCacheNotExist(LayoutC_v2_FamB);
                retVal &= BundleCacheExists(LayoutD_v4_FamC);
                retVal &= BundleCacheExists(LayoutE_v4_FamD);
                retVal &= BundleCacheExists(this.Layout);
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.LayoutBv5d_upgrade)
            {
                retVal &= BundleCacheNotExist(LayoutA_v2_FamA);
                retVal &= BundleCacheNotExist(LayoutB_v2_FamA);
                retVal &= BundleCacheNotExist(LayoutC_v2_FamB);
                retVal &= BundleCacheExists(LayoutD_v4_FamC);
                retVal &= BundleCacheExists(LayoutE_v4_FamD);
                retVal &= BundleCacheExists(this.Layout);
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV3_A)
            {
                retVal &= BundleCacheExists(LayoutA_v2_FamA);
                retVal &= BundleCacheExists(LayoutB_v2_FamA);
                retVal &= BundleCacheExists(LayoutC_v2_FamB);
                retVal &= BundleCacheExists(LayoutD_v4_FamC);
                retVal &= BundleCacheExists(LayoutE_v4_FamD);
                retVal &= BundleCacheExists(this.Layout);
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV3_A_B)
            {
                retVal &= BundleCacheExists(LayoutA_v2_FamA);
                retVal &= BundleCacheExists(LayoutB_v2_FamA);
                retVal &= BundleCacheExists(LayoutC_v2_FamB);
                retVal &= BundleCacheExists(LayoutD_v4_FamC);
                retVal &= BundleCacheExists(LayoutE_v4_FamD);
                retVal &= BundleCacheExists(this.Layout);
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV3_A_D_notExist)
            {
                if (m_Mode == TestMode.UninstallUpdateTarget)
                {
                    retVal &= BundleCacheNotExist(LayoutA_v2_FamA);
                }
                else
                {
                    retVal &= BundleCacheExists(LayoutA_v2_FamA);
                }
                if (m_Mode == TestMode.Repair)
                {
                    retVal &= BundleCacheNotExist(LayoutB_v2_FamA);
                    retVal &= BundleCacheNotExist(LayoutC_v2_FamB);
                    retVal &= BundleCacheExists(LayoutD_v4_FamC);
                    retVal &= BundleCacheNotExist(LayoutE_v4_FamD);
                }
                else
                {
                    retVal &= BundleCacheExists(LayoutB_v2_FamA);
                    retVal &= BundleCacheExists(LayoutC_v2_FamB);
                    retVal &= BundleCacheExists(LayoutD_v4_FamC);
                    retVal &= BundleCacheExists(LayoutE_v4_FamD);
                }
                if (this.InstallModeToUse == InstallMode.uninstall && m_Mode != TestMode.UninstallUpdateTarget)
                {
                    retVal &= BundleCacheNotExist(this.Layout); 
                }
                else
                {
                    retVal &= BundleCacheExists(this.Layout);
                }
            }
            else if (this.m_UpgradeUpdateType == UpgradeUpdateType.UpdateV5_noTargetExists)
            {
                retVal &= BundleCacheExists(LayoutA_v2_FamA);
                retVal &= BundleCacheExists(LayoutB_v2_FamA);
                retVal &= BundleCacheExists(LayoutC_v2_FamB);
                retVal &= BundleCacheExists(LayoutD_v4_FamC);
                retVal &= BundleCacheExists(LayoutE_v4_FamD);
                retVal &= BundleCacheNotExist(this.Layout);
            }
            else
            {
                Trace.WriteLine("Unknown UpgradeUpdateType, didn't verify anything", traceCategory);
                retVal = false;
            }

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

    }
}
