//-----------------------------------------------------------------------
// <copyright file="MultipleSwitches.MultipleSwitchesTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests how Lit handles multiple input switches.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Lit.MultipleInputSwitches
{
    using System;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
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