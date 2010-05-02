//-----------------------------------------------------------------------
// <copyright file="Cabs.ReuseCabTests.cs" company="Microsoft">
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
//     Tests for reusing cabs
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
    /// Tests for reusing cabs
    /// </summary>
    [TestClass]
    public class ReuseCabTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Tools\Light\Cabs\ReuseCabTests");

        [TestMethod]
        [Description("Verify that cabs can be reused")]
        [Priority(1)]
        [Ignore] // Bug
        public void SimpleReuseCab()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(ReuseCabTests.TestDataDirectory, @"SimpleReuseCab\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.ReuseCab = true;
            light.CachedCabsPath = Path.Combine(ReuseCabTests.TestDataDirectory, "SimpleReuseCab");
            light.Run();
        }
    }
}