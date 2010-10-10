//-----------------------------------------------------------------------
// <copyright file="Files.CopyFileTests.cs" company="Microsoft">
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
// <summary>
//     Tests for the CopyFile element
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Files
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for the CopyFile element
    /// </summary>
    [TestClass]
    public class CopyFileTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Files\CopyFileTests");

        [TestMethod]
        [Description("Verify that a file that is installed can be copied")]
        [Priority(1)]
        public void CopyInstalledFile()
        {
            string sourceFile = Path.Combine(CopyFileTests.TestDataDirectory, @"CopyInstalledFile\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");
            string query = "SELECT `DestName` FROM `DuplicateFile` WHERE `FileKey` = 'copytest'";
            Verifier.VerifyQuery(msi, query, "copytest.txt");
        }

        [TestMethod]
        [Description("Verify that a file that is installed can be moved")]
        [Priority(1)]
        public void MoveInstalledFile()
        {
            string sourceFile = Path.Combine(CopyFileTests.TestDataDirectory, @"MoveInstalledFile\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");
            string query = "SELECT `SourceName` FROM `MoveFile` WHERE `FileKey` = 'copytest'";
            string query1 = "SELECT `Options` FROM `MoveFile` WHERE `FileKey` = 'copytest'";
            Verifier.VerifyQuery(msi, query, "TextFile1.txt");
            Verifier.VerifyQuery(msi, query1, "1");
        }

        [TestMethod]
        [Description("Verify that a file that is already on the machine can be copied")]
        [Priority(1)]
        public void CopyExistingFile()
        {
            string sourceFile = Path.Combine(CopyFileTests.TestDataDirectory, @"CopyExistingFile\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");
            string query = "SELECT `SourceName` FROM `MoveFile` WHERE `FileKey` = 'copytest'";
            string query1 = "SELECT `Options` FROM `MoveFile` WHERE `FileKey` = 'copytest'";
            Verifier.VerifyQuery(msi, query, "TextFile1.txt");
            Verifier.VerifyQuery(msi, query1, "0");
        }

        [TestMethod]
        [Description("Verify that a file that is already on the machine can be moved")]
        [Priority(1)]
        public void MoveExistingFile()
        {
            string sourceFile = Path.Combine(CopyFileTests.TestDataDirectory, @"MoveExistingFile\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");
            string query = "SELECT `SourceName` FROM `MoveFile` WHERE `FileKey` = 'copytest'";
            string query1 = "SELECT `Options` FROM `MoveFile` WHERE `FileKey` = 'copytest'";
            Verifier.VerifyQuery(msi, query, "TextFile1.txt");
            Verifier.VerifyQuery(msi, query1, "1");
        }


        [TestMethod]
        [Description("Verify that there is an error if FileId is not a defined file")]
        [Priority(3)]
        public void CopyNonExistingFile()
        {
            string sourceFile = Path.Combine(CopyFileTests.TestDataDirectory, @"CopyNonExistingFile\product.wxs");

            Candle candle = new Candle();
            candle.SourceFiles.Add(sourceFile);
            candle.Run();

            Light light = new Light(candle);
            light.ExpectedExitCode = 94;
            light.ExpectedWixMessages.Add(new WixMessage(94, "Unresolved reference to symbol 'File:test' in section 'Product:*'.", WixMessage.MessageTypeEnum.Error));
            light.Run();
        }
    }
}
