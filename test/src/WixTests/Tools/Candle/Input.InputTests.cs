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
// <summary>Test the different ways for giving input files to Candle</summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Tools.Candle.Input
{
    using System;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;
    
     /// <summary>
    /// Test the different ways for giving input files to Candle.
    /// </summary>
    [TestClass]
    public class InputTests : WixTests
    {
        private static readonly string TestDataDirectory = @"%WIX_ROOT%\test\Data\Tools\Candle\Input\InputTests";

        [TestMethod]
        [Description("Verify that Candle accepts a single Windows Installer XML source (wxs) file as input")]
        [Priority(1)]
        public void SingleWxsFile()
        {
            Candle.Compile(Path.Combine(WixTests.SharedAuthoringDirectory, "BasicProduct.wxs"));
        }

        [TestMethod]
        [Description("Verify that Candle accepts multiple Windows Installer XML source (wxs) files as input")]
        [Priority(1)]
        public void MultipleWxsFiles()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(WixTests.SharedAuthoringDirectory, "BasicProduct.wxs"));
            candle.SourceFiles.Add(Path.Combine(Path.Combine(InputTests.TestDataDirectory, @"MultipleWxsFiles\product.wxs")));
            candle.SourceFiles.Add(Path.Combine(Path.Combine(InputTests.TestDataDirectory, @"MultipleWxsFiles\feature.wxs")));
            candle.Run();
        }
                
        [TestMethod]
        [Description("Verify that Candle can accept a WXS file without the wxs extension as input")]
        [Priority(2)]
        public void NoWxsExtension()
        {
            Candle.Compile(Path.Combine(InputTests.TestDataDirectory, @"NoWxsExtension\Product"));
        }

        [TestMethod]
        [Description("Verify that Candle can accept a file with .foo as extension")]
        [Priority(2)]
        public void ValidFileWithUnknownExtension()
        {
            Candle.Compile(Path.Combine(InputTests.TestDataDirectory, @"ValidFileWithUnknownExtension\Product.foo"));
        }

        [TestMethod]
        [Description("Verify that Candle can handle invalid wxs, a non wix text file with a wxs extension")]
        [Priority(1)]
        public void InvalidWxsFile()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(InputTests.TestDataDirectory, @"InvalidWxsFile\Product.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(104, "Not a valid source file; detail: Data at the root level is invalid. Line 1, position 1.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 104;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that Candle can handle empty wxs file")]
        [Priority(3)]
        public void EmptyWxsFile()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(InputTests.TestDataDirectory, @"EmptyWxsFile\Product.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(104, "Not a valid source file; detail: Root element is missing.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 104;
            candle.Run();
        }
    }
}