//-----------------------------------------------------------------------
// <copyright file="Binaries.BinaryTests.cs" company="Microsoft">
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
//     Tests for embedded binaries
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Integration.BuildingPackages.Binaries
{
    using System;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;

    /// <summary>
    /// Tests for embedded binaries
    /// </summary>
    [TestClass]
    public class BinaryTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Binaries\BinaryTests");

        [TestMethod]
        [Description("Verify that binary data can be embedded in an MSI")]
        [Priority(1)]
        public void SimpleBinary()
        {
            string msi = Builder.BuildPackage(Path.Combine(BinaryTests.TestDataDirectory, @"SimpleBinary\product.wxs"));
            Verifier.VerifyResults(Path.Combine(BinaryTests.TestDataDirectory, @"SimpleBinary\expected.msi"), msi, "Binary");

            string binaryPathDirectory = Utilities.FileUtilities.GetUniqueFileName();
            Directory.CreateDirectory(binaryPathDirectory);

            Dark dark = new Dark();
            dark.InputFile = msi;
            dark.OutputFile = "decompiled.wxs";
            dark.BinaryPath = binaryPathDirectory;
            dark.Run();
        }

        [TestMethod]
        [Description("Verify that a large file 512M(512,000,000 bytes) can be embedded in an MSI")]
        [Priority(2)]
        public void LargeBinary()
        {
            //create a file in temp direction with 0 size
            string filename = Utilities.FileUtilities.CreateSizedFile(512000000);
            string sourceFile = Path.Combine(BinaryTests.TestDataDirectory, @"LargeBinary\product.wxs");

            Candle candle = new Candle();
            candle.SourceFiles.Add(sourceFile);
            candle.PreProcessorParams.Add("MyVariable", filename);
            candle.Run();

            Light light = new Light(candle);
            light.Run();

            string query1 = "SELECT `FileSize` FROM `File` WHERE `File` = 'File1'";
            Verifier.VerifyQuery(light.OutputFile, query1, "512000000");

            File.Delete(filename);
        }

        [TestMethod]
        [Description("Verify that a 0 byte file can be embedded in an MSI")]
        [Priority(3)]
        public void SmallBinary()
        {
            //create a file in temp direction with 0 size
            string filename = Utilities.FileUtilities.CreateSizedFile(0);
            string sourceFile = Path.Combine(BinaryTests.TestDataDirectory, @"LargeBinary\product.wxs");

            Candle candle = new Candle();
            candle.SourceFiles.Add(sourceFile);
            candle.PreProcessorParams.Add("MyVariable", filename);
            candle.Run();

            Light light = new Light(candle);
            light.Run();

            string query1 = "SELECT `FileSize` FROM `File` WHERE `File` = 'File1'";
            Verifier.VerifyQuery(light .OutputFile , query1, "0");
            
            File.Delete(filename);
        }
    }
}
