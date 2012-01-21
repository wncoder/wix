//-----------------------------------------------------------------------
// <copyright file="UtilExtension.WixShellExecBinaryTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Util Extension WixShellExecBinary tests</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Extensions.UtilExtension
{
    using System;
    using System.IO;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;
   
    /// <summary>
    /// Util extension WixShellExecBinary element tests
    /// </summary>
    [TestClass]
    public class WixShellExecBinaryTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Extensions\UtilExtension\WixShellExecBinaryTests");
     
        [TestMethod]
        [Description("Verify that WixShellExecBinary executes the expected command.")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "true")]
        public void WixShellExecBinary_Install()
        {
            string sourceFile = Path.Combine(WixShellExecBinaryTests.TestDataDirectory, @"product.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixUtilExtension");

            string fileName = Environment.ExpandEnvironmentVariables(@"%TEMP%\DummyFile.txt");

            MSIExec.InstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);

            Assert.IsTrue(File.Exists(fileName) , "Command was not executed. File '{0}' does not exist.", fileName);

            MSIExec.UninstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);
        }
    }
}
