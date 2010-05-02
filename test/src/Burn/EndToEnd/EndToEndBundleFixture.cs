//-----------------------------------------------------------------------
// <copyright file="EndToEndBundleFixture.cs" company="Microsoft">
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
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager;
    
    class EndToEndBundleFixture : EndToEndBaseFixture
    {
        public EndToEndBundleFixture()
            : base()
        {
        }

        public void RunTest(InstallMode installMode,
            UiMode uiMode,
            UserType userType)
        {
            InstallModeToUse = installMode;
            UiModeToUse = uiMode;
            UserTypeToUse = userType;
            UseABundle = true;

            BuildLayout();
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
            UseABundle = true;

            VerifiableProperties = this.RunUninstallFromArpScenario(
                UiModeToUse,
                UserTypeToUse);
        }

        private void BuildLayout()
        {
            // Per-machine Exe
            this.Layout.AddExe(testExeFile, "TestExe1.exe", testExeUrl, true);
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[0]).InstallArguments = " /ec 000 /log %temp%\\TestExe1-TestExeLog.txt ";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[0]).RepairArguments = " /ec 0000 /log %temp%\\TestExe1-TestExeLog.txt ";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[0]).UninstallArguments = " /ec 00000 /log %temp%\\TestExe1-TestExeLog.txt ";

            // Per-user Exe
            this.Layout.AddExe(testExeFile, "TestExe2.exe", testExeUrl, true);
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[1]).InstallArguments = " /ec 000 /log %temp%\\TestExe2-TestExeLog.txt ";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[1]).RepairArguments = " /ec 0000 /log %temp%\\TestExe2-TestExeLog.txt ";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[1]).UninstallArguments = " /ec 00000 /log %temp%\\TestExe2-TestExeLog.txt ";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[1]).PerMachine = "no";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[1]).PerMachine = false;

            // Per-machine MSI
            this.Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, true);

            // Per-user MSI
            this.Layout.AddMsi(testMsiPerUserFile, null, testMsiPerUserUrl, true);
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem)this.Layout.ParameterInfo.Items.Items[3]).PerMachine = false;

            // Per-user MSI with external Cab
            List<LayoutManager.ExternalFile> extFiles = new List<LayoutManager.ExternalFile>();
            LayoutManager.ExternalFile extFile = new LayoutManager.ExternalFile();
            extFile.File = testMsiPerUserExtCabCabFile;
            extFile.Url = testMsiPerUserExtCabCabUrl;
            extFiles.Add(extFile);
            this.Layout.AddMsiAndExternalFiles(testMsiPerUserExtCabMsiFile, null, testMsiPerUserExtCabMsiUrl, true, extFiles);
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem)this.Layout.ParameterInfo.Items.Items[4]).PerMachine = false;


            if (UseABundle)
            {
                this.Layout.GenerateBundle();
            }
            else
            {
                throw new Exception("This fixture only works with bundles, not raw layouts");
            }
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

            retVal &= ExitCodeMatchesExpectation(0);
            retVal &= NoRunKeysSet();
            //retVal &= DhtmlLogContainsPerformerSucceededMessage();
            if (this.InstallModeToUse == InstallMode.uninstall)
            {
                retVal &= !AllPackagesInstalled();
                retVal &= ArpEntryDoesNotExist();
                retVal &= BundleCacheNotExist(this.Layout);
            }
            else
            {
                retVal &= AllPackagesInstalled();
                retVal &= PayloadCacheIsAccurate();
                retVal &= BundleCacheExists(this.Layout);
                retVal &= ArpEntryIsAccurate();
            }
            if (validateMetricsData) { retVal &= MetricsDataIsAccurate(); }

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        private bool AllPackagesInstalled()
        {
            bool retVal = true;
            string traceCategory = "Verify AllPackagesInstalled";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            foreach (Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnBaseItems item in this.Layout.ParameterInfo.Items.Items)
            {
                Type t = item.GetType();
                if (t == typeof(Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem))
                {
                    retVal &= VerifyExeIsInstalled(item.SourceFilePath, traceCategory);
                }
                if (t == typeof(Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem))
                {
                    retVal &= VerifyMsiIsInstalled(item.SourceFilePath, traceCategory);
                }
                if (t == typeof(Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem))
                {
                    retVal &= VerifyMspIsInstalled(item.SourceFilePath, traceCategory);
                }
            }

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        private bool VerifyExeIsInstalled(string ExeFile, string traceCategory)
        {
            bool retVal = true;
            Trace.WriteLine("Exe file not verified (no way to figure out if an Exe is installed): " + ExeFile, traceCategory);

            return retVal;
        }

        private bool VerifyMsiIsInstalled(string MsiFile, string traceCategory)
        {
            bool retVal = true;
            if (!Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsProductInstalled(Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.GetMSIProductCode(MsiFile)))
            {
                Trace.WriteLine("Msi file not installed: " + MsiFile, traceCategory);
                retVal = false;
            }
            else
            {
                Trace.WriteLine("Msi file installed: " + MsiFile, traceCategory);
            }

            return retVal;
        }

        private bool VerifyMspIsInstalled(string MspFile, string traceCategory)
        {
            bool retVal = true;
            if (!Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsPatchInstalled(Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.GetMSIProductCode(MspFile)))
            {
                Trace.WriteLine("Msp file not installed: " + MspFile, traceCategory);
                retVal = false;
            }
            else
            {
                Trace.WriteLine("Msp file installed: " + MspFile, traceCategory);
            }

            return retVal;
        }
    }
}
