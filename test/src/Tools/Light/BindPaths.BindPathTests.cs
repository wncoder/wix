//-----------------------------------------------------------------------
// <copyright file="BindPaths.BindPathTests.cs" company="Microsoft">
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
// <summary>
//     Test for setting bind paths
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Light.BindPaths
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Test for setting bind paths
    /// </summary>
    [TestClass]
    public class BindPathTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Tools\Light\BindPaths\BindPathTests");

        [TestMethod]
        [Description("Verify that Light can use a bind path")]
        [Priority(1)]
        public void SimpleBindPath()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(BindPathTests.TestDataDirectory, @"SimpleBindPath\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.BindPath = WixTests.SharedFilesDirectory;
            light.Run();

            Verifier.VerifyResults(Path.Combine(BindPathTests.TestDataDirectory, @"SimpleBindPath\expected.msi"), light.OutputFile);
        }

        [TestMethod]
        [Description("Verify that Light can use named bind paths")]
        [Priority(1)]
        public void NamedBindPath()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(BindPathTests.TestDataDirectory, @"NamedBindPath\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.BindPath = String.Concat("Test=", WixTests.SharedFilesDirectory);
            light.Run();

            Verifier.VerifyResults(Path.Combine(BindPathTests.TestDataDirectory, @"NamedBindPath\expected.msi"), light.OutputFile);
        }
    }
}