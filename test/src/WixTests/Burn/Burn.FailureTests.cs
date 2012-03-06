//-----------------------------------------------------------------------
// <copyright file="FailureTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Contains methods test Burn failure scenarios.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using Microsoft.Deployment.WindowsInstaller;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Win32;

    [TestClass]
    public class FailureTests : BurnTests
    {
        [TestMethod]
        [Priority(2)]
        [Description("Cancels Package B very, very early.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_CancelVeryEarly()
        {
            // Build the packages.
            string packageA = new PackageBuilder(this, "A") { Extensions = Extensions }.Build().Output;
            string packageB = new PackageBuilder(this, "B") { Extensions = Extensions }.Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("packageB", packageB);

            // Build the bundle.
            string bundleA = new BundleBuilder(this, "BundleA") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;

            // Cancel package B right away.
            this.SetPackageCancelExecuteAtProgress("PackageB", 1);

            // Install the bundle and hopefully it fails.
            BundleInstaller installerA = new BundleInstaller(this, bundleA).Install(expectedExitCode: ErrorCodes.ERROR_INSTALL_USEREXIT);
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageB));

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Cancels Package B very, very late.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_CancelVeryLate()
        {
            // Build the packages.
            string packageA = new PackageBuilder(this, "A") { Extensions = Extensions }.Build().Output;
            string packageB = new PackageBuilder(this, "B") { Extensions = Extensions }.Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("packageB", packageB);

            // Build the bundle.
            string bundleA = new BundleBuilder(this, "BundleA") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;

            // Cancel package B at the last moment possible..
            this.SetPackageCancelExecuteAtProgress("PackageB", 100);

            // Install the bundle and hopefully it fails.
            BundleInstaller installerA = new BundleInstaller(this, bundleA).Install(expectedExitCode: ErrorCodes.ERROR_INSTALL_USEREXIT);
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageB));

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Cancels Package A being executed while Package B still being cached.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_CancelExecuteWhileCaching()
        {
            // Build the packages.
            string packageA = new PackageBuilder(this, "A") { Extensions = Extensions }.Build().Output;
            string packageB = new PackageBuilder(this, "B") { Extensions = Extensions }.Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("packageB", packageB);

            // Build the bundle.
            string bundleA = new BundleBuilder(this, "BundleB") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;

            // Slow the caching of package B to ensure that package A starts installing and cancels.
            this.SetPackageCancelExecuteAtProgress("PackageA", 50);
            this.SetPackageSlowCache("PackageB", 1000);

            // Install the bundle and hopefully it fails.
            BundleInstaller installerA = new BundleInstaller(this, bundleA).Install(expectedExitCode: ErrorCodes.ERROR_INSTALL_USEREXIT);
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageB));

            this.CleanTestArtifacts = true;
        }
    }
}
