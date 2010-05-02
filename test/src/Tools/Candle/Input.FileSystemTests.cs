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
// <summary>Test how Candle handles different types of files</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Candle.Input
{
    using System;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Test how Candle handles different types of files.
    /// </summary>
    [TestClass]
    public class FileSystemTests
    {
        [TestMethod]
        [Description("Verify that Candle fails gracefully in case of input file on a network share with no permissions.")]
        [Priority(3)]
        [Ignore]
        public void NetworkShareNoPermissions()
        {
        }

        [TestMethod]
        [Description("Verify that Candle accepts input file path given as relative path.")]
        [Priority(1)]
        public void RelativePath()
        {
            string temporaryDirectory = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());

            DirectoryInfo workingDirectory = Directory.CreateDirectory(Path.Combine(temporaryDirectory, @"RelativePath\WorkingDirectory"));
            DirectoryInfo sourceFileDirectory = Directory.CreateDirectory(Path.Combine(temporaryDirectory, @"RelativePath\SourceFileDirectory"));

            string sourceFile = Path.Combine(WixTests.SharedAuthoringDirectory, "BasicProduct.wxs");
            string destinationFile = Path.Combine(sourceFileDirectory.FullName, "BasicProduct.wxs");
            File.Copy(sourceFile, destinationFile);

            Candle candle = new Candle(workingDirectory.FullName);
            candle.SourceFiles.Add(@"..\SourceFileDirectory\BasicProduct.wxs");
            candle.Run();
        }
       
        [TestMethod]
        [Description("Verify that Candle can handle a non existing input file")]
        [Priority(2)]
        public void NonExistingInputFile()
        {
            // Retrieving the path to a temporary directory
            string temporaryDirectory = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
            string testFile = Path.Combine(temporaryDirectory, "foo.wxs");
            Candle candle = new Candle();
            candle.SourceFiles.Add(testFile);
            string outputString = String.Format("The system cannot find the file '{0}' with type 'Source'.", testFile);
            candle.ExpectedWixMessages.Add(new WixMessage(103, outputString, WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 103;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that Candle can accept alphanumeric characters in the filename")]
        [Priority(2)]
        public void NonAlphaNumericCharactersInFileName()
        {
            DirectoryInfo temporaryDirectory = Directory.CreateDirectory(Path.Combine(Path.GetTempPath(), Path.GetRandomFileName()));
            string sourceFile = Path.Combine(WixTests.SharedAuthoringDirectory, "BasicProduct.wxs");
            string destinationFile = Path.Combine(temporaryDirectory.FullName, "#@%+BasicProduct.wxs");
            File.Copy(sourceFile, destinationFile, true);
            Candle.Compile(destinationFile);
        }

        [TestMethod]
        [Description("Verify that Candle can accept read only files as input")]
        [Priority(2)]
        public void ReadOnlyInputFile()
        {
            // Retrieving the path to a temporary directory
            string testFile = Path.Combine(Path.GetTempPath(), String.Concat(Path.GetTempFileName(),".wxs"));
            string sourceFile = Path.Combine(WixTests.SharedAuthoringDirectory, "BasicProduct.wxs");
            File.Copy(sourceFile, testFile, true);

            // Set the file to readonly
            File.SetAttributes(testFile, FileAttributes.ReadOnly);

            Candle.Compile(testFile);
        }

        [TestMethod]
        [Description("Verify that Candle can accept files from a network share")]
        [Priority(2)]
        [Ignore]
        public void MultipleFilesFromNetworkShare()
        {
        }

        [TestMethod]
        [Description("Verify that Candle accepts s large size input file.")]
        [Priority(2)]
        [Ignore]
        public void LargeSizeInputFile()
        {
        }

        [TestMethod]
        [Description("Verify that Candle can accept file names with single quotes")]
        [Priority(3)]
        public void FileNameWithSingleQuotes()
        {
            string testFile = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Tools\Candle\Input\FileSystemTests\FileNameWithSingleQuotes\'Product'.wxs");
            Candle.Compile(testFile);
        }
               
        [TestMethod]
        [Description("Verify that Candle can accepts a small size input file")]
        [Priority(3)]
        [Ignore]
        public void SmallSizeInputFile()
        {
        }

        [TestMethod]
        [Description("Verify that Candle can accepts an input file with space in its name")]
        [Priority(3)]
        [Ignore]
        public void FileNameWithSpace()
        {
        }

        [TestMethod]
        [Description("Verify that Candle accepts an input file path that is more than 256 characters")]
        [Priority(3)]
        [Ignore]
        public void LongFilePath()
        {
        }
              
        [TestMethod]
        [Description("Verify that Candle can accept an input file from a URI path")]
        [Priority(3)]
        [Ignore]
        public void URI()
        {
        }
    }
}