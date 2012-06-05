//-----------------------------------------------------------------------
// <copyright file="Cultures.CultureTests.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>
//     Tests for cultures
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Tools.Light.Cultures
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;

    /// <summary>
    /// Tests for cultures
    /// </summary>
    [TestClass]
    public class CulturesTests : WixTests
    {
        [TestMethod]
        [Description("Verify that passing an invalid culture to light does not cause an error.")]
        [Priority(2)]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/index.php?func=detail&aid=1942991&group_id=105970&atid=642714")]
        public void InvalidCultures()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(WixTests.BasicProductWxs);
            candle.Run();

            Light light = new Light(candle);
            light.Cultures = "en-US;in-VA;lid";
            light.Run();
        }
    }
}