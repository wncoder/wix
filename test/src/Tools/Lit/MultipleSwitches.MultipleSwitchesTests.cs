//-----------------------------------------------------------------------
// <copyright file="MultipleSwitches.MultipleSwitchesTests.cs" company="Microsoft">
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