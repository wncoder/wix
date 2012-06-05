//-----------------------------------------------------------------------
// <copyright file="MultipleSwitches.MultipleSwitchesTests.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>
//     Tests how Lit handles multiple input switches.
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Tools.Lit.MultipleInputSwitches
{
    using System;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;
    
    /// <summary>
    /// Tests how Lit handles multiple input switches.
    /// </summary>
    [TestClass]
    public class MultipleSwitchesTests : WixTests
    {
        [TestMethod]
        [Description("Verify that Lit handles the case when multiple switches like 'Suppress All Warnings' and 'Treat Warnings as Errors' are given. In this scenario, Lit honors the sw switch and suppresses all warnings")]
        [Priority(3)]
        [Ignore] // ignored until we know the right behavior
        public void WxAndSw()
        {
            Lit lit = new Lit();
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.OtherArguments = " -abc";
            lit.TreatAllWarningsAsErrors = true;
            lit.SuppressAllWarnings = true;
            lit.Run();
        }
    }
}