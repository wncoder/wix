//-----------------------------------------------------------------------
// <copyright file="ICEs.SuppressICEsTests.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>
//     Tests for suppressing ICEs
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Tools.Light.ICEs
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;

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