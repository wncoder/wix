//-----------------------------------------------------------------------
// <copyright file="Bundle.BundleElementTests.cs" company="Microsoft">
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
//     Tests for Bundle Element
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Bundle
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Xml;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for Bundle
    /// </summary>
    [TestClass]
    public class BundleElementTests : BundleTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Bundle\BundleElementTests");

        [TestMethod]
        [Description("Verify that build fails if UX element defined more than once under Bundle.")]
        [Priority(3)]
        public void BundleDoubleUX()
        {
            string sourceFile = Path.Combine(BundleElementTests.TestDataDirectory, @"BundleDoubleUX\BundleDoubleUX.wxs");

            Candle candle = new Candle();
            candle.SourceFiles.Add(sourceFile);
            candle.ExpectedWixMessages.Add(new WixMessage(41, @"The Bundle element contains multiple UX child elements.  There can only be one UX child element per Bundle element.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 41;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that build fails if UX element is not defined under Bundle.")]
        [Priority(3)]
        public void BundleMissingUX()
        {
            string sourceFile = Path.Combine(BundleElementTests.TestDataDirectory, @"BundleMissingUX\BundleMissingUX.wxs");
            
            Candle candle = new Candle();
            candle.SourceFiles.Add(sourceFile);
            candle.ExpectedWixMessages.Add(new WixMessage(63, @"A Bundle element must have at least one child element of type UX.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 63;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that build fails if Chain element is not defined under Bundle.")]
        [Priority(3)]
        public void BundleMissingChain()
        {
            string sourceFile = Path.Combine(BundleElementTests.TestDataDirectory, @"BundleMissingChain\BundleMissingChain.wxs");

            Candle candle = new Candle();
            candle.SourceFiles.Add(sourceFile);
            candle.ExpectedWixMessages.Add(new WixMessage(63, @"A Bundle element must have at least one child element of type Chain.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 63;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that build fails if a Payload element is a child of Bundle")]
        [Priority(3)]
        public void BundleWithPayloadChild()
        {
            string sourceFile = Path.Combine(BundleElementTests.TestDataDirectory, @"BundleWithPayloadChild\BundleWithPayloadChild.wxs");

            Candle candle = new Candle();
            candle.SourceFiles.Add(sourceFile);
            string message = @"The Bundle element contains an unexpected child element 'Payload'.";
            candle.ExpectedWixMessages.Add(new WixMessage(5, message, WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 5;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that build fails if a PayloadGroup element is a child of Bundle")]
        [Priority(3)]
        public void BundleWithPayloadGroupChild()
        {
            string sourceFile = Path.Combine(BundleElementTests.TestDataDirectory, @"BundleWithPayloadGroupChild\BundleWithPayloadGroupChild.wxs");

            // build the bootstrapper
            string bootstrapper = Builder.BuildBundlePackage(this.TestDirectory, sourceFile);

            Assert.IsTrue(File.Exists(bootstrapper), "The bootstrapper file '{0}' was not created as expected.", bootstrapper);
        }

        [TestMethod]
        [Description("Verify that build fails if there are multiple Bundle elements defined.")]
        [Priority(3)]
        public void MultipleBundleElements()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(BundleElementTests.TestDataDirectory, @"MultipleBundleElements\Product.wxs"));
            candle.SourceFiles.Add(Path.Combine(BundleElementTests.TestDataDirectory, @"MultipleBundleElements\SecondBundleElement.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.OutputFile = "Setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(89,Message.MessageTypeEnum.Error)); // Multiple entry sections error
            light.ExpectedWixMessages.Add(new WixMessage(90,Message.MessageTypeEnum.Error)); // Location of entry section related to previous error.
            light.ExpectedExitCode = 90;
            light.Run();
        }

    }
}
