//-----------------------------------------------------------------------
// <copyright file="Warnings.SuppressWarningsTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests for suppressing warnings
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
    /// Tests for suppressing warnings
    /// </summary>
    [TestClass]
    public class SuppressWarningsTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Tools\Light\Warnings");

        [TestMethod]
        [Description("Verify that specific warnings can be suppressed")]
        [Priority(1)]
        public void SimpleSuppressWarnings()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Environment.ExpandEnvironmentVariables(Path.Combine(SuppressWarningsTests.TestDataDirectory, @"Shared\Warning1079.wxs")));
            candle.Run();

            Light light = new Light(candle);
            light.SuppressWarnings.Add("1079");
            light.Run();
        }
    }
}