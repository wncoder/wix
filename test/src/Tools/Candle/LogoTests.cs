//-----------------------------------------------------------------------
// <copyright file="LogoTests.cs" company="Microsoft">
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
// <summary>Test how Candle handles the NoLogo switch.</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Candle.Logo
{
    using System;
    using System.IO;
    using System.Text.RegularExpressions;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Test how Candle handles the NoLogo switch.
    /// </summary>
    [TestClass]
    public class LogoTests
    {
        private static readonly string ProductWxs = Path.Combine(WixTests.SharedAuthoringDirectory, "BasicProduct.wxs");

        [TestMethod]
        [Description("Verify that Candle prints the Logo information.")]
        [Priority(2)]
        public void PrintLogo()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(LogoTests.ProductWxs);
            candle.NoLogo = false;
            candle.ExpectedOutputRegexs.Add(new Regex(@"Microsoft \(R\) Windows Installer Xml Compiler version 3\.0\.\d{4}.0"));
            candle.ExpectedOutputStrings.Add("Copyright (C) Microsoft Corporation. All rights reserved.");
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that Candle does not print the Logo information.")]
        [Priority(2)]
        public void PrintWithoutLogo()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(LogoTests.ProductWxs);
            candle.NoLogo = true;
            Result result = candle.Run();
            string logoOutput1 = "Microsoft (R) Windows Installer Xml Compiler version";
            string logoOutput2 = "Copyright (C) Microsoft Corporation 2003. All rights reserved.";
            Assert.IsFalse(result.StandardOutput.Contains(logoOutput1), "'{0}' is printed", logoOutput1);
            Assert.IsFalse(result.StandardOutput.Contains(logoOutput2), "'{0}' is printed", logoOutput2);
        }
    }
}