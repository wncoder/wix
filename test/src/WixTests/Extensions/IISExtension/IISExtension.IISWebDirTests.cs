//-----------------------------------------------------------------------
// <copyright file="IISExtension.IISWebDirTests.cs" company="Microsoft">
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
// <summary>IIS Extension IISWebDir tests</summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Extensions.IISExtension
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using WixTest;
    using WixTest.Verifiers.Extensions;

    /// <summary>
    /// IIS extension IISWebDir element tests
    /// </summary>
    [TestClass]
    public class IISWebDirTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Extensions\IISExtension\IISWebDirTests");

        [TestMethod]
        [Description("Verify that the (IIsWebDir,CustomAction) Tables are created in the MSI and have expected data")]
        [Priority(1)]
        public void IISWebDir_VerifyMSITableData()
        {
            string sourceFile = Path.Combine(IISWebDirTests.TestDataDirectory, @"product.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixIIsExtension");

            Verifier.VerifyCustomActionTableData(msiFile,
                new CustomActionTableData("ConfigureIIs", 1, "IIsSchedule", "ConfigureIIs"),
                new CustomActionTableData("ConfigureIIsExec", 3073, "IIsSchedule", "ConfigureIIsExec"),
                new CustomActionTableData("StartMetabaseTransaction", 11265, "IIsExecute", "StartMetabaseTransaction"),
                new CustomActionTableData("RollbackMetabaseTransaction", 11521, "IIsExecute", "RollbackMetabaseTransaction"),
                new CustomActionTableData("CommitMetabaseTransaction", 11777, "IIsExecute", "CommitMetabaseTransaction"),
                new CustomActionTableData("WriteMetabaseChanges", 11265, "IIsExecute", "WriteMetabaseChanges"));

            Verifier.VerifyTableData(msiFile, MSITables.IIsWebDir,
                new TableRow(IISWebDirColumns.WebDir.ToString(), "testwebdir"),
                new TableRow(IISWebDirColumns.Component_.ToString(), "TestWebSiteProductComponent"),
                new TableRow(IISWebDirColumns.Web_.ToString(), "Test"),
                new TableRow(IISWebDirColumns.Path.ToString(), "webdir"),
                new TableRow(IISWebDirColumns.DirProperties_.ToString(), "ReadAndExecute"),
                new TableRow(IISWebDirColumns.Application_.ToString(), string.Empty)
                );
        }

        [TestMethod]
        [Description("Install the MSI. Verify that the web directory was created.Uninstall MSI . Verify that the web directory was removed ")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "true")]
        [TestProperty("OSquery", "124674")]     //Vista and Above 
        public void IISWebDir_Install()
        {
            string sourceFile = Path.Combine(IISWebDirTests.TestDataDirectory, @"product.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixIIsExtension");

            // Install  Msi
            MSIExec.InstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);

            // Verify WebDir was created
            Assert.IsTrue(IISVerifier.WebDirExist("webdir", "Test web server"), "WebDir '{0}' in site '{1}' was not created on Install", "webdir", "Test web server");

            // UnInstall Msi
            MSIExec.UninstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);

            // Verify WebDir was removed
            Assert.IsFalse(IISVerifier.WebDirExist("webdir", "Test web server"), "WebDir '{0}' in site '{1}' was not removed on Uninstall", "webdir", "Test web server");
        }

        [TestMethod]
        [Description("Install the MSI to a 64-bit specific loaction. Verify that the web directory was created. Uninstall MSI . Verify that the web directory was removed ")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "true")]
        [TestProperty("Is64BitSpecificTest", "true")]
        public void IISWebDir_Install_64bit()
        {
            string sourceFile = Path.Combine(IISWebDirTests.TestDataDirectory, @"product_64.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixIIsExtension");

            // Install  Msi
            MSIExec.InstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);

            // Verify WebDir was created
            Assert.IsTrue(IISVerifier.WebDirExist("webdir", "Test web server"), "WebDir '{0}' in site '{1}' was not created on Install", "webdir", "Test web server");

            // UnInstall Msi
            MSIExec.UninstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);

            // Verify WebDir was removed
            Assert.IsFalse(IISVerifier.WebDirExist("webdir", "Test web server"), "WebDir '{0}' in site '{1}' was not removed on Uninstall", "webdir", "Test web server");
        }

        [TestMethod]
        [Description("Cancel install of  MSI. Verify that the web directory was not created.")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "true")]
        public void IISWebDir_InstallFailure()
        {
            string sourceFile = Path.Combine(IISWebDirTests.TestDataDirectory, @"product_fail.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixIIsExtension");

            // Install  Msi
            MSIExec.InstallProduct(msiFile, MSIExec.MSIExecReturnCode.ERROR_INSTALL_FAILURE);
           
            // Verify WebDir was not created
            Assert.IsFalse(IISVerifier.WebDirExist("webdir", "Test web server"), "WebDir '{0}' in site '{1}' was created on failed install", "webdir", "Test web server");
        }
    }
}
