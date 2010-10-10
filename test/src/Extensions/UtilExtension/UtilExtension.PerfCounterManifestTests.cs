//-----------------------------------------------------------------------
// <copyright file="UtilExtension.PerfCounterManifestTests.cs" company="Microsoft">
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
// <summary>Util Extension PerfCounterManifest tests</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Extensions.UtilExtension
{
    using System;
    using System.IO;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers.Extensions;
    
    using Microsoft.Win32;
   
    /// <summary>
    /// Util extension PerfCounterManifest element tests
    /// </summary>
    [TestClass]
    public class PerfCounterManifestTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Extensions\UtilExtension\PerfCounterManifestTests");

        [TestMethod]
        [Description("Verify that the (PerfCounterManifest and CustomAction) Tables are created in the MSI and have expected data.")]
        [Priority(1)]
        public void PerfCounterManifest_VerifyMSITableData()
        {
            string sourceFile = Path.Combine(PerfCounterManifestTests.TestDataDirectory, @"product.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixUtilExtension");

            Verifier.VerifyCustomActionTableData(msiFile,
                new CustomActionTableData("ConfigurePerfmonManifestRegister", 1, "ScaSchedule", "ConfigurePerfmonManifestRegister"),
                new CustomActionTableData("ConfigurePerfmonManifestUnregister", 1, "ScaSchedule", "ConfigurePerfmonManifestUnregister"),
                new CustomActionTableData("RegisterPerfmonManifest", 3073, "WixCA", "CAQuietExec"),
                new CustomActionTableData("UnregisterPerfmonManifest", 3137, "WixCA", "CAQuietExec"),
                new CustomActionTableData("RollbackRegisterPerfmonManifest", 3393, "WixCA", "CAQuietExec"),
                new CustomActionTableData("RollbackUnregisterPerfmonManifest", 3329, "WixCA", "CAQuietExec")
                );

            Verifier.VerifyTableData(msiFile, MSITables.PerfmonManifest,
                new TableRow(PerfmonManifestColumns.Component_.ToString(), "TestPerfmonManifestProductComponent"),
                new TableRow(PerfmonManifestColumns.File.ToString(), "[#SymFile]"),
                new TableRow(PerfmonManifestColumns.ResourceFileDirectory.ToString(), "[TestPerfmonProductDirectory]"));
        }

        [TestMethod]
        [Description("Verify that the Performance counter manifest is created upon install.")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "true")]
        public void PerfCounterManifest_Install()
        {
            string sourceFile = Path.Combine(PerfCounterManifestTests.TestDataDirectory, @"product.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixUtilExtension");

            MSIExec.InstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);

            string keyName = @"SOFTWARE\Microsoft\Windows NT\CurrentVersion\Perflib\_V2Providers\{42aaeb49-78e7-4d78-81a0-6f35bfde65bc}";
            string valueName = "ProviderName";
            string expectedValue = "ApServerPerfmon";

            RegistryVerifier.VerifyRegistryKeyValue(RegistryHive.LocalMachine, keyName, valueName, expectedValue);

            MSIExec.UninstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);

            // Verify that the file was removed
            Assert.IsFalse(RegistryVerifier.RegistryKeyExists(RegistryHive.LocalMachine, keyName), "Registry Key '{0}' was not removed on uninstall.", keyName);
        }

        [TestMethod]
        [Description("Verify that the Performance counter manifest was removed upon rollback.")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "true")]
        public void PerfCounterManifest_InstallFailure()
        {
            string sourceFile = Path.Combine(PerfCounterManifestTests.TestDataDirectory, @"product_fail.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixUtilExtension");

            MSIExec.InstallProduct(msiFile, MSIExec.MSIExecReturnCode.ERROR_INSTALL_FAILURE);

            string keyName = @"SOFTWARE\Microsoft\Windows NT\CurrentVersion\Perflib\_V2Providers\{42aaeb49-78e7-4d78-81a0-6f35bfde65bc}";

            // Verify that the file was removed
            Assert.IsFalse(RegistryVerifier.RegistryKeyExists(RegistryHive.LocalMachine, keyName), "Registry Key '{0}' was not removed on uninstall.", keyName);
        }
    }
}
