//-----------------------------------------------------------------------
// <copyright file="MultipleInputSwitchesTests.cs" company="Microsoft">
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
// <summary>Tests how Candle handles multiple input switches.</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Candle.MultipleInputSwitches
{
    using System;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Tests how Candle handles multiple input switches.
    /// </summary>
    [TestClass]
    public class MultipleInputSwitchesTests
    {
        [TestMethod]
        [Description("Verify that Candle handles the case when multiple switches like 'Suppress All Warnings' and 'Treat Warnings as Errors' are given. In this scenario, Candle honors the sw switch and suppresses all warnings")]
        [Priority(3)]
        public void WxAndSw()
        {
            string testFile = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Tools\Candle\MultipleInputSwitchesTests\WxAndSw\Product.wxs");
            Candle candle = new Candle();
            candle.TreatAllWarningsAsErrors = true;
            candle.SuppressAllWarnings = true;
            candle.SourceFiles.Add(testFile);
            candle.Run();
        }
    }
}