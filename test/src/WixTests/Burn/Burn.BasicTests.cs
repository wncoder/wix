//-----------------------------------------------------------------------
// <copyright file="BurnTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Contains methods test Burn.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn
{
    using System;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Win32;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers;

    [TestClass]
    public class BasicTests : BurnTests
    {
        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A then bundle B then removes in same order.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_InstallUninstall()
        {
            // Build the packages.
            string packageA = new PackageBuilder(this, "A") { Extensions = Extensions }.Build().Output;
            string packageB = new PackageBuilder(this, "B") { Extensions = Extensions }.Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("packageB", packageB);

            // Build the bundles.
            string bundleA = new BundleBuilder(this, "BundleA") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;
            string bundleB = new BundleBuilder(this, "BundleB") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;

            // Install the bundles.
            BundleInstaller installerA = new BundleInstaller(this, bundleA).Install();
            BundleInstaller installerB = new BundleInstaller(this, bundleB).Install();

            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageB));

            // Attempt to uninstall bundleA.
            installerA.Uninstall();

            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageB));

            // Uninstall bundleB now.
            installerB.Uninstall();

            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageB));

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A then removes it.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_MajorUpgradeRemovesPackageFixedByRepair()
        {
            string v2Version = "2.0.0.0";

            // Build the packages.
            string packageAv1 = new PackageBuilder(this, "A").Build().Output;
            string packageAv2 = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", v2Version } } }.Build().Output;
            string packageB = new PackageBuilder(this, "B").Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPathsv1 = new Dictionary<string, string>() { { "packageA", packageAv1 } };
            Dictionary<string, string> bindPathsv2 = new Dictionary<string, string>() { { "packageA", packageAv2 }, { "packageB", packageB } };

            // Build the bundles.
            string bundleAv1 = new BundleBuilder(this, "BundleA") { BindPaths = bindPathsv1, Extensions = Extensions }.Build().Output;
            string bundleB = new BundleBuilder(this, "BundleB") { BindPaths = bindPathsv2, Extensions = Extensions }.Build().Output;

            // Initialize with first bundle.
            BundleInstaller installerA = new BundleInstaller(this, bundleAv1).Install();
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv1));

            // Install second bundle which will major upgrade away v1.
            BundleInstaller installerB = new BundleInstaller(this, bundleB).Install();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv1));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv2));

            // Uninstall second bundle which will remove all packages
            installerB.Uninstall();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv1));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv2));

            // Repair first bundle to get v1 back on the machine.
            installerA.Repair();
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv1));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv2));

            // Uninstall first bundle and everything should be gone.
            installerA.Uninstall();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv1));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv2));

            this.CleanTestArtifacts = true;
        }
    }
}
