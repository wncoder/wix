//-----------------------------------------------------------------------
// <copyright file="Output.OutputTests.cs" company="Microsoft">
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
//     Test how Lit handles the Out switch. 
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Lit.Output
{
    using System;
    using System.IO;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Test how Lit handles the Out switch.
    /// </summary>
    [TestClass]
    public class OutputTests : WixTests
    {
        [TestMethod]
        [Description("Verify that the default behavior when the –out switch is not provided.")]
        [Priority(1)]
        public void DefaultOutput()
        {
            Lit lit = new Lit();
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.SetOutputFileIfNotSpecified = false;
            lit.OutputFile = string.Empty;
            lit.Run();
        }
        
        [TestMethod]
        [Description("Verify that Lit handles the -out switch and creates the wixobj in the specified directory.")]
        [Priority(1)]
        public void OutSwitch()
        {
            DirectoryInfo outputDirectory = Directory.CreateDirectory(Builder.GetUniqueFileName());

            Lit lit = new Lit();
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.OutputFile = Path.Combine(outputDirectory.FullName, "SimpleFragment.wix"); ;
            lit.Run();
            if (! File .Exists (Path.Combine(outputDirectory.FullName, "SimpleFragment.wix")))
            {
                Assert .Fail ("failed to handle -out swith of lit");
            }
        }

        [TestMethod]
        [Description("Verify that Lit can create that output directory and output the wix to that directory.")]
        [Priority(1)]
        public void NonExistingOutputDirectory()
        {
            string outputDirectory = Builder.GetUniqueFileName();

            Lit lit = new Lit();
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.OutputFile = Path.Combine(outputDirectory, "SimpleFragment.wix");
            lit.Run();
            if (!File.Exists(Path.Combine(outputDirectory, "SimpleFragment.wix")))
            {
                Assert.Fail("failed to create output directory specified in -out swith of lit");
            }
        }

        [TestMethod]
        [Description("Verify that the appropriate error message is generated for output filenames containing illegal characters.")]
        [Priority(2)]
        public void InvalidOutputFileName()
        {
            string[] invalidFileNames = new string[] { "testfile>wixobj", "testfile<wixobj", "testfile?wixobj", "testfile|wixobj", "testfile*wixobj" };
            string inputFile = Candle.Compile(WixTests.PropertyFragmentWxs);
            Lit lit;

            foreach (string invalidFileName in invalidFileNames)
            {
                lit = new Lit();
                lit.ObjectFiles.Add(inputFile);
                lit.OutputFile = invalidFileName;
                lit.SetOutputFileIfNotSpecified = false;
                string expectedOutput = string.Format("Invalid file name specified on the command line: '{0}'. Error message: 'Illegal characters in path.'", invalidFileName);
                lit.ExpectedWixMessages.Add(new WixMessage(284, expectedOutput, WixMessage.MessageTypeEnum.Error));
                lit.ExpectedExitCode = 284;
                lit.Run();
            }
        }

        [TestMethod]
        [Description("Verify that the appropriate error message is generated for output filenames containing double quotes.")]
        [Priority(2)]
        public void DoubleQuotesInOutputFileName()
        {
            Lit lit = new Lit();
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.OutputFile = "testfile\\\"wixobj";
            lit.SetOutputFileIfNotSpecified = false;
            string expectedOutput = string.Format("Your file or directory path '{0}' cannot contain a quote. Quotes are often accidentally introduced when trying to refer to a directory path with spaces in it, such as \"C:\\Out Directory\\\".  The correct representation for that path is: \"C:\\Out Directory\\\\\".", "testfile\"wixobj");
            lit.ExpectedWixMessages.Add(new WixMessage(117, expectedOutput, WixMessage.MessageTypeEnum.Error));
            lit.ExpectedExitCode = 117;
            lit.Run();
        }
    }
}