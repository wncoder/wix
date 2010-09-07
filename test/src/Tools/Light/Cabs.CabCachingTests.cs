//-----------------------------------------------------------------------
// <copyright file="Cabs.CabCachingTests.cs" company="Microsoft">
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
//     Tests for cab caching
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Light.Cabs
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for cab caching
    /// </summary>
    [TestClass]
    public class CabCachingTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Tools\Light\Cabs\CabCachingTests");

        [TestMethod]
        [Description("Verify that Light can bind files into a wixout")]
        [Priority(1)]
        [Ignore] // Bug
        public void SimpleCabCaching()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(CabCachingTests.TestDataDirectory, @"SimpleCabCaching\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.CachedCabsPath = Path.Combine(Path.GetDirectoryName(light.OutputFile), "CabCacheDirectory");
            light.ReuseCab = false;
            light.Run();

            string cachedCab = Path.Combine(light.CachedCabsPath, "product.cab");
            Assert.IsTrue(File.Exists(cachedCab), "The cabinet file was not cached in {0}", cachedCab);
        }

        [TestMethod]
        [Description("Verify that passing an existing file path instead of a directory path as a cab cache path results in the expected error message")]
        [Priority(2)]
        public void InvalidCabCache()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add((Path.Combine(CabCachingTests.TestDataDirectory,@"InvalidCabCache\product.wxs")));
            candle.Run();

            string invalidCabCachePath = Path.Combine(Path.GetDirectoryName(candle.OutputFile), "testCabCache.cab");
            System.IO.File.Create(invalidCabCachePath);
            string expectedErrorMessage = string.Format("The -cc option requires a directory, but the provided path is a file: {0}", invalidCabCachePath);

            Light light = new Light(candle);
            light.CachedCabsPath = invalidCabCachePath;
            light.ExpectedWixMessages.Add(new WixMessage(280, expectedErrorMessage, WixMessage.MessageTypeEnum.Error));
            light.ExpectedExitCode = 280;
            light.Run();
        }

    }
}