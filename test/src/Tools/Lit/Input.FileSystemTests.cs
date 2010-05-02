//-----------------------------------------------------------------------
// <copyright file="Input.FileSystemTests.cs" company="Microsoft">
//   Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>
//   Test how Lit handles different types of files
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Lit.Input
{
    using System;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Test how Lit handles different types of files.
    /// </summary>
    [TestClass]
    public class FileSystemTests : WixTests
    {
        [TestMethod]
        [Description("Verify that Lit accepts input file path given as relative path.")]
        [Priority(1)]
        public void RelativePath()
        {
            string temporaryDirectory = Builder.GetUniqueFileName();
            DirectoryInfo workingDirectory = Directory.CreateDirectory(Path.Combine(temporaryDirectory, @"RelativePath\WorkingDirectory"));
            DirectoryInfo objectFileDirectory = Directory.CreateDirectory(Path.Combine(temporaryDirectory, @"RelativePath\ObjectFileDirectory"));

            Candle candle = new Candle();
            candle.SourceFiles.Add(WixTests.PropertyFragmentWxs);
            candle.OutputFile = Path.Combine(objectFileDirectory.FullName, "Fragment.wixobj");
            candle.Run();

            Lit lit = new Lit(workingDirectory.FullName);
            lit.ObjectFiles.Add(@"..\ObjectFileDirectory\Fragment.wixobj");
            lit.Run();
        }
       
        [TestMethod]
        [Description("Verify that Lit can handle a non existing input file")]
        [Priority(2)]
        public void NonExistingInputFile()
        {
            string testFile = Path.Combine(Builder.GetUniqueFileName(), "foo.wixobj");

            Lit lit = new Lit();
            lit.ObjectFiles.Add(testFile);
            string outputString = String.Format("The system cannot find the file '{0}' with type 'Source'.", testFile);
            lit.ExpectedWixMessages.Add(new WixMessage(103, outputString, WixMessage.MessageTypeEnum.Error));
            lit.ExpectedExitCode = 103;
            lit.Run();
        }

        [TestMethod]
        [Description("Verify that Lit can accept non-alphanumeric characters in the filename")]
        [Priority(2)]
        public void NonAlphaNumericCharactersInFileName()
        {
            DirectoryInfo temporaryDirectory = Directory.CreateDirectory(Builder.GetUniqueFileName());

            Candle candle = new Candle();
            candle.SourceFiles.Add(WixTests.PropertyFragmentWxs);
            candle.OutputFile = Path.Combine(temporaryDirectory.FullName, "~!@#$%^&()_-=+,.wixobj");
            candle.Run();

            Lit lit = new Lit(candle);
            lit.Run();
        }

        [TestMethod]
        [Description("Verify that Lit can accept read only files as input")]
        [Priority(2)]
        public void ReadOnlyInputFile()
        {
            string testFile = Candle.Compile(WixTests.PropertyFragmentWxs);

            // Set the file to readonly
            File.SetAttributes(testFile, FileAttributes.ReadOnly);

            Lit lit = new Lit();
            lit.ObjectFiles.Add(testFile);
            lit.Run();
        }

        [TestMethod]
        [Description("Verify that Lit can accept files from a network share")]
        [Priority(2)]
        [Ignore]
        public void MultipleFilesFromNetworkShare()
        {
        }

        [TestMethod]
        [Description("Verify that Lit accepts s large size input file.")]
        [Priority(2)]
        [Ignore]
        public void LargeSizeInputFile()
        {
        }

        [TestMethod]
        [Description("Verify that Lit can accept file names with single quotes")]
        [Priority(3)]
        public void FileNameWithSingleQuotes()
        {
            DirectoryInfo temporaryDirectory = Directory.CreateDirectory(Builder.GetUniqueFileName());

            Candle candle = new Candle();
            candle.SourceFiles.Add(WixTests.PropertyFragmentWxs);
            candle.OutputFile = Path.Combine(temporaryDirectory.FullName, "'BasicProduct'.wixobj");
            candle.Run();

            Lit lit = new Lit(candle);
            lit.Run();
        }

        [TestMethod]
        [Description("Verify that Lit can accepts an input file with space in its name")]
        [Priority(3)]
        public void FileNameWithSpace()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(WixTests.PropertyFragmentWxs);
            candle.OutputFile = "SimpleFragment.wixobj";
            candle.Run();

            string testFile = Path.Combine(candle.OutputDirectory, "  Simple Fragment                           .wixobj");
            File.Copy(candle.OutputFile, testFile);

            Lit lit = new Lit();
            lit.ObjectFiles.Add(testFile);
            lit.Run();
        }

        [TestMethod]
        [Description("Verify that Lit fails gracefully in case of input file on a network share with no permissions.")]
        [Priority(3)]
        [Ignore]
        public void NetworkShareNoPermissions()
        {
        }
 
        [TestMethod]
        [Description("Verify that Lit can accept an input file from a URI path")]
        [Priority(3)]
        [Ignore]
        public void URI()
        {
        }
    }
}