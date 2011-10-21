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
    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using System.Text;

    [TestClass]
    public sealed class DependencyExtensionTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Extensions\DependencyExtension\DependencyExtensionTests");

        // Tables and rows are verified in the auto-generated QTests.

        [TestMethod]
        [Priority(2)]
        [Description("Install products A then B, and uninstall in reverse order as normal.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_Install()
        {
            string packageA = BuildPackage("A", null);
            string packageB = BuildPackage("B", null);

            try
            {
                MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
                MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

                // Uninstall in reverse order.
                MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
                MSIExec.UninstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
            }
            finally
            {
                CleanupInstalledProduct(packageB);
                CleanupInstalledProduct(packageA);
            }
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install product B without dependency A installed and fail.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_MissingDependency()
        {
            string packageB = BuildPackage("B", null);

            try
            {
                string logB = MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.ERROR_INSTALL_FAILURE);
                Assert.IsTrue(LogVerifier.MessageInLogFile(logB, @"WixDependencyCheck:  The dependency ""Microsoft.WiX.DependencyExtension_MissingDependency.A,v1.0"" is missing or is not the required version."));
            }
            finally
            {
                CleanupInstalledProduct(packageB);
            }
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install product B without dependency A installed and override.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_MissingDependencyOverride()
        {
            string packageB = BuildPackage("B", null);

            try
            {
                string logB = InstallProductWithProperties(packageB, MSIExec.MSIExecReturnCode.SUCCESS, "DISABLEDEPENDENCYCHECK=1");
                Assert.IsTrue(LogVerifier.MessageInLogFile(logB, @"WixDependencyCheck:  Skipping the dependency check since DISABLEDEPENDENCYCHECK is set."));

                MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
            }
            finally
            {
                CleanupInstalledProduct(packageB);
            }
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install products A then B, and uninstall A while B is still present and appear to succeed.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_UninstallDependency()
        {
            string packageA = BuildPackage("A", null);
            string packageB = BuildPackage("B", null);

            try
            {
                MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
                MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

                // Now attempt the uninstall of dependency package A.
                string logA = MSIExec.UninstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsTrue(LogVerifier.MessageInLogFile(logA, @"WixDependencyCheck:  Found existing dependent"));

                // Uninstall in reverse order.
                MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
                MSIExec.UninstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
            }
            finally
            {
                CleanupInstalledProduct(packageB);
                CleanupInstalledProduct(packageA);
            }
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install products A then B, and uninstall A while B is still present and override.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_UninstallDependencyOverrideAll()
        {
            string packageA = BuildPackage("A", null);
            string packageB = BuildPackage("B", null);

            try
            {
                MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
                MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

                // Now attempt the uninstall of dependency package A.
                string logA = UninstallProductWithProperties(packageA, MSIExec.MSIExecReturnCode.SUCCESS, "IGNOREDEPENDENCIES=ALL");
                Assert.IsTrue(LogVerifier.MessageInLogFile(logA, @"WixDependencyCheck:  Skipping the dependencies check since IGNOREDEPENDENCIES contains ""ALL""."));

                MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
            }
            finally
            {
                CleanupInstalledProduct(packageB);
                CleanupInstalledProduct(packageA);
            }
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install products A then B, and uninstall A while B is still present and override.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_UninstallDependencyOverrideSpecific()
        {
            string packageA = BuildPackage("A", null);
            string packageB = BuildPackage("B", null);

            try
            {
                MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
                MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

                // Now attempt the uninstall of dependency package A.
                string productCode = MsiUtils.GetMSIProductCode(packageB);
                string ignoreDependencies = String.Concat("IGNOREDEPENDENCIES=", productCode);

                UninstallProductWithProperties(packageA, MSIExec.MSIExecReturnCode.SUCCESS, ignoreDependencies);

                MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
            }
            finally
            {
                CleanupInstalledProduct(packageB);
                CleanupInstalledProduct(packageA);
            }
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

            try
            {
                MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
                MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

                // Build the upgraded dependency A1 and make sure A was removed.
                MSIExec.InstallProduct(packageA1, MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsFalse(IsPackageInstalled(packageA));

                // Now attempt the uninstall of upgraded dependency package A1.
                string logA = MSIExec.UninstallProduct(packageA1, MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsTrue(LogVerifier.MessageInLogFile(logA, @"WixDependencyCheck:  Found existing dependent"));

                // Uninstall in reverse order.
                MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
                MSIExec.UninstallProduct(packageA1, MSIExec.MSIExecReturnCode.SUCCESS);
            }
            finally
            {
                CleanupInstalledProduct(packageA1);
                CleanupInstalledProduct(packageB);
                CleanupInstalledProduct(packageA);
            }
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

            try
            {
                // Install the bundles.
                this.InstallBundleWithArguments(bundleA, MSIExec.MSIExecReturnCode.SUCCESS);
                this.InstallBundleWithArguments(bundleB, MSIExec.MSIExecReturnCode.SUCCESS);

                // Make sure the MSIs are installed.
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsPackageInstalled(packageB));

                // Uninstall in reverse order.
                this.UninstallBundleWithArguments(bundleB, MSIExec.MSIExecReturnCode.SUCCESS);
                this.UninstallBundleWithArguments(bundleA, MSIExec.MSIExecReturnCode.SUCCESS);

                // Make sure the MSIs are not installed.
                Assert.IsFalse(this.IsPackageInstalled(packageB));
                Assert.IsFalse(this.IsPackageInstalled(packageA));
            }
            finally
            {
                this.CleanupInstalledBundle(bundleB);
                this.CleanupInstalledBundle(bundleA);

                this.CleanupInstalledProduct(packageB);
                this.CleanupInstalledProduct(packageA);
            }
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

            try
            {
                // Install the bundles.
                this.InstallBundleWithArguments(bundleA, MSIExec.MSIExecReturnCode.SUCCESS);
                this.InstallBundleWithArguments(bundleB, MSIExec.MSIExecReturnCode.SUCCESS);

                // Make sure the MSIs are installed.
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsPackageInstalled(packageB));

                // Attempt to uninstall bundleA.
                this.UninstallBundleWithArguments(bundleA, MSIExec.MSIExecReturnCode.SUCCESS);

                // Verify packageA is still installed.
                Assert.IsTrue(this.IsPackageInstalled(packageA));

                // Uninstall bundleB now.
                this.UninstallBundleWithArguments(bundleB, MSIExec.MSIExecReturnCode.SUCCESS);

                // Make sure the MSIs are not installed.
                Assert.IsFalse(this.IsPackageInstalled(packageB));
                Assert.IsFalse(this.IsPackageInstalled(packageA));
            }
            finally
            {
                this.CleanupInstalledBundle(bundleB);
                this.CleanupInstalledBundle(bundleA);

                this.CleanupInstalledProduct(packageB);
                this.CleanupInstalledProduct(packageA);
            }
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

            try
            {
                // Install the bundles.
                this.InstallBundleWithArguments(bundleA, MSIExec.MSIExecReturnCode.SUCCESS);
                this.InstallBundleWithArguments(bundleB, MSIExec.MSIExecReturnCode.SUCCESS);

                // Make sure the MSIs are installed.
                Assert.IsTrue(this.IsPackageInstalled(packageA));
                Assert.IsTrue(this.IsPackageInstalled(packageB));

                // Attempt to upgrade bundleA.
                this.InstallBundleWithArguments(bundleA1, MSIExec.MSIExecReturnCode.SUCCESS);

                // Verify packageA was uninstalled.
                Assert.IsFalse(this.IsPackageInstalled(packageA));

                // Uninstall bundleA1 and verify that packageA1 is still installed.
                this.UninstallBundleWithArguments(bundleA1, MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsTrue(this.IsPackageInstalled(packageA1));

                // Uninstall bundleB now.
                this.UninstallBundleWithArguments(bundleB, MSIExec.MSIExecReturnCode.SUCCESS);

                // BUG: BundleB does not know about PackageA1 (A,v2), so remove it explicitly.
                MSIExec.UninstallProduct(packageA1, MSIExec.MSIExecReturnCode.SUCCESS);

                // Make sure the MSIs are not installed.
                Assert.IsFalse(this.IsPackageInstalled(packageB));
                Assert.IsFalse(this.IsPackageInstalled(packageA1));
            }
            finally
            {
                this.CleanupInstalledBundle(bundleA1);
                this.CleanupInstalledBundle(bundleB);
                this.CleanupInstalledBundle(bundleA);

                this.CleanupInstalledProduct(packageA1);
                this.CleanupInstalledProduct(packageB);
                this.CleanupInstalledProduct(packageA);
            }
        }

        /// <summary>
        /// Passes in per-test data to avoid collisions with failed tests when installing dependencies.
        /// </summary>
        /// <param name="name">The name of the source file (sans extension) to build.</param>
        /// <param name="version">The optional version to pass to the compiler.</param>
        /// <returns>The path to the build MSI package.</returns>
        private string BuildPackage(string name, string version)
        {
            // Get the name of the calling method.
            StackTrace stack = new StackTrace();
            string caller = stack.GetFrame(1).GetMethod().Name;

            // Create paths.
            string source = Path.Combine(TestDataDirectory, String.Concat(name, ".wxs"));
            string rootDirectory = FileUtilities.GetUniqueFileName();
            string objDirectory = Path.Combine(rootDirectory, Settings.WixobjFolder);
            string msiDirectory = Path.Combine(rootDirectory, Settings.MSIFolder);
            string package = Path.Combine(msiDirectory, String.Concat(name, ".msi"));

            // Add the root directory to be cleaned up.
            this.TestArtifacts.Add(new DirectoryInfo(rootDirectory));

            // Compile.
            Candle candle = new Candle();
            candle.Extensions.Add("WixDependencyExtension");
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
            light.Extensions.Add("WixDependencyExtension");
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.OutputFile = package;
            light.SuppressMSIAndMSMValidation = true;
            light.WorkingDirectory = TestDataDirectory;
            light.Run();

            // Make sure the output directory is cleaned up.
            this.TestArtifacts.Add(new DirectoryInfo(msiDirectory));

            return light.OutputFile;
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
            // Get the name of the calling method.
            StackTrace stack = new StackTrace();
            string caller = stack.GetFrame(1).GetMethod().Name;

            // Create paths.
            string source = Path.Combine(TestDataDirectory, String.Concat(name, ".wxs"));
            string rootDirectory = FileUtilities.GetUniqueFileName();
            string objDirectory = Path.Combine(rootDirectory, Settings.WixobjFolder);
            string exeDirectory = Path.Combine(rootDirectory, "Bundles");
            string bundle = Path.Combine(exeDirectory, String.Concat(name, ".exe"));

            // Add the root directory to be cleaned up.
            this.TestArtifacts.Add(new DirectoryInfo(rootDirectory));

            // Compile.
            Candle candle = new Candle();
            candle.Extensions.Add("WixBalExtension");
            candle.Extensions.Add("WixDependencyExtension");
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

            // Add each named bind path.
            light.OtherArguments = String.Empty;
            foreach (KeyValuePair<string, string> bindPath in bindPaths)
            {
                if (null != bindPath.Value)
                {
                    string argument = String.Format(" -b {0}={1}", bindPath.Key, bindPath.Value);
                    light.OtherArguments = String.Concat(light.OtherArguments, argument);
                }
            }
            light.Extensions.Add("WixBalExtension");
            light.Extensions.Add("WixDependencyExtension");
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.OutputFile = bundle;
            light.WorkingDirectory = TestDataDirectory;
            light.Run();

            // Make sure the output directory is cleaned up.
            this.TestArtifacts.Add(new DirectoryInfo(exeDirectory));

            return light.OutputFile;
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

            exec.Run();

            return exec.LogFile;
        }

        /// <summary>
        /// Ensures the package specified by the <paramref name="path"/> is uninstalled without throwing an exception.
        /// </summary>
        /// <param name="path">The path to the MSI to uninstall.</param>
        private void CleanupInstalledProduct(string path)
        {
            MSIExec exec = new MSIExec();
            exec.ExecutionMode = MSIExec.MSIExecMode.Uninstall;
            exec.OtherArguments = "IGNOREDEPENDENCIES=ALL";
            exec.Product = path;

            exec.Run(false);
        }

        /// <summary>
        /// Installs the bundle with optional arguments.
        /// </summary>
        /// <param name="path">Path to the bundle.</param>
        /// <param name="expectedExitCode">Expected exit code.</param>
        /// <param name="arguments">Optional arguments to pass to the tool.</param>
        /// <returns>Path to the generated log file.</returns>
        private string InstallBundleWithArguments(string path, MSIExec.MSIExecReturnCode expectedExitCode, params string[] arguments)
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
        private string UninstallBundleWithArguments(string path, MSIExec.MSIExecReturnCode expectedExitCode, params string[] arguments)
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
        private string RunBundleWithArguments(string path, MSIExec.MSIExecReturnCode expectedExitCode, MSIExec.MSIExecMode mode, params string[] arguments)
        {
            TestTool bundle = new TestTool(path, null);
            StringBuilder sb = new StringBuilder();

            // Be sure to run silent.
            sb.Append(" -quiet");

            // Get the name of the calling method.
            StackTrace stack = new StackTrace();
            string caller = stack.GetFrame(2).GetMethod().Name;

            // Generate the log file name.
            string logFile = String.Format("{0}_{3:yyyyMMddhhmmss}_{2}_{1}.log", caller, Path.GetFileNameWithoutExtension(path), mode, DateTime.UtcNow);
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
            bundle.ExpectedExitCode = (int)expectedExitCode;
            bundle.Run();

            // Return the log file name.
            return logFile;
        }

        /// <summary>
        /// Ensures the bundle specified by the <paramref name="path"/> is uninstalled without throwing an exception.
        /// </summary>
        /// <param name="path">The path to the MSI to uninstall.</param>
        private void CleanupInstalledBundle(string path)
        {
            // Get the name of the calling method.
            StackTrace stack = new StackTrace();
            string caller = stack.GetFrame(1).GetMethod().Name;

            TestTool bundle = new TestTool(path, null);
            StringBuilder sb = new StringBuilder();

            // Run silent uninstall.
            sb.Append("-quiet -uninstall");

            // Generate the log file name.
            string logFile = String.Format("{0}_{2:yyyyMMddhhmmss}_Cleanup_{1}.log", caller, Path.GetFileNameWithoutExtension(path), DateTime.UtcNow);
            sb.AppendFormat(" -log {0}", Path.Combine(Path.GetTempPath(), logFile));

            bundle.Arguments = sb.ToString();
            bundle.Run();
        }

    }
}
