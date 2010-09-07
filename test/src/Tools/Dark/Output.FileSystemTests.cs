//-----------------------------------------------------------------------
// <copyright file="Output.FileSystemTests.cs" company="Microsoft">
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
// <summary>Test how Dark handles different values for output file</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Dark.Output
{
    using System;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Test how Dark handles different values for output file.
    /// </summary>
    [TestClass]
    public class FileSystemTests : WixTests
    {
        [TestMethod]
        [Description("Verify that Dark generates the expected error when the output directory is has the same name as an existing file.")]
        [Priority(2)]
        public void InvalidOutputFile()
        {
            // make sure we have an existing file with the same name of dark output directory
            string invalidDirectoryName = Path.Combine(Environment.CurrentDirectory,"file.txt");
            System.IO.File.Create(invalidDirectoryName);

            Dark dark = new Dark();
            dark.InputFile = Path.Combine(WixTests.SharedBaselinesDirectory, @"MSIs\BasicProduct.msi");
            dark.OutputFile = Path.Combine(invalidDirectoryName,"output.wxs");
            string expectedErrorMessage = string.Format("Error writing to the path: '{0}'. Error message: 'Cannot create \"{1}\" because a file or directory with the same name already exists.'", dark.OutputFile, invalidDirectoryName);
            dark.ExpectedWixMessages.Add(new WixMessage(283, expectedErrorMessage, WixMessage.MessageTypeEnum.Error));
            dark.ExpectedExitCode = 283;
            dark.Run();
        }

        [TestMethod]
        [Description("Verify that the appropriate error message is generated for output filenames containing illegal characters.")]
        [Priority(2)]
        public void InvalidOutputFileName()
        {
            string[] invalidFileNames = new string[] { "testfile>msi", "testfile<msi", "testfile?msi", "testfile|msi", "testfile*msi" };
            Dark dark;

            foreach (string invalidFileName in invalidFileNames)
            {
                dark = new Dark();
                dark.InputFile = Path.Combine(WixTests.SharedBaselinesDirectory, @"MSIs\BasicProduct.msi");
                dark.OutputFile = string.Empty;
                dark.OtherArguments = " -out " + invalidFileName;
                string expectedOutput = string.Format("Invalid file name specified on the command line: '{0}'. Error message: 'Illegal characters in path.'", invalidFileName);
                dark.ExpectedWixMessages.Add(new WixMessage(284, expectedOutput, WixMessage.MessageTypeEnum.Error));
                dark.ExpectedExitCode = 284;
                dark.Run();
            }

            // test for quotes
            dark = new Dark();
            dark.InputFile = Path.Combine(WixTests.SharedBaselinesDirectory, @"MSIs\BasicProduct.msi");
            dark.OutputFile = string.Empty;
            dark.OtherArguments = " -out testfile\\\"msi";
            string expectedOutput2 = string.Format("Your file or directory path '{0}' cannot contain a quote. Quotes are often accidentally introduced when trying to refer to a directory path with spaces in it, such as \"C:\\Out Directory\\\".  The correct representation for that path is: \"C:\\Out Directory\\\\\".", "testfile\"msi");
            dark.ExpectedWixMessages.Add(new WixMessage(117, expectedOutput2, WixMessage.MessageTypeEnum.Error));
            dark.ExpectedExitCode = 117;
            dark.Run();
        }
    }
}