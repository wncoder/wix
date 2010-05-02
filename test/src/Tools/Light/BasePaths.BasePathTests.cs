//-----------------------------------------------------------------------
// <copyright file="BasePaths.BasePathTests.cs" company="Microsoft">
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
//     Test for setting base paths
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Light.BasePaths
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Test for setting base paths
    /// </summary>
    [TestClass]
    public class BasePathTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Tools\Light\BasePaths\BasePathTests");

        [TestMethod]
        [Description("Verify that Light can use a base path")]
        [Priority(1)]
        public void SimpleBasePath()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(BasePathTests.TestDataDirectory, @"SimpleBasePath\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.BasePath = WixTests.SharedFilesDirectory;
            light.Run();

            Verifier.VerifyResults(Path.Combine(BasePathTests.TestDataDirectory, @"SimpleBasePath\expected.msi"), light.OutputFile);
        }
    }
}