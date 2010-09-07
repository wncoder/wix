//-----------------------------------------------------------------------
// <copyright file="SmokeTests.cs" company="Microsoft">
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
// <summary>Smoke tests for Candle/Light Integration</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Smoke tests for Candle/Light Integration
    /// </summary>
    [TestClass]
    public class SmokeTests : WixTests
    {
        [TestMethod]
        [Description("A small, typical case scenario for using Candle and Light")]
        [Priority(1)]
        public void Scenario01()
        {
            string sourceFile = Path.Combine(WixTests.SharedAuthoringDirectory, "BasicProduct.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");
            
            Verifier.VerifyResults(Path.Combine(WixTests.SharedBaselinesDirectory, @"MSIs\BasicProduct.msi"), msi);
        }

        [TestMethod]
        [Description("A scenario for using Candle and Light that exercises several features")]
        [Priority(1)]
        public void Scenario02()
        {
            string testDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\SmokeTests\Scenario02");

            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(testDirectory, "customactions.wxs"));
            candle.SourceFiles.Add(Path.Combine(testDirectory, "components.wxs"));
            candle.SourceFiles.Add(Path.Combine(testDirectory, "features.wxs"));
            candle.SourceFiles.Add(Path.Combine(testDirectory, "product.wxs"));
            candle.SourceFiles.Add(Path.Combine(testDirectory, "properties.wxs"));
            candle.Run();

            Light light = new Light();
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.SuppressedICEs.Add("ICE49");
            light.Run();

            // Verify
            Verifier.VerifyResults(Path.Combine(testDirectory, @"expected.msi"), light.OutputFile);
        }
    }
}
