//-----------------------------------------------------------------------
// <copyright file="InstallPackages.PackageTests.cs" company="Microsoft">
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
//     Tests for the Package element as it applies to the Product element.
//     Summary Information and Compression are the main areas to test.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.InstallPackages
{
    using System;
    using System.IO;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for the Package element as it applies to the Product element
    /// </summary>
    [TestClass]
    public class PackageTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Integration\BuildingPackages\InstallPackages\PackageTests");

        [TestMethod]
        [Description("Verify that a simple MSI can be built and that the expected default values are set")]
        [Priority(1)]
        public void SimplePackage()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PackageTests.TestDataDirectory, @"SimplePackage\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.Run();

            Verifier.VerifyResults(Path.Combine(PackageTests.TestDataDirectory, @"SimplePackage\expected.msi"), light.OutputFile);
        }

        [TestMethod]
        [Description("Verify that a package can compress its files in a cab")]
        [Priority(1)]
        public void CompressedPackage()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PackageTests.TestDataDirectory, @"CompressedPackage\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.Run();

            Verifier.VerifyResults(Path.Combine(PackageTests.TestDataDirectory, @"CompressedPackage\expected.msi"), light.OutputFile);

            string expectedCab = Path.Combine(Path.GetDirectoryName(light.OutputFile), "product.cab");
            Assert.IsTrue(File.Exists(expectedCab), "The expected cab file {0} does not exist", expectedCab);
        }

        [TestMethod]
        [Description("Verify that a package Id can be static or auto-generated")]
        [Priority(2)]
        public void PackageIds()
        {
            // The Summary Information Stream property Id for the package code
            int packageCodePropertyId = 9;

            // These are the valid package Ids that will be tested
            Dictionary<string, Regex> ids = new Dictionary<string, Regex>();
            ids.Add("{E3B6D482-3AB1-4246-BA97-4D75CF4F55F1}", new Regex("^{E3B6D482-3AB1-4246-BA97-4D75CF4F55F1}$"));
            ids.Add("E3B6D482-3AB1-4246-BA97-4D75CF4F55F1", new Regex("^{E3B6D482-3AB1-4246-BA97-4D75CF4F55F1}$"));
            ids.Add("aaaaaaaa-bbbb-cccc-dddd-eeeeeeffffff", new Regex("^{AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEFFFFFF}$"));
            ids.Add("*", new Regex("^{[0-9A-F]{8}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{12}}$"));

            foreach (string id in ids.Keys)
            {
                Candle candle = new Candle();
                candle.SourceFiles.Add(Path.Combine(PackageTests.TestDataDirectory, @"PackageIds\product.wxs"));
                
                // Set a preprocessor variable that defines the package code
                candle.PreProcessorParams.Add("PackageId", id);

                candle.IgnoreExtraWixMessages = true;
                candle.Run();

                Light light = new Light(candle);
                light.Run();

                // Verify that the package code was set properly
                string packageId = Verifier.GetSummaryInformationProperty(light.OutputFile, packageCodePropertyId);
                Assert.IsTrue(ids[id].IsMatch(packageId), "The Summary Info property {0} in {1} with a value of {2} does not match the regular expression {3}", packageCodePropertyId, light.OutputFile, packageId, ids[id].ToString()); 
            }
        }

        [TestMethod]
        [Description("Verify that all of the valid codepages are allowed")]
        [Priority(2)]
        [Ignore]
        public void ValidCodePages()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error for an invalid codeage")]
        [Priority(2)]
        [Ignore]
        public void InvalidCodePages()
        {
        }

        [TestMethod]
        [Description("Verify that a package can support any of the three platforms intel, intel64 and x64")]
        [Priority(2)]
        [Ignore]
        public void Platforms()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if an invalid platform is specified")]
        [Priority(2)]
        [Ignore]
        public void InvalidPlatform()
        {
        }

        [TestMethod]
        [Description("Verify that install privileges can be specified on a package")]
        [Priority(2)]
        [Ignore]
        public void InstallPrivileges()
        {
        }

        [TestMethod]
        [Description("Verify that the source can be an admin image")]
        [Priority(2)]
        [Ignore]
        public void AdminImage()
        {
        }

        [TestMethod]
        [Description("Verify that installer version can be set to any valid version")]
        [Priority(2)]
        [Ignore]
        public void InstallerVersion()
        {
        }

        [TestMethod]
        [Description("Verify that an invalid installer version cannot be set")]
        [Priority(3)]
        [Ignore]
        public void InvalidInstallerVersion()
        {
        }

        [TestMethod]
        [Description("Verify that short filenames can be in the source")]
        [Priority(3)]
        [Ignore]
        public void ShortNames()
        {
        }
    }
}