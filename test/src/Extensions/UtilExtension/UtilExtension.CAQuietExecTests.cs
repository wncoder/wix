//-----------------------------------------------------------------------
// <copyright file="UtilExtension.CAQuietExecTests.cs" company="Microsoft">
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
// <summary>Util Extension CAQuietExec tests</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Extensions.UtilExtension
{
    using System;
    using System.IO;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;
   
    /// <summary>
    /// Util extension CAQuietExec element tests
    /// </summary>
    [TestClass]
    public class CAQuietExecTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Extensions\UtilExtension\CAQuietExecTests");
     
        [TestMethod]
        [Description("Verify that CAQuietExec executes the expected command.")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "true")]
        public void CAQuietExec_Install()
        {
            string sourceFile = Path.Combine(CAQuietExecTests.TestDataDirectory, @"product.wxs");
           
            string immediateFileName = Path.Combine(Path.GetTempPath(), "ImmediateCommand.cmd");
            string immediateOutputFileName = Path.Combine(Path.GetTempPath(), "immediate.txt");
          
            string deferredFileName = Path.Combine(Path.GetTempPath(), "DeferredCommand.cmd");
            string deferredOutputFileName = Path.Combine(Path.GetTempPath(), "deferred.txt");

            File.Copy(Path.Combine(CAQuietExecTests.TestDataDirectory, @"ImmediateCommand.cmd"), immediateFileName, true);
            File.Copy(Path.Combine(CAQuietExecTests.TestDataDirectory, @"DeferredCommand.cmd"), deferredFileName, true);
            
            if (File.Exists(immediateOutputFileName))
            {
                File.Delete(immediateOutputFileName);
            }
            if (File.Exists(deferredOutputFileName))
            {
                File.Delete(deferredOutputFileName);
            }
            
            string msiFile = Builder.BuildPackage(Environment.CurrentDirectory, sourceFile, "test.msi", string.Format("-dimmediate={0} -ddeferred={1} -ext WixUtilExtension",immediateFileName, deferredFileName), "-ext WixUtilExtension");

            MSIExec.InstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);

            Assert.IsTrue(File.Exists(immediateOutputFileName), "Immediate Command was not executed. File '{0}' does not exist.", immediateOutputFileName);
            Assert.IsTrue(File.Exists(deferredOutputFileName), "Deferred Command was not executed. File '{0}' does not exist.", deferredOutputFileName);

            MSIExec.UninstallProduct(msiFile, MSIExec.MSIExecReturnCode.SUCCESS);
        }
    }
}
