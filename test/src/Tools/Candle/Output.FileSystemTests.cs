//------------------------------------------------------------------------------------------------------------------------
// <copyright file="Output.FileSystemTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
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
    public class FileSystemTests
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
            string outputDirectory = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
            candle.OutputFile = Path.Combine(outputDirectory, "#@%+BasicProduct.wixobj");
            candle.SourceFiles.Add(Path.Combine(Tests.WixTests.SharedAuthoringDirectory, "BasicProduct.wxs"));
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that Candle can output files to a read only share")]
        [Priority(2)]
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
            string outputDirectory = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
            candle.SourceFiles.Add(Path.Combine(Tests.WixTests.SharedAuthoringDirectory, "BasicProduct.wxs"));
            candle.OutputFile = Path.Combine(outputDirectory, "Basic'Product'.wixobj");
            candle.Run();
        }
               
        [TestMethod]
        [Description("Verify that Candle can output a file with space in its name.")]
        [Priority(3)]
        [Ignore]
        public void FileNameWithSpace()
        {
        }

        [TestMethod]
        [Description("Verify that Candle output to a file path that is more than 256 characters.")]
        [Priority(3)]
        [Ignore]
        public void LongFilePath()
        {
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