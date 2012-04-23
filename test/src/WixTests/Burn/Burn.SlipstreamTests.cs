//-----------------------------------------------------------------------
// <copyright file="SlipstreamTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Contains methods test Burn slipstreaming.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Win32;
    using System.IO;

    [TestClass]
    public class SlipstreamTests : BurnTests
    {
        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle with slipstream then removes it.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_SlipstreamInstallUninstall()
        {
            string patchedVersion = "1.0.1.0";

            // Build the packages.
            string packageA = new PackageBuilder(this, "A").Build().Output;
            string packageAUpdate = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", patchedVersion } }, NeverGetsInstalled = true }.Build().Output;
            string patchA = new PatchBuilder(this, "PatchA") { TargetPath = packageA, UpgradePath = packageAUpdate }.Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("patchA", patchA);

            string bundleA = new BundleBuilder(this, "BundleA") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;
            BundleInstaller install = new BundleInstaller(this, bundleA).Install();

            string packageSourceCodeInstalled = this.GetTestInstallFolder(@"A\A.wxs");
            Assert.IsTrue(File.Exists(packageSourceCodeInstalled), String.Concat("Should have found Package A payload installed at: ", packageSourceCodeInstalled));
            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual(patchedVersion, actualVersion);
            }

            install.Uninstall();

            Assert.IsFalse(File.Exists(packageSourceCodeInstalled), String.Concat("Package A payload should have been removed by uninstall from: ", packageSourceCodeInstalled));
            Assert.IsNull(this.GetTestRegistryRoot(), "Test registry key should have been removed during uninstall.");

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle with slipstream then removes it.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_SlipstreamRepair()
        {
            string patchedVersion = "1.0.1.0";

            // Build the packages.
            string packageA = new PackageBuilder(this, "A").Build().Output;
            string packageAUpdate = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", patchedVersion } }, NeverGetsInstalled = true }.Build().Output;
            string patchA = new PatchBuilder(this, "PatchA") { TargetPath = packageA, UpgradePath = packageAUpdate }.Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("patchA", patchA);

            string bundleA = new BundleBuilder(this, "BundleA") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;
            BundleInstaller install = new BundleInstaller(this, bundleA).Install();

            string packageSourceCodeInstalled = this.GetTestInstallFolder(@"A\A.wxs");
            Assert.IsTrue(File.Exists(packageSourceCodeInstalled), String.Concat("Should have found Package A payload installed at: ", packageSourceCodeInstalled));
            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual(patchedVersion, actualVersion);
            }

            // Delete the installed file and registry key.
            File.Delete(packageSourceCodeInstalled);
            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                root.DeleteValue("A");
            }

            // Repair and verify the repair fixed everything.
            install.Repair();

            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual(patchedVersion, actualVersion);
            }
            Assert.IsTrue(File.Exists(packageSourceCodeInstalled), String.Concat("Should have found Package A payload repaired at: ", packageSourceCodeInstalled));

            // Clean up.
            install.Uninstall();

            Assert.IsFalse(File.Exists(packageSourceCodeInstalled), String.Concat("Package A payload should have been removed by uninstall from: ", packageSourceCodeInstalled));
            Assert.IsNull(this.GetTestRegistryRoot(), "Test registry key should have been removed during uninstall.");

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle with slipstream then removes it.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_SlipstreamRemovePatchAlone()
        {
            string patchedVersion = "1.0.1.0";
            // Build the packages.
            string packageA = new PackageBuilder(this, "A").Build().Output;
            string packageAUpdate = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", patchedVersion } }, NeverGetsInstalled = true }.Build().Output;
            string patchA = new PatchBuilder(this, "PatchA") { TargetPath = packageA, UpgradePath = packageAUpdate }.Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("patchA", patchA);

            string bundleA = new BundleBuilder(this, "BundleA") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;
            BundleInstaller install = new BundleInstaller(this, bundleA).Install();

            string packageSourceCodeInstalled = this.GetTestInstallFolder(@"A\A.wxs");
            Assert.IsTrue(File.Exists(packageSourceCodeInstalled), String.Concat("Should have found Package A payload installed at: ", packageSourceCodeInstalled));
            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual(patchedVersion, actualVersion);
            }

            // Remove only the slipstream patch and ensure the version is back to default.
            this.SetPackageRequestedState("patchA", Bootstrapper.RequestState.Absent);
            install.Modify();

            Assert.IsTrue(File.Exists(packageSourceCodeInstalled), String.Concat("Should have found Package A payload *still* installed at: ", packageSourceCodeInstalled));
            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual("1.0.0.0", actualVersion, "Patch A should have been removed and so the registry key would go back to default version.");
            }

            install.Uninstall(); // uninstall just to make sure no error occur removing the package without the patch.

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle with slipstreamed package A and package B then removes both package A and patch A at same time.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_SlipstreamRemovePackageAndPatch()
        {
            string patchedVersion = "1.0.1.0";

            // Build the packages.
            string packageA = new PackageBuilder(this, "A").Build().Output;
            string packageAUpdate = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", patchedVersion } }, NeverGetsInstalled = true }.Build().Output;
            string patchA = new PatchBuilder(this, "PatchA") { TargetPath = packageA, UpgradePath = packageAUpdate }.Build().Output;
            string packageB = new PackageBuilder(this, "B").Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("patchA", patchA);
            bindPaths.Add("packageB", packageB);

            // Create bundle and install everything.
            string bundleB = new BundleBuilder(this, "BundleB") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;
            BundleInstaller install = new BundleInstaller(this, bundleB).Install();

            string packageSourceCodeInstalled = this.GetTestInstallFolder(@"A\A.wxs");
            Assert.IsTrue(File.Exists(packageSourceCodeInstalled), String.Concat("Should have found Package A payload installed at: ", packageSourceCodeInstalled));
            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual(patchedVersion, actualVersion);
            }

            packageSourceCodeInstalled = this.GetTestInstallFolder(@"B\B.wxs");
            Assert.IsTrue(File.Exists(packageSourceCodeInstalled), String.Concat("Should have found Package B payload installed at: ", packageSourceCodeInstalled));

            // Remove package A and it's patch should go with it.
            this.SetPackageRequestedState("packageA", Bootstrapper.RequestState.Absent);
            this.SetPackageRequestedState("patchA", Bootstrapper.RequestState.Absent);
            install.Modify();

            this.ResetPackageStates("packageA");
            this.ResetPackageStates("patchA");

            packageSourceCodeInstalled = this.GetTestInstallFolder(@"A\A.wxs");
            Assert.IsFalse(File.Exists(packageSourceCodeInstalled), String.Concat("After modify, should *not* have found Package A payload installed at: ", packageSourceCodeInstalled));

            // Remove.
            install.Uninstall();

            packageSourceCodeInstalled = this.GetTestInstallFolder(@"B\B.wxs");
            Assert.IsFalse(File.Exists(packageSourceCodeInstalled), String.Concat("After uninstall bundle, should *not* have found Package B payload installed at: ", packageSourceCodeInstalled));
            Assert.IsNull(this.GetTestRegistryRoot(), "Test registry key should have been removed during uninstall.");

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle with slipstreamed package A and package B and trigger error rollback.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_SlipstreamFailureRollback()
        {
            string patchedVersion = "1.0.1.0";

            // Build the packages.
            string packageA = new PackageBuilder(this, "A").Build().Output;
            string packageAUpdate = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", patchedVersion } }, NeverGetsInstalled = true }.Build().Output;
            string patchA = new PatchBuilder(this, "PatchA") { TargetPath = packageA, UpgradePath = packageAUpdate }.Build().Output;
            string packageB = new PackageBuilder(this, "B").Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("patchA", patchA);
            bindPaths.Add("packageB", packageB);

            // Create a folder with same name as the file to be installed in package B, this will trigger error in B and rollback A
            string errorTriggeringFolder = this.GetTestInstallFolder(@"B\B.wxs");
            if (!Directory.Exists(errorTriggeringFolder))
            {
                Directory.CreateDirectory(errorTriggeringFolder);
            }

            // Create bundle and install everything.
            string bundleB = new BundleBuilder(this, "BundleB") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;
            BundleInstaller install = new BundleInstaller(this, bundleB).Install((int)MSIExec.MSIExecReturnCode.ERROR_INSTALL_FAILURE);

            // Nothing should exist after the rollback
            string packageSourceCodeInstalled = this.GetTestInstallFolder(@"A\A.wxs");
            Assert.IsFalse(File.Exists(packageSourceCodeInstalled), String.Concat("Should NOT have found Package A payload installed at: ", packageSourceCodeInstalled));

            packageSourceCodeInstalled = this.GetTestInstallFolder(@"B\B.wxs");
            Assert.IsFalse(File.Exists(packageSourceCodeInstalled), String.Concat("Should NOT have found Package B payload installed at: ", packageSourceCodeInstalled));
            Assert.IsNull(this.GetTestRegistryRoot(), "Test registry key should NOT exist after rollback.");
            
            // Delete the directory
            Directory.Delete(errorTriggeringFolder);

            this.CleanTestArtifacts = true;
        }

        [Ignore]
        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle using automatic slipstreaming then removes it.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_AutomaticSlipstreamInstallUninstall()
        {
            const string originalVersion = "1.0.0.0";
            const string patchedVersion = "1.0.1.0";

            // Build the packages.
            string packageA = new PackageBuilder(this, "A").Build().Output;
            string packageAUpdate = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", patchedVersion } }, NeverGetsInstalled = true }.Build().Output;
            string packageB = new PackageBuilder(this, "B").Build().Output;
            string packageBUpdate = new PackageBuilder(this, "B") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", patchedVersion} }, NeverGetsInstalled = true }.Build().Output;
            string patchA = new PatchBuilder(this, "PatchA") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", patchedVersion } }, TargetPaths = new string[] { packageA, packageB }, UpgradePaths = new string[] { packageAUpdate, packageBUpdate } }.Build().Output;
            string patchB = new PatchBuilder(this, "PatchB") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", patchedVersion } }, TargetPaths = new string[] { packageA, packageB }, UpgradePaths = new string[] { packageAUpdate, packageBUpdate } }.Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("packageB", packageB);
            bindPaths.Add("patchA", patchA);
            bindPaths.Add("patchB", patchB);

            string bundleC = new BundleBuilder(this, "BundleC") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;
            BundleInstaller install = new BundleInstaller(this, bundleC).Install();

            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                // Product A should've slipstreamed both patches.
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual(patchedVersion, actualVersion);

                actualVersion = root.GetValue("A2") as string;
                Assert.AreEqual(patchedVersion, actualVersion);

                // Product B should've only slipstreamed patch B.
                actualVersion = root.GetValue("B") as string;
                Assert.AreEqual(originalVersion, actualVersion);

                actualVersion = root.GetValue("B2") as string;
                Assert.AreEqual(patchedVersion, actualVersion);
            }

            install.Uninstall();

            Assert.IsNull(this.GetTestRegistryRoot(), "Test registry key should have been removed during uninstall.");

            this.CleanTestArtifacts = true;
        }
    }
}
