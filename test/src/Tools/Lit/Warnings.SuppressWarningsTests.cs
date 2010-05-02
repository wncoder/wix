//-----------------------------------------------------------------------
// <copyright file="Warnings.SuppressWarningTests.cs" company="Microsoft">
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
//     Tests how Lit handles the suppress warning switch.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Lit.Warnings
{
    using System;
    using System.IO;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Test how Lit handles the suppress warning switch.
    /// </summary>
    [TestClass]
    public class SuppressWarningsTests : WixTests
    {
        [TestMethod]
        [Description("Verify that Lit honors the sw switch when specified.")]
        [Priority(2)]
        public void SuppressSpecificWarnings()
        {
            Lit lit = new Lit();
            lit.OtherArguments = " -abc";
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.SuppressWarnings.Add(1098);
            lit.Run();
        }

        [TestMethod]
        [Description("Verify that Lit does not suppress warnings when the sw switch is not specified.")]
        [Priority(2)]
        public void NoSuppressWarningsSwitch()
        {
            Lit lit = new Lit();
            lit.OtherArguments = " -abc";
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.ExpectedWixMessages.Add(new WixMessage(1098, "'abc' is not a valid command line argument.", WixMessage.MessageTypeEnum.Warning));
            lit.Run();
        }

        [TestMethod]
        [Description("Verify that all warnings are suppressed with the sw switch.")]
        [Priority(2)]
        public void SuppressAllWarningsSwitch()
        {
            Lit lit = new Lit();
            lit.OtherArguments = " -abc";
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.SuppressAllWarnings = true;
            lit.Run();
        }

        [TestMethod]
        [Description("Verify that Lit honors the swall switch when specified and displays that the switch is deprecated.")]
        [Priority(2)]
        public void VerifyDeprecatedSwitch()
        {
            Lit lit = new Lit();
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.OtherArguments = "-swall";
            lit.ExpectedWixMessages.Add(new WixMessage(1108, "The command line switch 'swall' is deprecated. Please use 'sw' instead.", WixMessage.MessageTypeEnum.Warning));
            lit.Run();
        }
    }
}