//-----------------------------------------------------------------------
// <copyright file="Preprocessor.ErrorsAndWarningsTests.cs" company="Microsoft">
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
// <summary>Test how Candle handles preprocessing for errors and warnings.</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Candle.PreProcessor
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Test how Candle handles preprocessing for errors and warnings.
    /// </summary>
    [TestClass]
    public class ErrorsAndWarningsTests
    {
        [TestMethod]
        [Description("Verify that Candle can preprocess errors.")]
        [Priority(2)]
        public void Error()
        {
            string testFile = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Tools\Candle\PreProcessor\ErrorsAndWarningsTests\Error\Product.wxs");
            Candle candle = new Candle();
            candle.SourceFiles.Add(testFile);
            candle.ExpectedWixMessages.Add(new WixMessage(250, "Preprocessor error", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 250;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that Candle can preprocess warnings and continue.")]
        [Priority(2)]
        public void Warning()
        {
            string testFile = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Tools\Candle\PreProcessor\ErrorsAndWarningsTests\Warning\Product.wxs");
            Candle candle = new Candle();
            candle.SourceFiles.Add(testFile);
            candle.ExpectedWixMessages.Add(new WixMessage(1096, "Preprocessor warning", WixMessage.MessageTypeEnum.Warning));
            candle.Run();
        }
    }
}