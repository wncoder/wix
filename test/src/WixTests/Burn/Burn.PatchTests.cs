//-----------------------------------------------------------------------
// <copyright file="SlipstreamTests.cs" company="Microsoft">
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
//     Contains methods test Burn slipstreaming.
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Burn
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Win32;
    using System.IO;

    [TestClass]
    public class PatchTests : BurnTests
    {
        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle with slipstream then removes it.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_PatchInstallUninstall()
        {
            string originalVersion = "1.0.0.0";
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
            string bundleAPatch = new BundleBuilder(this, "PatchBundleA") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;

            // Install the unpatched bundle.
            BundleInstaller installA = new BundleInstaller(this, bundleA).Install();
            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual(originalVersion, actualVersion);
            }

            // Install the patch bundle.
            BundleInstaller installAPatch = new BundleInstaller(this, bundleAPatch).Install();
            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual(patchedVersion, actualVersion);
            }

            // Uninstall the patch bundle.
            installAPatch.Uninstall();
            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual(originalVersion, actualVersion);
            }

            installA.Uninstall();
            Assert.IsNull(this.GetTestRegistryRoot(), "Test registry key should have been removed during uninstall.");

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs patch bundle with repeated Detect phases.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_PatchRedetect()
        {
            this.SetRedetectCount(1);
            this.Burn_PatchInstallUninstall();
        }
    }
}
