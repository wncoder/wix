//-----------------------------------------------------------------------
// <copyright file="RegressionTests.cs" company="Microsoft">
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
// <summary>Regression tests for Candle/Light Integration</summary>
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
    /// Regression tests for Candle/Light Integration
    /// </summary>
    [TestClass]
    public class RegressionTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\RegressionTests");

        [TestMethod]
        [Description("Verify that a Directory inherits its parent directory's Name, not the ShortName")]
        [TestProperty("Bug Link", "https://sourceforge.net/tracker/index.php?func=detail&aid=1824809&group_id=105970&atid=642714")]
        [Priority(3)]
        public void SF1824809()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(RegressionTests.TestDataDirectory, @"SF1824809\product.wxs"));
            candle.Run();

            Light light = new Light();
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.Run();
        }

        [TestMethod]
        [Description("Builds an MSI with a file search. This is not currently a test because there is an open spec issue.")]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/index.php?func=detail&aid=1656236&group_id=105970&atid=642714")]
        [Priority(3)]
        [Ignore]
        public void SF1656236()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(RegressionTests.TestDataDirectory, @"SF1656236\product.wxs"));
            candle.Run();

            Light light = new Light();
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.OutputFile = Path.Combine(candle.OutputDirectory, "product.msi");
            light.Run();

            // Verify that the DrLocator table was generated correctly
        }

        [TestMethod]
        [Description("Verify that a Duplicate authoring of IgnoreModularization element do not cause a build failure.")]
        [Priority(3)]
        public void DuplicateIgnoreModularization()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(RegressionTests.TestDataDirectory, @"DuplicateIgnoreModularization\product.wxs"));
            // supress deprecated element warning
            candle.SuppressWarnings.Add(1085);
            candle.Run();

            Light light = new Light();
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.Run();
        }
    }
}
