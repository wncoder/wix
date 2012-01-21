//-----------------------------------------------------------------------
// <copyright file="UtilExtensionTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Util extension tests</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Sql extension tests
    /// </summary>
    [TestClass]
    public class RegressionTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Extensions\UtilExtension\RegressionTests");

        [TestMethod]
        [Description("Verify that using util:ServiceConfig without the -out paramter does not cause a light error.")]
        [Priority(2)]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/index.php?func=detail&aid=1951034&group_id=105970&atid=642714")]
        public void ValidServiceConfig()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(RegressionTests.TestDataDirectory,@"ValidServiceConfig\product.wxs"));
            candle.Extensions.Add("WixUtilExtension");
            candle.Run();

            Light light = new Light(candle);
            light.Extensions.Add("WixUtilExtension");
            light.OutputFile = string.Empty;
            light.Run();
        }
    }
}
