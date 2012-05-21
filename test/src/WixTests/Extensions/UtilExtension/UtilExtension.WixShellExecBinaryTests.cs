//-----------------------------------------------------------------------
// <copyright file="UtilExtension.WixShellExecBinaryTests.cs" company="Microsoft">
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
// <summary>Util Extension WixShellExecBinary tests</summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Extensions.UtilExtension
{
    using System;
    using System.IO;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using WixTest;
   
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
