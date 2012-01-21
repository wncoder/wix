//-----------------------------------------------------------------------
// <copyright file="ICEs.SuppressICEsTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests for suppressing ICEs
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Light.ICEs
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for suppressing ICEs
    /// </summary>
    [TestClass]
    public class SuppressICEsTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Tools\Light\ICEs\SuppressICEsTests");

        [TestMethod]
        [Description("Verify that an ICE can be suppressed")]
        [Priority(1)]
        public void SimpleSuppressICE()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(SuppressICEsTests.TestDataDirectory, @"SimpleSuppressICE\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.SuppressedICEs.Add("ICE18");
            light.Run();
        }
    }
}