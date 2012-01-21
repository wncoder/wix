//-----------------------------------------------------------------------
// <copyright file="UtilExtension.WixShellExecTests.cs" company="Microsoft">
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
// <summary>Util Extension WixShellExec tests</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Extensions.UtilExtension
{
    using System;
    using System.IO;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;
   
    /// <summary>
    /// Util extension WixShellExec element tests
    /// </summary>
    [TestClass]
    public class WixShellExecTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Extensions\UtilExtension\WixShellExecTests");
     
        [TestMethod]
        [Description("Verify that WixShellExec executes the expected command.")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "true")]
        public void WixShellExec_Install()
        {
            string sourceFile = Path.Combine(WixShellExecTests.TestDataDirectory, @"product.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixUtilExtension");

            string fileName = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), @"WixTestFolder\out.txt");
            if (File.Exists(fileName))
            {
                File.Delete(fileName);
            }

            MSIExec.InstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);

            Assert.IsTrue(File.Exists(fileName), "Command was not executed. File '{0}' does not exist.", fileName);

            MSIExec.UninstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);
        }
    }
}
