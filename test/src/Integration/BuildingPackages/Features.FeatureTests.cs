//-----------------------------------------------------------------------
// <copyright file="Features.FeatureTests.cs" company="Microsoft">
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
//     Tests for authoring Features
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Features
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for authoring Features
    /// </summary>
    [TestClass]
    public class FeatureTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Integration\BuildingPackages\Features\FeatureTests");

        [TestMethod]
        [Description("Verify that a simple Feature can be defined and that the expected default values are set")]
        [Priority(1)]
        public void SimpleFeature()
        {
            string sourceFile = Path.Combine(FeatureTests.TestDataDirectory, @"SimpleFeature\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");
            Verifier.VerifyResults(Path.Combine(FeatureTests.TestDataDirectory, @"SimpleFeature\expected.msi"), msi, "Feature");
        }

        [TestMethod]
        [Description("Verify that a feature Id is required")]
        [Priority(2)]
        [Ignore]
        public void MissingId()
        {
        }

        [TestMethod]
        [Description("Verify that a feature can be advertised")]
        [Priority(2)]
        [Ignore]
        public void AllowAdvertise()
        {
        }

        [TestMethod]
        [Description("Verify that the feature level can be set to any valid value")]
        [Priority(2)]
        [Ignore]
        public void FeatureLevel()
        {
        }

        [TestMethod]
        [Description("Verify that a configurable directory can be set")]
        [Priority(2)]
        [Ignore]
        public void ConfigurableDirectory()
        {
        }

        [TestMethod]
        [Description("Verify that the feature level is required")]
        [Priority(3)]
        [Ignore]
        public void MissingFeatureLevel()
        {
        }

        [TestMethod]
        [Description("Verify that the initial display of a feature can be set")]
        [Priority(3)]
        [Ignore]
        public void FeatureDisplay()
        {
        }

        [TestMethod]
        [Description("Verify that a feature can be specified to allow or disallow it to be Absent")]
        [Priority(3)]
        [Ignore]
        public void Absent()
        {
        }

        [TestMethod]
        [Description("Verify that a the default install location of a feature can be set")]
        [Priority(3)]
        [Ignore]
        public void InstallDefault()
        {
        }

        [TestMethod]
        [Description("Verify that a the default advertise state of a feature can be set")]
        [Priority(3)]
        [Ignore]
        public void TypicalDefault()
        {
        }

    }
}
