//-----------------------------------------------------------------------
// <copyright file="Warnings.SuppressWarningsTests.cs" company="Microsoft">
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
//     Tests for suppressing warnings
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Tools.Light.Warnings
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;

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