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
    using System.Diagnostics;
    using System.IO;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

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
            string packageA = BuildPackage("A");
            string packageB = BuildPackage("B");

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
            string packageB = BuildPackage("B");

            string logB = MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.ERROR_INSTALL_USEREXIT);
            Assert.IsTrue(LogVerifier.MessageInLogFile(logB, @"WixDependencyCheck:  The dependency ""Microsoft.WiX.DependencyExtension_MissingDependency.A,v1.0"" is missing or is not the required version."));
        }

        [TestMethod]
        [Priority(2)]
        [Description("Install product B without dependency A installed and override.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void DependencyExtension_MissingDependencyOverride()
        {
            string packageB = BuildPackage("B");

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
        public void DependencyExtension_UninstallDependent()
        {
            string packageA = BuildPackage("A");
            string packageB = BuildPackage("B");

            try
            {
                MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
                MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

                // Now attempt the uninstall of dependent package A.
                string logA = MSIExec.UninstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
                Assert.IsTrue(LogVerifier.MessageInLogFile(logA, @"WixDependencyCheck:  Found existing dependent ""{B8117EC4-D29D-45AB-8CBD-B0B6121886B1}v1.0""."));

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
        public void DependencyExtension_UninstallDependentOverrideAll()
        {
            string packageA = BuildPackage("A");
            string packageB = BuildPackage("B");

            try
            {
                MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
                MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

                // Now attempt the uninstall of dependent package A.
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
        public void DependencyExtension_UninstallDependentOverrideSpecific()
        {
            string packageA = BuildPackage("A");
            string packageB = BuildPackage("B");

            try
            {
                MSIExec.InstallProduct(packageA, MSIExec.MSIExecReturnCode.SUCCESS);
                MSIExec.InstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);

                // Now attempt the uninstall of dependent package A.
                UninstallProductWithProperties(packageA, MSIExec.MSIExecReturnCode.SUCCESS, "IGNOREDEPENDENCIES={B8117EC4-D29D-45AB-8CBD-B0B6121886B1}v1.0");

                MSIExec.UninstallProduct(packageB, MSIExec.MSIExecReturnCode.SUCCESS);
            }
            finally
            {
                CleanupInstalledProduct(packageB);
                CleanupInstalledProduct(packageA);
            }
        }

        /// <summary>
        /// Passes in per-test data to avoid collisions with failed tests when installing dependencies.
        /// </summary>
        /// <param name="name">The name of the source file (sans extension) to build.</param>
        /// <returns>The path to the build MSI package.</returns>
        private string BuildPackage(string name)
        {
            // Get the name of the calling method.
            StackTrace stack = new StackTrace();
            string caller = stack.GetFrame(1).GetMethod().Name;

            // Create paths.
            string source = Path.Combine(TestDataDirectory, String.Concat(name, ".wxs"));
            string rootDirectory = FileUtilities.GetUniqueFileName();
            string objDirectory = Path.Combine(rootDirectory, Settings.WixobjFolder);
            string package = Path.Combine(objDirectory, String.Concat(name, ".msi"));

            // Compile.
            Candle candle = new Candle();
            candle.Extensions.Add("WixDependencyExtension");
            candle.OtherArguments = String.Concat("-dTestName=", caller);
            candle.OutputFile = String.Concat(TestDataDirectory, @"\");
            candle.SourceFiles.Add(source);
            candle.WorkingDirectory = TestDataDirectory;
            candle.Run();

            // Link.
            Light light = new Light();
            light.Extensions.Add("WixDependencyExtension");
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.OutputFile = package;
            light.WorkingDirectory = TestDataDirectory;

            FileUtilities.CreateOutputDirectory(Path.GetDirectoryName(light.OutputFile));
            light.Run();

            return light.OutputFile;
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
    }
}
