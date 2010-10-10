//-----------------------------------------------------------------------
// <copyright file="VSExtension.VSSetupTests.cs" company="Microsoft">
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
// <summary>VS Extension VSSetup tests</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Extensions.VSExtension
{
    using System;
    using System.IO;
    using System.Text;
    using System.Diagnostics;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Win32;

    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers.Extensions;

    /// <summary>
    /// NetFX extension VSSetup element tests
    /// </summary>
    [TestClass]
    public class VSExtensionTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Extensions\VSExtension\VSExtensionTests");
        private static readonly string DevenvRegistryKey = @"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\9.0\Setup\VS\";
        private static readonly string DevenvRegistryValueName = @"EnvironmentPath";
        private static string OutputFileName;
        private static string DevenvOriginalLocation;

        [ClassInitialize]
        public static void ClassInitialize(TestContext context)
        {
            // create a new command file
            string commandFileName = Path.Combine(Path.GetTempPath(), "stubdevenv.cmd");
            VSExtensionTests.OutputFileName = Builder.GetUniqueFileName();
            File.WriteAllText(commandFileName, string.Format("echo %* > {0}", VSExtensionTests.OutputFileName));

            // backup the original devenv.exe registry key first
            VSExtensionTests.DevenvOriginalLocation = (string)Registry.GetValue(VSExtensionTests.DevenvRegistryKey, VSExtensionTests.DevenvRegistryValueName, string.Empty);

            // replace the devenv.exe registry key  with the new command file
            Registry.SetValue(VSExtensionTests.DevenvRegistryKey, VSExtensionTests.DevenvRegistryValueName, commandFileName);
        }

        [TestMethod]
        [Description("Verify that the propject templates are installed to the correct folder on install")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "true")]
        public void VS90InstallVSTemplates_Install()
        {
            string sourceFile = Path.Combine(VSExtensionTests.TestDataDirectory, @"VS90InstallVSTemplates.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixVSExtension");

            MSIExec.InstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);

            Assert.IsTrue(File.Exists(VSExtensionTests.OutputFileName), "devenv.exe was not called");
            string acctualParamters = File.ReadAllText(VSExtensionTests.OutputFileName).Trim();
            string expectedParamters = "/InstallVSTemplates";
            Assert.IsTrue(acctualParamters.ToLowerInvariant().Equals(expectedParamters.ToLowerInvariant()), "devenv.exe was not called with the expected paramters. Acctual: '{0}'. Expected '{1}'.", acctualParamters, expectedParamters);

            MSIExec.UninstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);
        }

        [TestMethod]
        [Description("Verify that the propject templates are installed to the correct folder on install")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "true")]
        public void VSSetup_Install()
        {
            string sourceFile = Path.Combine(VSExtensionTests.TestDataDirectory, @"VS90Setup.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixVSExtension");

            MSIExec.InstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);

            Assert.IsTrue(File.Exists(VSExtensionTests.OutputFileName), "devenv.exe was not called");
            string acctualParamters = File.ReadAllText(VSExtensionTests.OutputFileName).Trim();
            string expectedParamters = "/setup";
            Assert.IsTrue(acctualParamters.ToLowerInvariant().Equals(expectedParamters.ToLowerInvariant()), "devenv.exe was not called with the expected paramters. Acctual: '{0}'. Expected '{1}'.", acctualParamters, expectedParamters);

            MSIExec.UninstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);
        }

        [TestCleanup]
        public override void CleanUp()
        {
            File.Delete(VSExtensionTests.OutputFileName);
            // make sure to call the base class cleanup method
            base.CleanUp();
        }

        [ClassCleanup]
        public static void ClassCleanUp()
        {
            // replace the devenv.exe registry key  with the original file
            Registry.SetValue(VSExtensionTests.DevenvRegistryKey, VSExtensionTests.DevenvRegistryValueName, VSExtensionTests.DevenvOriginalLocation);
        }
    }
}
