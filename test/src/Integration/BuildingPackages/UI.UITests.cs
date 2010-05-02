//-----------------------------------------------------------------------
// <copyright file="UI.UITests.cs" company="Microsoft">
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
//     General tests for UI
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.UI
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// General tests for UI
    /// </summary>
    [TestClass]
    public class UITests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Integration\BuildingPackages\UI\UITests");

        [TestMethod]
        [Description("Verify that a simple UI can be defined")]
        [Priority(1)]
        public void SimpleUI()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(UITests.TestDataDirectory, @"SimpleUI\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.SuppressedICEs.Add("ICE20");
            light.SuppressedICEs.Add("ICE31");
            light.Run();

            string expectedMsi = Path.Combine(UITests.TestDataDirectory, @"SimpleUI\expected.msi");
            Verifier.VerifyResults(expectedMsi, light.OutputFile);
        }
    }
}