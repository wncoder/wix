//-----------------------------------------------------------------------
// <copyright file="BurnTests.cs" company="Microsoft">
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
//     Contains methods test Burn.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn
{
    using System;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class BurnTests : WixTests
    {
        public static string PayloadCacheFolder = "Package Cache";
        public static string PerMachinePayloadCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%ProgramData%\" + PayloadCacheFolder);
        public static string PerUserPayloadCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%LOCALAPPDATA%\" + PayloadCacheFolder);

        private static readonly string[] Extensions = new string[] { "WixBalExtension", "WixDependencyExtension", "WixUtilExtension" };

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A then removes it.")]
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

            try
            {
                // Install the bundles.
                BundleInstaller installerA = new BundleInstaller(this, bundleA).Install();
                BundleInstaller installerB = new BundleInstaller(this, bundleB).Install();

                // Make sure the MSIs and EXE are installed.
                //Assert.IsTrue(this.IsPackageInstalled(packageA));
                //Assert.IsTrue(this.IsPackageInstalled(packageB));
                //Assert.IsTrue(this.IsRegistryValueEqual("Version", "1.0.0.0"));

                // Attempt to uninstall bundleA.
                installerA.Uninstall();

                // Verify packageA and ExeA are still installed.
                //Assert.IsTrue(this.IsPackageInstalled(packageA));
                //Assert.IsTrue(this.IsRegistryValueEqual("Version", "1.0.0.0"));

                // Uninstall bundleB now.
                installerB.Uninstall();

                // Make sure the MSIs are not installed.
                //Assert.IsFalse(this.IsPackageInstalled(packageB));
                //Assert.IsFalse(this.IsPackageInstalled(packageA));
                //Assert.IsTrue(this.IsRegistryValueEqual("Version", null));
            }
            finally
            {
                //this.CleanupRegistry();
            }
        }
    }
}
