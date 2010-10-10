//------------------------------------------------------------------------------------------------------------------------
// <copyright file="Output.FileSystemTests.cs" company="Microsoft">
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
// <summary>Test Candle to verify that it interacts appropriately with the file system to produce output files. </summary>
//------------------------------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Candle.Output
{
    using System;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Test Candle to verify that it interacts appropriately with the file system to produce output files.
    /// </summary>
    [TestClass]
    public class FileSystemTests : WixTests
    {
        [TestMethod]
        [Description("Verify that Candle fails gracefully in case of creating a output file on a network share with no permissions.")]
        [Priority(2)]
        [Ignore]
        public void NetworkShareNoPermissions()
        {
        }

        [TestMethod]
        [Description("Verify that Candle can output nonalphanumeric characters in the filename")]
        [Priority(2)]
        public void NonAlphaNumericCharactersInFileName()
        {
            Candle candle = new Candle();
            candle.OutputFile = Path.Combine(Builder.GetUniqueFileName(), "#@%+BasicProduct.wixobj");
            candle.SourceFiles.Add(Path.Combine(Tests.WixTests.SharedAuthoringDirectory, "BasicProduct.wxs"));
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that Candle can output files to a read only share")]
        [Priority(2)]
        [Ignore]
        public void ReadOnlyShare()
        {
        }

        [TestMethod]
        [Description("Verify that Candle can output files to a network share")]
        [Priority(2)]
        [Ignore]
        public void FileToNetworkShare()
        {
        }

        [TestMethod]
        [Description("Verify that Candle can output file names with single quotes")]
        [Priority(3)]
        public void FileNameWithSingleQuotes()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(Tests.WixTests.SharedAuthoringDirectory, "BasicProduct.wxs"));
            candle.OutputFile = Path.Combine(Builder.GetUniqueFileName(), "Basic'Product'.wixobj");
            candle.Run();
        }
               
        [TestMethod]
        [Description("Verify that Candle can output a file with space in its name.")]
        [Priority(3)]
        public void FileNameWithSpace()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(Tests.WixTests.SharedAuthoringDirectory, "BasicProduct.wxs"));
            candle.OutputFile = Path.Combine(Builder.GetUniqueFileName(), "Pro  duct.wixobj");
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that Candle output to a file path that is more than 256 characters.")]
        [Priority(3)]
        public void LongFilePath()
        {
            //the max length of a path is 247 chars, the filepath is 259 chars

            string OutputPath = Builder.GetUniqueFileName();
            //add initial 170 chars
            OutputPath = Path.Combine(OutputPath, @"FilePathNewfolder11(20chars)\Newfolder12(20chars)Newfolder12(20chars)Newfolder13(20chars)Newfolder13(20chars)\Newfolder14(20chars)Newfolder15(20chars)Newfolder16(20chars)");

            int i = 245 - OutputPath.Length;
            while (i > 0)
            {
                OutputPath = Path.Combine(OutputPath, @"pt");
                i = 245 - OutputPath.Length;
            }
            if (OutputPath.Length < 246)
            {
                OutputPath = Path.Combine(OutputPath, "T");
            }

            Assert.IsTrue(OutputPath.Length  <248, "The output path is not less than 248 chars"); 
            Directory.CreateDirectory(OutputPath);

            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(Tests.WixTests.SharedAuthoringDirectory, "BasicProduct.wxs"));
            candle.OutputFile = Path.Combine(OutputPath, "Prod.wixobj");

            Assert.IsTrue((candle.OutputFile.Length > 256) && (candle.OutputFile.Length < 260), "The output filepath{0} is not between 256 and 260 chars",candle .OutputFile .Length );
           
            candle.Run();
        }
              
        [TestMethod]
        [Description("Verify that Candle can output a file to a URI path.")]
        [Priority(3)]
        [Ignore]
        public void URI()
        {
        }
    }
}