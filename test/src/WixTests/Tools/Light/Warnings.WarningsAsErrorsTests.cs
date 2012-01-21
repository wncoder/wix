//-----------------------------------------------------------------------
// <copyright file="Warnings.WarningsAsErrorsTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests for the -wx switch
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Light.Warnings
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for the -wx switch
    /// </summary>
    [TestClass]
    public class WarningsAsErrorsTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Tools\Light\Warnings");

        [TestMethod]
        [Description("Verify that warnings are treated as errors")]
        [Priority(1)]
        public void SimpleWarningsAsErrors()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(WarningsAsErrorsTests.TestDataDirectory, @"Shared\Warning1079.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.TreatAllWarningsAsErrors = true;
            light.ExpectedWixMessages.Add(new WixMessage(1079, WixMessage.MessageTypeEnum.Error));
            light.ExpectedExitCode = 1079;
            light.Run();

        }
    }
}