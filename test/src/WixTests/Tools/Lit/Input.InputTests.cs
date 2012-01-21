//-----------------------------------------------------------------------
// <copyright file="Input.InputTests.cs" company="Microsoft">
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
//     Test how Lit handles different input files
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Lit.Input
{
    using System;
    using System.IO;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    ///  Test how Lit handles different input files
    /// </summary>
    [TestClass]
    public class InputTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Tools\Lit\Input\InputTests");

        [TestMethod]
        [Description("Verify that Lit accepts a single Windows Installer XML source (wxs) file as input")]
        [Priority(0)]
        public void SingleWixObjFile()
        {
            string testFile = Candle.Compile(WixTests.PropertyFragmentWxs);

            Lit lit = new Lit();
            lit.ObjectFiles.Add(testFile);
            lit.Run();
        }

        [TestMethod]
        [Description("Verify that Lit accepts multiple Windows Installer XML object (.wix) files as input")]
        [Priority(0)]
        public void MultipleWixObjFiles()
        {
            Lit lit = new Lit();
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.ObjectFiles.Add(Candle.Compile(Path.Combine(InputTests.TestDataDirectory, @"MultipleWixObjFiles\ComponentFragment.wxs")));
            lit.ObjectFiles.Add(Candle.Compile(Path.Combine(InputTests.TestDataDirectory, @"MultipleWixObjFiles\PropertyFragment.wxs")));
            lit.Run();
        }
               
        [TestMethod]
        [Description("Verify that Lit can accept a WixObj file without the .wixobj extension as input")]
        [Priority(2)]
        public void NoWixObjExtension()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(WixTests.PropertyFragmentWxs);
            candle.OutputFile = "Library";
            candle.Run();

            Lit lit = new Lit(candle);
            lit.Run();
        }

        [TestMethod]
        [Description("Verify that Lit can accept a file with .foo as extension")]
        [Priority(2)]
        public void ValidFileWithUnknownExtension()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(WixTests.PropertyFragmentWxs);
            candle.OutputFile = "Library.foo";
            candle.Run();

            Lit lit = new Lit(candle);
            lit.Run();
        }

        [TestMethod]
        [Description("Verify that Lit can handle invalid WixObj, a non wix text file with a wixobj extension")]
        [Priority(1)]
        public void InvalidWixObjFile()
        {
            Lit lit = new Lit();
            lit.ObjectFiles.Add(Path.Combine(InputTests.TestDataDirectory, @"InvalidWixObjFile\Library.wixobj"));
            lit.ExpectedWixMessages.Add(new WixMessage(104, "Not a valid object file; detail: Data at the root level is invalid. Line 1, position 1.", WixMessage.MessageTypeEnum.Error));
            lit.ExpectedExitCode = 104;
            lit.Run();
        }

        [TestMethod]
        [Description("Verify that Lit can handle invalid WixObj, a non wix text file with a wixobj extension")]
        [Priority(1)]
        public void WildcardInput()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(InputTests.TestDataDirectory, @"WildcardInput\PropertyFragment1.wxs"));
            candle.SourceFiles.Add(Path.Combine(InputTests.TestDataDirectory, @"WildcardInput\PropertyFragment2.wxs"));
            candle.SourceFiles.Add(Path.Combine(InputTests.TestDataDirectory, @"WildcardInput\TestPropertyFragment3.wxs"));
            candle.SourceFiles.Add(Path.Combine(InputTests.TestDataDirectory, @"WildcardInput\TestPropertyFragment4.wxs"));
            candle.Run();

            Lit lit = new Lit();
            lit.ObjectFiles.Add(Path.Combine(Path.GetDirectoryName(candle.OutputFile),@"PropertyFragment?.wixobj"));
            lit.ObjectFiles.Add(Path.Combine(Path.GetDirectoryName(candle.OutputFile),@"Test*.wixobj"));
            lit.Run();

            Verifier.VerifyWixLibProperty(lit.OutputFile, "Property1", "Property1_Value");
            Verifier.VerifyWixLibProperty(lit.OutputFile, "Property2", "Property2_Value");
            Verifier.VerifyWixLibProperty(lit.OutputFile, "Property3", "Property3_Value");
            Verifier.VerifyWixLibProperty(lit.OutputFile, "Property4", "Property4_Value");
        }

        [TestMethod]
        [Description("Verify that Lit can handle response file")]
        [Priority(3)]
        public void ResponseFile()
        {
            Lit lit = new Lit();
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.ResponseFile = Path.Combine(InputTests.TestDataDirectory, @"ResponseFile\ResponseFile.txt");
            lit.Run();

            // verify the loc file added by the @ResponseFile is read and added to the library
            Verifier.VerifyWixLibLocString(lit.OutputFile, "en-us", "String1", "String1(en-us)");
        }

        [TestMethod]
        [Description("Verify that Lit can handle empty wixobj file")]
        [Priority(3)]
        public void EmptyWixObjFile()
        {
            Lit lit = new Lit();
            lit.ObjectFiles.Add(Path.Combine(InputTests.TestDataDirectory, @"EmptyWixObjFile\EmptyFile.wixobj"));
            lit.ExpectedWixMessages.Add(new WixMessage(104, "Not a valid object file; detail: Root element is missing.", WixMessage.MessageTypeEnum.Error));
            lit.ExpectedExitCode = 104;
            lit.Run();
        }
    }
}