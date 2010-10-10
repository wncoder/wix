//-----------------------------------------------------------------------
// <copyright file="PedanticTests.cs" company="Microsoft">
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
// <summary>Tests how Candle handles the Pedantic switch</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Candle.Pedantic
{
    using System;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Test how Candle handles the Pedantic switch.
    /// </summary>
    [TestClass]
    public class PedanticTests : WixTests
    {
        private static readonly string TestFile = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Tools\Candle\PedanticTests\Product.wxs");

        [TestMethod]
        [Description("Verify that Candle honors the pedantic switch when specified.")]
        [Priority(2)]
        public void PedanticSwitch()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(PedanticTests.TestFile);
            string guid = "aaaffB15-DF17-43b8-9971-DddC3D4F3490"; 
            candle.Pedantic = true;
            string expectedOutput = String.Format("The Product/@Id attribute's value, '{0}', is a mixed-case guid.  All letters in a guid value should be uppercase.", guid);
            candle.ExpectedWixMessages.Add(new WixMessage(87, expectedOutput, WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 87;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that Candle does not print pedantic messages when Pedantic switch is not specified.")]
        [Priority(2)]
        public void NoPedanticSwitch()
        {
            Candle candle = new Candle();
            candle.Pedantic = false;
            candle.SourceFiles.Add(PedanticTests.TestFile);
            candle.Run();
        }
    }
}