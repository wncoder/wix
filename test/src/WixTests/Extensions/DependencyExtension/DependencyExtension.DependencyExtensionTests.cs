//-----------------------------------------------------------------------
// <copyright file="DependencyExtension.DependencySetupTests.cs" company="Microsoft">
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
// <summary>Dependency extension functional tests.</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Extensions.DependencyExtension
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Text;

    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Win32;

    [TestClass]
    public sealed class DependencyExtensionTests : WixTests
    {
        private const int E_INSTALLFAILURE = unchecked((int)0x80070643);

        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Extensions\DependencyExtension\DependencyExtensionTests");
        private static readonly string[] Extensions = new string[] { "WixBalExtension", "WixDependencyExtension", "WixUtilExtension" };

        [TestMethod]
        [Priority(2)]
        [Description("Install products A then B, and uninstall in reverse order as normal.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_Install()
        {
            string packageA = BuildPackage("A", null);
            string packageB = BuildPackage("B", null);

            MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
            MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

            // Uninstall in reverse order.
            MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
            MSIExec.UninstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install product B without dependency A installed and fail.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_MissingDependency()
        {
            string packageB = BuildPackage("B", null);

            string logB = MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.ERROR_INSTALL_FAILURE);
            Assert.IsTrue(LogVerifier.MessageInLogFile(logB, @"WixDependencyCheck:  The dependency ""Microsoft.WiX.DependencyExtension_MissingDependency.A,v1.0"" is missing or is not the required version."));
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install product B without dependency A installed and override.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_MissingDependencyOverride()
        {
            string packageB = BuildPackage("B", null);

            string logB = InstallProductWithProperties(packageB, MSIExec.MSIExecReturnCode.SUCCESS, "DISABLEDEPENDENCYCHECK=1");
            Assert.IsTrue(LogVerifier.MessageInLogFile(logB, @"WixDependencyCheck:  Skipping the dependency check since DISABLEDEPENDENCYCHECK is set."));

            MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install products A then B, and uninstall A while B is still present and appear to succeed.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_UninstallDependency()
        {
            string packageA = BuildPackage("A", null);
            string packageB = BuildPackage("B", null);

            MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
            MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

            // Now attempt the uninstall of dependency package A.
            string logA = MSIExec.UninstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
            Assert.IsTrue(LogVerifier.MessageInLogFile(logA, @"WixDependencyCheck:  Found dependent"));

            // Uninstall in reverse order.
            MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
            MSIExec.UninstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install products A then B, and uninstall A while B is still present and override.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_UninstallDependencyOverrideAll()
        {
            string packageA = BuildPackage("A", null);
            string packageB = BuildPackage("B", null);

            MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
            MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

            // Now attempt the uninstall of dependency package A.
            string logA = UninstallProductWithProperties(packageA, MSIExec.MSIExecReturnCode.SUCCESS, "IGNOREDEPENDENCIES=ALL");
            Assert.IsTrue(LogVerifier.MessageInLogFile(logA, @"WixDependencyCheck:  Skipping the dependencies check since IGNOREDEPENDENCIES contains ""ALL""."));

            MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install products A then B, and uninstall A while B is still present and override.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_UninstallDependencyOverrideSpecific()
        {
            string packageA = BuildPackage("A", null);
            string packageB = BuildPackage("B", null);

            MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
            MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

            // Now attempt the uninstall of dependency package A.
            string productCode = MsiUtils.GetMSIProductCode(packageB);
            string ignoreDependencies = String.Concat("IGNOREDEPENDENCIES=", productCode);

            UninstallProductWithProperties(packageA, MSIExec.MSIExecReturnCode.SUCCESS, ignoreDependencies);

            MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install products A then B, upgrades A, then attempts to uninstall A while B is still present.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_UninstallUpgradedDependency()
        {
            string packageA = BuildPackage("A", null);
            string packageB = BuildPackage("B", null);
            string packageA1 = BuildPackage("A", "1.0.1.0");

            MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
            MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

            // Build the upgraded dependency A1 and make sure A was removed.
            MSIExec.InstallProduct(packageA1, MSIExec.MSIExecReturnCode.SUCCESS);
            Assert.IsFalse(IsPackageInstalled(packageA));

            // Now attempt the uninstall of upgraded dependency package A1.
            string logA = MSIExec.UninstallProduct(packageA1, MSIExec.MSIExecReturnCode.SUCCESS);
            Assert.IsTrue(LogVerifier.MessageInLogFile(logA, @"WixDependencyCheck:  Found dependent"));

            // Uninstall in reverse order.
            MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
            MSIExec.UninstallProduct(packageA1, MSIExec.MSIExecReturnCode.SUCCESS);
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A then bundle B, and removes them in reverse order.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_InstallBundles()
        {
            // Build the packages.
            string packageA = this.BuildPackage("A", null);
            string packageB = this.BuildPackage("B", null);

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("packageB", packageB);

            // Build the bundles.
            string bundleA = this.BuildBundle("BundleA", null, bindPaths);
            string bundleB = this.BuildBundle("BundleB", null, bindPaths);

            // Install the bundles.
            this.InstallBundleWithArguments(bundleA, (int)MSIExec.MSIExecReturnCode.SUCCESS);
            this.InstallBundleWithArguments(bundleB, (int)MSIExec.MSIExecReturnCode.SUCCESS);

            // Make sure the MSIs are installed.
            Assert.IsTrue(this.IsPackageInstalled(packageA));
            Assert.IsTrue(this.IsPackageInstalled(packageB));
            Assert.IsTrue(this.IsRegistryValueEqual("Version", "1.0.0.0"));

            // Uninstall in reverse order.
            this.UninstallBundleWithArguments(bundleB, (int)MSIExec.MSIExecReturnCode.SUCCESS);
            this.UninstallBundleWithArguments(bundleA, (int)MSIExec.MSIExecReturnCode.SUCCESS);

            // Make sure the MSIs are not installed.
            Assert.IsFalse(this.IsPackageInstalled(packageB));
            Assert.IsFalse(this.IsPackageInstalled(packageA));
            Assert.IsTrue(this.IsRegistryValueEqual("Version", null));
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A then bundle B, attempts to remove bundle A, then removes them in reverse order.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_UninstallBundleWithDependent()
        {
            // Build the packages.
            string packageA = this.BuildPackage("A", null);
            string packageB = this.BuildPackage("B", null);

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("packageB", packageB);

            // Build the bundles.
            string bundleA = this.BuildBundle("BundleA", null, bindPaths);
            string bundleB = this.BuildBundle("BundleB", null, bindPaths);

            // Install the bundles.
                this.InstallBundleWithArguments(bundleA, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                this.InstallBundleWithArguments(bundleB, (int)MSIExec.MSIExecReturnCode.SUCCESS);

                // Make sure the MSIs and EXE are installed.
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsPackageInstalled(packageB));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", "1.0.0.0"));

                // Attempt to uninstall bundleA.
                this.UninstallBundleWithArguments(bundleA, (int)MSIExec.MSIExecReturnCode.SUCCESS);

                // Verify packageA and ExeA are still installed.
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", "1.0.0.0"));

                // Uninstall bundleB now.
                this.UninstallBundleWithArguments(bundleB, (int)MSIExec.MSIExecReturnCode.SUCCESS);

                // Make sure the MSIs are not installed.
                Assert.IsFalse(this.IsPackageInstalled(packageB));
                Assert.IsFalse(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", null));
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install bundle A then B, upgrades A, then attempts to uninstall A while B is still present.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_UninstallUpgradedBundle()
        {
            // Build the packages.
            string packageA = BuildPackage("A", null);
            string packageB = BuildPackage("B", null);
            string packageA1 = BuildPackage("A", "1.0.1.0");

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("packageB", packageB);

            // Build the bundles.
            string bundleA = this.BuildBundle("BundleA", null, bindPaths);
            string bundleB = this.BuildBundle("BundleB", null, bindPaths);

            // Override the path for A to A1.
            bindPaths["packageA"] = packageA1;
            string bundleA1 = this.BuildBundle("BundleA", "1.0.1.0", bindPaths);

                // Install the bundles.
                this.InstallBundleWithArguments(bundleA, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                this.InstallBundleWithArguments(bundleB, (int)MSIExec.MSIExecReturnCode.SUCCESS);

                // Make sure the MSIs and EXE are installed.
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsPackageInstalled(packageB));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", "1.0.0.0"));

                // Attempt to upgrade bundleA.
                this.InstallBundleWithArguments(bundleA1, (int)MSIExec.MSIExecReturnCode.SUCCESS);

                // Verify packageA1 was installed and packageA was uninstalled.
                Assert.IsTrue(this.IsPackageInstalled(packageA1));
                Assert.IsFalse(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", "1.0.1.0"));

                // Uninstall bundleA1 and verify that packageA1 is still installed.
                this.UninstallBundleWithArguments(bundleA1, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsTrue(this.IsPackageInstalled(packageA1));

                // Uninstall bundleB now.
                this.UninstallBundleWithArguments(bundleB, (int)MSIExec.MSIExecReturnCode.SUCCESS);

                // BUG: BundleB does not know about PackageA1 (A,v2), so remove it explicitly (SFBUG:3307315).
                MSIExec.UninstallProduct(packageA1, MSIExec.MSIExecReturnCode.SUCCESS);

                // Make sure the MSIs are not installed.
                Assert.IsFalse(this.IsPackageInstalled(packageB));
                Assert.IsFalse(this.IsPackageInstalled(packageA1));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", null));
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install bundle A, then upgrade it with a slipstream of package A and patch A.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_InstallUpgradeSlipstreamBundle()
        {
            // Build the packages.
            string packageA = BuildPackage("A", null);
            string packageA1 = BuildPackage("A", "1.0.1.0");
            string patchA = BuildPatch(packageA, packageA1, "PatchA", null);

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("patchA", patchA);

            // Build the bundles.
            string bundleA = this.BuildBundle("BundleA", null, bindPaths);
            string bundleC = this.BuildBundle("BundleC", "1.0.1.0", bindPaths);

                // Install the base bundle and make sure it's installed.
                this.InstallBundleWithArguments(bundleA, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", "1.0.0.0"));

                // Install the upgrade bundle with a slipstreamed patch and make sure the patch is installed.
                // SFBUG:3387046 - Uninstalling bundle registers a dependency on a package
                this.InstallBundleWithArguments(bundleC, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsTrue(MsiUtils.IsPatchInstalled(patchA));

                // BundleC doesn't carry the EXE, so make sure it's removed.
                Assert.IsTrue(this.IsRegistryValueEqual("Version", null));

                // Repair the upgrade bundle to make sure it does not prompt for source.
                // SFBUG:3386927 - MSIs get removed from cache during upgrade
                this.InstallBundleWithArguments(bundleC, (int)MSIExec.MSIExecReturnCode.SUCCESS, "-repair");

                // Uninstall the slipstream bundle and make sure both packages are uninstalled.
                this.UninstallBundleWithArguments(bundleC, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsFalse(MsiUtils.IsPatchInstalled(patchA));
                Assert.IsFalse(this.IsPackageInstalled(packageA));
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install bundle A, then install upgrade bundle D which will fail and roll back.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_RollbackUpgradeBundle()
        {
            // Build the packages.
            string packageA = BuildPackage("A", null);

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);

            // Build the bundles.
            string bundleA = this.BuildBundle("BundleA", null, bindPaths);
            string bundleD = this.BuildBundle("BundleD", "1.0.1.0", bindPaths);

                // Install the base bundle and make sure it's installed.
                this.InstallBundleWithArguments(bundleA, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", "1.0.0.0"));

                // Install the upgrade bundle that will fail and rollback. Make sure packageA is still present.
                // SFBUG:3405221 - pkg dependecy not removed in rollback if pkg already present
                this.InstallBundleWithArguments(bundleD, DependencyExtensionTests.E_INSTALLFAILURE);
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", "1.0.0.0"));

                // Uninstall the first bundle and make sure packageA is uninstalled.
                this.UninstallBundleWithArguments(bundleA, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsFalse(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", null));
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs an MSI then fails a non-vital package to test that the bundle still installs successfully.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_FailNonVitalPackage()
        {
            // Build the packages.
            string packageA = BuildPackage("A", null);
            string packageB = BuildPackage("Fail", null);

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("packageB", packageB);

            // Build the bundle.
            string bundleE = this.BuildBundle("BundleE", null, bindPaths);

                // Install the bundle and make sure packageA is installed.
                // SFBUG:3435047 - Make sure during install we don't fail for non-vital packages.
                this.InstallBundleWithArguments(bundleE, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsFalse(this.IsPackageInstalled(packageB));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", null));

                // Repair the bundle.
                // SFBUG:3435047 - Make sure during repair we don't fail for the same reason in a different code path.
                this.InstallBundleWithArguments(bundleE, (int)MSIExec.MSIExecReturnCode.SUCCESS, "-repair");
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsFalse(this.IsPackageInstalled(packageB));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", null));

                // Uninstall the bundle and make sure packageA is uninstalled.
                this.UninstallBundleWithArguments(bundleE, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsFalse(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", null));
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs a bundle, then an addon bundle, and uninstalls the main bundle.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_UninstallAddonBundle()
        {
            // Build the packages.
            string packageA = BuildPackage("A", null);
            string packageB = BuildPackage("B", null);

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("packageB", packageB);

            // Build the bundles.
            string bundleA1 = this.BuildBundle("BundleA", null, bindPaths);
            string bundleA2 = this.BuildBundle("BundleA", null, bindPaths);
            string bundleF = this.BuildBundle("BundleF", null, bindPaths);

                // Install the base bundle and make sure all packages are installed.
                this.InstallBundleWithArguments(bundleF, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsPackageInstalled(packageB));

                // Install an addon bundle and make sure all packages are installed.
                this.InstallBundleWithArguments(bundleA1, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsPackageInstalled(packageB));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", "1.0.0.0"));

                // Install a second addon bundle and make sure all packages are installed.
                this.InstallBundleWithArguments(bundleA2, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsPackageInstalled(packageB));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", "1.0.0.0"));

                // Uninstall the base bundle and make sure all packages are uninstalled.
                this.UninstallBundleWithArguments(bundleF, (int)MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsFalse(this.IsPackageInstalled(packageA));
                Assert.IsFalse(this.IsPackageInstalled(packageB));
                Assert.IsTrue(this.IsRegistryValueEqual("Version", null));
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs a bundle, then an addon bundle, and uninstalls the main bundle.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_InstallPatchBundle()
        {
            // Build the packages.
            string packageA1 = BuildPackage("A", null);
            string packageA2 = BuildPackage("A", "1.0.1.0");
            string packageB = BuildPackage("B", null);
            string patchA = BuildPatch(packageA1, packageA2, "PatchA", null);

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA1);
            bindPaths.Add("packageB", packageB);
            bindPaths.Add("patchA", patchA);

            // Build the bundles.
            string bundleF = this.BuildBundle("BundleF", null, bindPaths);
            string bundleG = this.BuildBundle("BundleG", null, bindPaths);

            // Install the base bundle and make sure all packages are installed.
            this.InstallBundleWithArguments(bundleF, (int)MSIExec.MSIExecReturnCode.SUCCESS);
            Assert.IsTrue(this.IsPackageInstalled(packageA1));
            Assert.IsTrue(this.IsPackageInstalled(packageB));

            // Install patch bundle and make sure all packages are installed.
            this.InstallBundleWithArguments(bundleG, (int)MSIExec.MSIExecReturnCode.SUCCESS);
            Assert.IsTrue(this.IsPackageInstalled(packageA1));
            Assert.IsTrue(this.IsPackageInstalled(packageB));
            Assert.IsTrue(MsiUtils.IsPatchInstalled(patchA));

            // Uninstall the base bundle and make sure all packages are uninstalled.
            this.UninstallBundleWithArguments(bundleF, (int)MSIExec.MSIExecReturnCode.SUCCESS);
            Assert.IsFalse(this.IsPackageInstalled(packageA1));
            Assert.IsFalse(this.IsPackageInstalled(packageB));
            Assert.IsFalse(MsiUtils.IsPatchInstalled(patchA));
        }

        /// <summary>
        /// Passes in per-test data to avoid collisions with failed tests when installing dependencies.
        /// </summary>
        /// <param name="name">The name of the source file (sans extension) to build.</param>
        /// <param name="version">The optional version to pass to the compiler.</param>
        /// <returns>The path to the build MSI package.</returns>
        private string BuildPackage(string name, string version)
        {
            PackageBuilder builder = new PackageBuilder(this, name) { Extensions = DependencyExtensionTests.Extensions };
            if (!String.IsNullOrEmpty(version))
            {
                builder.PreprocessorVariables.Add("Version", version);
            }

            return builder.Build().Output;
        }

        /// <summary>
        /// Builds a bundle uses named bind paths for packages.
        /// </summary>
        /// <param name="name">The name of the bundle to build.</param>
        /// <param name="version">Optional version for the bundle.</param>
        /// <param name="bindPaths">A dictionary of named bind paths.</param>
        /// <returns>The path to the bundle.</returns>
        private string BuildBundle(string name, string version, Dictionary<string, string> bindPaths)
        {
            BundleBuilder builder = new BundleBuilder(this, name) { BindPaths = bindPaths, Extensions = DependencyExtensionTests.Extensions };
            if (!String.IsNullOrEmpty(version))
            {
                builder.PreprocessorVariables.Add("Version", version);
            }

            return builder.Build().Output;
        }

        /// <summary>
        /// Builds a patch using given paths for the target and upgrade packages.
        /// </summary>
        /// <param name="targetPath">The path to the target MSI.</param>
        /// <param name="upgradePath">The path to the upgrade MSI.</param>
        /// <param name="name">The name of the patch to build.</param>
        /// <param name="version">Optional version for the bundle.</param>
        /// <returns>The path to the patch.</returns>
        private string BuildPatch(string targetPath, string upgradePath, string name, string version)
        {
            // Get the name of the calling method.
            StackTrace stack = new StackTrace();
            string caller = stack.GetFrame(1).GetMethod().Name;

            // Create paths.
            string source = Path.Combine(TestDataDirectory, String.Concat(name, ".wxs"));
            string rootDirectory = FileUtilities.GetUniqueFileName();
            string objDirectory = Path.Combine(rootDirectory, Settings.WixobjFolder);
            string msiDirectory = Path.Combine(rootDirectory, Settings.MSIFolder);
            string wixmst = Path.Combine(objDirectory, String.Concat(name, ".wixmst"));
            string wixmsp = Path.Combine(objDirectory, String.Concat(name, ".wixmsp"));
            string package = Path.Combine(msiDirectory, String.Concat(name, ".msp"));

            // Add the root directory to be cleaned up.
            this.TestArtifacts.Add(new DirectoryInfo(rootDirectory));

            // Compile.
            Candle candle = new Candle();
            candle.Extensions.AddRange(DependencyExtensionTests.Extensions);
            candle.OtherArguments = String.Concat("-dTestName=", caller);
            if (!String.IsNullOrEmpty(version))
            {
                candle.OtherArguments = String.Concat(candle.OtherArguments, " -dVersion=", version);
            }
            candle.OutputFile = String.Concat(objDirectory, @"\");
            candle.SourceFiles.Add(source);
            candle.WorkingDirectory = TestDataDirectory;
            candle.Run();

            // Make sure the output directory is cleaned up.
            this.TestArtifacts.Add(new DirectoryInfo(objDirectory));

            // Link.
            Light light = new Light();
            light.Extensions.AddRange(DependencyExtensionTests.Extensions);
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.OutputFile = wixmsp;
            light.SuppressMSIAndMSMValidation = true;
            light.WorkingDirectory = TestDataDirectory;
            light.Run();

            // Make sure the output directory is cleaned up.
            this.TestArtifacts.Add(new DirectoryInfo(msiDirectory));

            // Torch.
            Torch torch = new Torch();
            torch.TargetInput = Path.ChangeExtension(targetPath, "wixpdb");
            torch.UpdatedInput = Path.ChangeExtension(upgradePath, "wixpdb");
            torch.PreserveUnmodified = true;
            torch.XmlInput = true;
            torch.OutputFile = wixmst;
            torch.WorkingDirectory = TestDataDirectory;
            torch.Run();

            // Pyro.
            Pyro pyro = new Pyro();
            pyro.Baselines.Add(torch.OutputFile, name);
            pyro.InputFile = light.OutputFile;
            pyro.OutputFile = package;
            pyro.WorkingDirectory = TestDataDirectory;
            pyro.SuppressWarnings.Add("1079");
            pyro.Run();

            return pyro.OutputFile;
        }

        /// <summary>
        /// Gets whether the product defined by the package <paramref name="path"/> is installed.
        /// </summary>
        /// <param name="path">The path to the package to test.</param>
        /// <returns>True if the package is installed; otherwise, false.</returns>
        private bool IsPackageInstalled(string path)
        {
            string productCode = MsiUtils.GetMSIProductCode(path);
            return MsiUtils.IsProductInstalled(productCode);
        }

        /// <summary>
        /// Gets whether the two values are equal.
        /// </summary>
        /// <param name="name">The name of the registry value to check.</param>
        /// <param name="value">The value to check. Pass null to check if the registry value is missing.</param>
        /// <returns>True if the values are equal; otherwise, false.</returns>
        private bool IsRegistryValueEqual(string name, string value)
        {
            // Get the name of the calling method.
            StackTrace stack = new StackTrace();
            string caller = stack.GetFrame(1).GetMethod().Name;

            string key = String.Format(@"Software\WiX\Tests\{0}", caller);
            string data = null;

            using (RegistryKey reg = Registry.LocalMachine.OpenSubKey(key))
            {
                if (null != reg)
                {
                    object o = reg.GetValue(name);
                    if (null != o)
                    {
                        data = o.ToString();
                    }
                }
            }

            return String.Equals(data, value);
        }

        /// <summary>
        /// Installs the product and passes zero or more property settings.
        /// </summary>
        /// <param name="path">The path to the MSI to install.</param>
        /// <param name="expectedExitCode">The expected exit code.</param>
        /// <param name="properties">Zero or more properties to pass to the install.</param>
        /// <returns>The path to the log file.</returns>
        private string InstallProductWithProperties(string path, MSIExec.MSIExecReturnCode expectedExitCode, params string[] properties)
        {
            return RunMSIWithProperties(path, expectedExitCode, MSIExec.MSIExecMode.Install, properties);
        }

        /// <summary>
        /// Uninstalls the product and passes zero or more property settings.
        /// </summary>
        /// <param name="path">The path to the MSI to install.</param>
        /// <param name="expectedExitCode">The expected exit code.</param>
        /// <param name="properties">Zero or more properties to pass to the install.</param>
        /// <returns>The path to the log file.</returns>
        private string UninstallProductWithProperties(string path, MSIExec.MSIExecReturnCode expectedExitCode, params string[] properties)
        {
            return RunMSIWithProperties(path, expectedExitCode, MSIExec.MSIExecMode.Uninstall, properties);
        }

        /// <summary>
        /// Runs msiexec on the given <paramref name="path"/> with zero or more property settings.
        /// </summary>
        /// <param name="path">The path to the MSI to run.</param>
        /// <param name="expectedExitCode">The expected exit code.</param>
        /// <param name="mode">The installation mode for the MSI.</param>
        /// <param name="properties">Zero or more properties to pass to the install.</param>
        /// <returns>The path to the log file.</returns>
        private string RunMSIWithProperties(string path, MSIExec.MSIExecReturnCode expectedExitCode, MSIExec.MSIExecMode mode, params string[] properties)
        {
            MSIExec exec = new MSIExec();
            exec.ExecutionMode = mode;
            exec.ExpectedExitCode = expectedExitCode;
            exec.OtherArguments = null != properties ? String.Join(" ", properties) : null;
            exec.Product = path;

            // Get the name of the calling method.
            StackTrace stack = new StackTrace();
            string caller = stack.GetFrame(2).GetMethod().Name;

            // Generate the log file name.
            string logFile = String.Format("{0}_{1:yyyyMMddhhmmss}_{3}_{2}.log", caller, DateTime.UtcNow, Path.GetFileNameWithoutExtension(path), mode);
            exec.LogFile = Path.Combine(Path.GetTempPath(), logFile);

            exec.Run();

            return exec.LogFile;
        }

        /// <summary>
        /// Installs the bundle with optional arguments.
        /// </summary>
        /// <param name="path">Path to the bundle.</param>
        /// <param name="expectedExitCode">Expected exit code.</param>
        /// <param name="arguments">Optional arguments to pass to the tool.</param>
        /// <returns>Path to the generated log file.</returns>
        private string InstallBundleWithArguments(string path, int expectedExitCode, params string[] arguments)
        {
            return this.RunBundleWithArguments(path, expectedExitCode, MSIExec.MSIExecMode.Install, arguments);
        }

        /// <summary>
        /// Uninstalls the bundle with optional arguments.
        /// </summary>
        /// <param name="path">Path to the bundle.</param>
        /// <param name="expectedExitCode">Expected exit code.</param>
        /// <param name="arguments">Optional arguments to pass to the tool.</param>
        /// <returns>Path to the generated log file.</returns>
        private string UninstallBundleWithArguments(string path, int expectedExitCode, params string[] arguments)
        {
            return this.RunBundleWithArguments(path, expectedExitCode, MSIExec.MSIExecMode.Uninstall, arguments);
        }

        /// <summary>
        /// Executes the bundle with optional arguments.
        /// </summary>
        /// <param name="path">Path to the bundle.</param>
        /// <param name="expectedExitCode">Expected exit code.</param>
        /// <param name="mode">Install mode.</param>
        /// <param name="arguments">Optional arguments to pass to the tool.</param>
        /// <returns>Path to the generated log file.</returns>
        private string RunBundleWithArguments(string path, int expectedExitCode, MSIExec.MSIExecMode mode, params string[] arguments)
        {
            TestTool bundle = new TestTool(path, null);
            StringBuilder sb = new StringBuilder();

            // Be sure to run silent.
            sb.Append(" -quiet");

            // Get the name of the calling method.
            StackTrace stack = new StackTrace();
            string caller = stack.GetFrame(2).GetMethod().Name;

            // Generate the log file name.
            string logFile = String.Format("{0}_{1:yyyyMMddhhmmss}_{3}_{2}.log", caller, DateTime.UtcNow, Path.GetFileNameWithoutExtension(path), mode);
            sb.AppendFormat(" -log {0}", Path.Combine(Path.GetTempPath(), logFile));

            // Set operation.
            if (MSIExec.MSIExecMode.Uninstall == mode)
            {
                sb.Append(" -uninstall");
            }

            // Add additional arguments.
            if (null != arguments)
            {
                sb.Append(String.Join(" ", arguments));
            }

            // Set the arguments.
            bundle.Arguments = sb.ToString();

            // Run the tool and assert the expected code.
            bundle.ExpectedExitCode = expectedExitCode;
            bundle.Run();

            // Return the log file name.
            return logFile;
        }
    }
}
