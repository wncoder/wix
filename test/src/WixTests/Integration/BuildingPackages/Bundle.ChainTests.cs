//-----------------------------------------------------------------------
// <copyright file="Bundle.ChainTests.cs" company="Microsoft">
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
//     Tests for Bundle Chain element
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Integration.BuildingPackages.Bundle
{
    using System;
    using System.Collections.Generic;
    using System.IO;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;

    /// <summary>
    /// Tests for Bundle Chain element
    /// </summary>
    [TestClass]
    public class ChainTests : BundleTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Bundle\ChainTests");

        [TestMethod]
        [Description("Verify output of a Chain element with 1 Package child elements")]
        [Priority(2)]
        public void ChainPackageChild()
        {
            string sourceFile = Path.Combine(ChainTests.TestDataDirectory, @"ChainPackageChild\Product.wxs");
            string outputDirectory = this.TestDirectory;

            // build the bootstrapper
            string bootstrapper = Builder.BuildBundlePackage(outputDirectory, sourceFile);

            // verify the ParameterInfo and burnManifest has the correct information 
            PackageTests.VerifyMsiPackageInformation(outputDirectory, "MsiPackage.msi", "MsiPackage", null, BundleTests.MsiPackageProductCode, false, false, string.Format("{0}v0.1.0.0", BundleTests.MsiPackageProductCode), null, BundleTests.MsiPackageFile);
            PackageTests.VerifyMspPackageInformation(outputDirectory, "MspPackage.msp", "MspPackage", null, BundleTests.MspPackagePatchCode, false, false, BundleTests.MspPackagePatchCode, null, BundleTests.MspPackageFile);
            PackageTests.VerifyMsuPackageInformation(outputDirectory, "MsuPackage.msu", "MsuPackage", null, false, false, null, BundleTests.MsuPackageFile);
            PackageTests.VerifyExePackageInformation(outputDirectory, "ExePackage.exe", "ExePackage", null, false, false, string.Empty, string.Empty, string.Empty, null, BundleTests.ExePackageFile);
        }

        [TestMethod]
        [Description("Verify output of a Chain element with 1 PackageGroupRef child elements")]
        [Priority(2)]
        public void ChainPackageGroupRefChild()
        {
            string sourceFile = Path.Combine(ChainTests.TestDataDirectory, @"ChainPackageGroupRefChild\Product.wxs");
            string outputDirectory = this.TestDirectory;

            // build the bootstrapper
            string bootstrapper = Builder.BuildBundlePackage(outputDirectory, sourceFile);

            // verify the ParameterInfo and burnManifest has the correct information 
            PackageTests.VerifyMsiPackageInformation(outputDirectory, "MsiPackage.msi", "MsiPackage", null, BundleTests.MsiPackageProductCode, false, false, string.Format("{0}v0.1.0.0", BundleTests.MsiPackageProductCode), null, BundleTests.MsiPackageFile);
            PackageTests.VerifyMspPackageInformation(outputDirectory, "MspPackage.msp", "MspPackage", null, BundleTests.MspPackagePatchCode, false, false, BundleTests.MspPackagePatchCode, null, BundleTests.MspPackageFile);
            PackageTests.VerifyMsuPackageInformation(outputDirectory, "MsuPackage.msu", "MsuPackage", null, false, false, null, BundleTests.MsuPackageFile);
            PackageTests.VerifyExePackageInformation(outputDirectory, "ExePackage.exe", "ExePackage", null, false, false, string.Empty, string.Empty, string.Empty, null, BundleTests.ExePackageFile);
        }

        [TestMethod]
        [Description("50 Package child elements.")]
        [Priority(3)]
        [Ignore]
        public void ChainMultiplePackageChildren()
        {
        }

        [TestMethod]
        [Description("Verify that build fails if two sibling Packages are the same Id")]
        [Priority(3)]
        public void ChainDuplicatePackages()
        {
            string candleOutput = Candle.Compile(Path.Combine(ChainTests.TestDataDirectory, @"ChainDuplicatePackages\Product.wxs"));

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(91, "Duplicate symbol 'ChainPackage:Package1' found.", Message.MessageTypeEnum.Error));
            light.ExpectedWixMessages.Add(new WixMessage(92, "Location of symbol related to previous error.", Message.MessageTypeEnum.Error));
            light.ExpectedExitCode = 92;
            light.Run();
        }

        [TestMethod]
        [Description("Verify that build fails if two sibling PackageGroups contain the same Id")]
        [Priority(3)]
        public void ChainDuplicatePackageInPackageGroups()
        {
            string candleOutput = Candle.Compile(Path.Combine(ChainTests.TestDataDirectory, @"ChainDuplicatePackageInPackageGroups\Product.wxs"));

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(91, Message.MessageTypeEnum.Error)); // Duplicate symbol error
            light.ExpectedWixMessages.Add(new WixMessage(92, Message.MessageTypeEnum.Error)); // Location of symbol related to previous error
            light.IgnoreWixMessageOrder = true;
            light.ExpectedExitCode = 92;
            light.Run();
        }

        [TestMethod]
        [Description("Verify that build fails if two sibling PackageGroupRefs reference the same PackageGroup")]
        [Priority(3)]
        public void ChainDuplicatePackageGroupRefs()
        {
            string candleOutput = Candle.Compile(Path.Combine(ChainTests.TestDataDirectory, @"ChainDuplicatePackageGroupRefs\Product.wxs"));

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(343, Message.MessageTypeEnum.Error)); //A circular reference of ordering dependencies was detected.
            light.IgnoreWixMessageOrder = true;
            light.ExpectedExitCode = 343;
            light.Run();
        }
    }
}
