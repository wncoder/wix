//-----------------------------------------------------------------------
// <copyright file="Input.WixobjTests.cs" company="Microsoft">
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
// <summary>Test for giving wixobj files as input to Light</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Light.Input
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests;

    /// <summary>
    /// Test for giving wixobj files as input to Light
    /// </summary>
    [TestClass]
    public class WixobjTests
    {
        /// <summary>
        /// This authoring will be used by many tests
        /// </summary>
        private static readonly string ProductWxs = Path.Combine(WixTests.SharedAuthoringDirectory, "BasicProduct.wxs");
        private static readonly string ModuleWxs = Path.Combine(WixTests.SharedAuthoringDirectory, "BasicModule.wxs");

        #region wixobjs

        [TestMethod]
        [Description("Verify that Light accepts a single wixobj as input")]
        [Priority(1)]
        public void SingleWixobj()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(WixobjTests.ProductWxs);
            candle.Run();

            Light light = new Light();
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.OutputFile = Path.Combine(candle.OutputDirectory, "product.msi");
            light.Run();
        }

        [TestMethod]
        [Description("Verify that Light accepts multiple wixobjs as input")]
        [Priority(1)]
        public void MultipleWixobjs()
        {
            string testDirectory = @"%WIX%\test\data\Tools\Light\Input\WixobjTests";

            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(testDirectory, "product.wxs"));
            candle.SourceFiles.Add(Path.Combine(testDirectory, "features.wxs"));
            candle.SourceFiles.Add(Path.Combine(testDirectory, "component1.wxs"));
            candle.Run();

            Light light = new Light();
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.OutputFile = Path.Combine(candle.OutputDirectory, "product.msi");
            light.Run();
        }

        [TestMethod]
        [Description("Verify that Light accepts multiple wixobjs where at least one wixobj is not referenced")]
        [Priority(2)]
        public void UnreferencedWixobj()
        {
            string testDirectory = @"%WIX%\test\data\Tools\Light\Input\WixobjTests";

            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(testDirectory, "product.wxs"));
            candle.SourceFiles.Add(Path.Combine(testDirectory, "features.wxs"));
            candle.SourceFiles.Add(Path.Combine(testDirectory, "component1.wxs"));
            // Component2 in component2.wxs is not referenced anywhere
            candle.SourceFiles.Add(Path.Combine(testDirectory, "component2.wxs"));
            candle.Run();

            Light light = new Light();
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.OutputFile = Path.Combine(candle.OutputDirectory, "product.msi");
            light.Run();
        }

        [TestMethod]
        [Description("Verify a Light error for a single wixobj with no entry section")]
        [Priority(2)]
        public void SingleWixobjWithNoEntrySection()
        {
            Candle candle = new Candle();
            // component1.wxs does not have an entry section
            candle.SourceFiles.Add(@"%WIX%\test\data\Tools\Light\Input\WixobjTests\component1.wxs");
            candle.Run();

            Light light = new Light();
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.ExpectedWixMessages.Add(new WixMessage(93, "Could not find entry section in provided list of intermediates. Expected section of type 'Unknown'.", WixMessage.MessageTypeEnum.Error));
            light.ExpectedExitCode = 93;
            light.Run();
        }

        [TestMethod]
        [Description("Verify a Light error for a multiple wixobjs with no entry section")]
        [Priority(3)]
        public void MultipleWixobjsWithNoEntrySection()
        {
            string testDirectory = @"%WIX%\test\data\Tools\Light\Input\WixobjTests";

            Candle candle = new Candle();
            // These files do not have entry sections
            candle.SourceFiles.Add(Path.Combine(testDirectory, "features.wxs"));
            candle.SourceFiles.Add(Path.Combine(testDirectory, "component1.wxs"));
            candle.SourceFiles.Add(Path.Combine(testDirectory, "component2.wxs"));
            candle.Run();

            Light light = new Light();
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.OutputFile = Path.Combine(candle.OutputDirectory, "product.msi");
            light.ExpectedWixMessages.Add(new WixMessage(93, "Could not find entry section in provided list of intermediates. Expected section of type 'Product'.", WixMessage.MessageTypeEnum.Error));
            light.ExpectedExitCode = 93;
            light.Run();
        }

        [TestMethod]
        [Description("Verify a Light error for a multiple wixobjs with multiple entry sections. This test is disabled until a spec issue is resolved.")]
        [Priority(2)]
        [Ignore]
        public void MultipleWixobjEntrySections()
        {
            Candle candle = new Candle();
            // These files both have entry sections
            candle.SourceFiles.Add(WixobjTests.ProductWxs);
            candle.SourceFiles.Add(WixobjTests.ModuleWxs);
            candle.Run();

            Light light = new Light();
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.OutputFile = Path.Combine(candle.OutputDirectory, "product.msi");
            light.ExpectedWixMessages.Add(new WixMessage(89, "Multiple entry sections '{12345678-1234-1234-1234-123456789012}' and 'Module1' found.  Only one entry section may be present in a single target.", WixMessage.MessageTypeEnum.Error));
            light.ExpectedExitCode = 89;
            light.Run(); ;
        }

        #endregion
    }
}