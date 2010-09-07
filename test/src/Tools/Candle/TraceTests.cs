//-----------------------------------------------------------------------
// <copyright file="TraceTests.cs" company="Microsoft">
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
// <summary>Tests how Candle handles the Trace switch.</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Candle.Trace
{
    using System;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Test how Candle handles the Trace switch.
    /// </summary>
    [TestClass]
    public class TraceTests : WixTests
    {
        private static string testFile = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Tools\Candle\TraceTests\Product.wxs");

        [TestMethod]
        [Description("Verify that Candle honors the trace switch when specified.")]
        [Priority(2)]
        public void TraceSwitch()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(testFile);
            candle.Trace = true;
            //WixMessage verification will not work on multiple line outputs, we have to check standard output.
            string output = String.Format("warning CNDL1075 : The Product/@UpgradeCode attribute was not found; it is strongly recommended to ensure that this product can be upgraded.Source trace:{0}at {1}: line 6", Environment.NewLine, testFile);
            candle.ExpectedOutputStrings.Add(output);
            candle.ExpectedWixMessages.Add(new WixMessage(1075, WixMessage.MessageTypeEnum.Warning));
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that Candle does not print trace messages when trace switch is not specified.")]
        [Priority(2)]
        public void NoTraceSwitch()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(testFile);
            candle.Trace = false;
            candle.ExpectedWixMessages.Add(new WixMessage(1075, "The Product/@UpgradeCode attribute was not found; it is strongly recommended to ensure that this product can be upgraded.", WixMessage.MessageTypeEnum.Warning));
            candle.Run();
        }
    }
}