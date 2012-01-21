//-----------------------------------------------------------------------
// <copyright file="Preprocessor.ErrorsAndWarningsTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Test how Candle handles preprocessing for errors and warnings.</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Candle.PreProcessor
{
    using System;
    using System.IO;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Test how Candle handles preprocessing for errors and warnings.
    /// </summary>
    [TestClass]
    public class ErrorsAndWarningsTests : WixTests
    {
        private static readonly string TestDataDirectory = @"%WIX_ROOT%\test\data\Tools\Candle\PreProcessor\ErrorsAndWarningsTests";

        [TestMethod]
        [Description("Verify that Candle can preprocess errors.")]
        [Priority(2)]
        public void Error()
        {
            string testFile = Environment.ExpandEnvironmentVariables(Path.Combine(ErrorsAndWarningsTests.TestDataDirectory, @"Error\Product.wxs"));
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
            string testFile = Environment.ExpandEnvironmentVariables(Path.Combine(ErrorsAndWarningsTests.TestDataDirectory, @"Warning\Product.wxs"));
            Candle candle = new Candle();
            candle.SourceFiles.Add(testFile);
            candle.ExpectedWixMessages.Add(new WixMessage(1096, "Preprocessor warning", WixMessage.MessageTypeEnum.Warning));
            candle.Run();
        }
    }
}