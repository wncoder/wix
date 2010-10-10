//-----------------------------------------------------------------------
// <copyright file="Localization.LocalizationTests.cs" company="Microsoft">
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
//     Test MSI localization with Light
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Light.Localization
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Test MSI localization with Light
    /// </summary>
    [TestClass]
    public class LocalizationTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Tools\Light\Localization\LocalizationTests");

        [TestMethod]
        [Description("Verify that an MSI can be localized")]
        [Priority(2)]
        public void Hebrew()
        {
            string wixobj = Candle.Compile(Path.Combine(LocalizationTests.TestDataDirectory, @"Shared\product.wxs"));

            Light light = new Light();
            light.ObjectFiles.Add(wixobj);
            light.LocFiles.Add(Path.Combine(LocalizationTests.TestDataDirectory, @"he-il\he-il.wxl"));
            light.OutputFile = Path.Combine(Path.Combine(this.TestContext.TestDir, Path.GetRandomFileName()), "he-il.msi");
            light.Run();

            Verifier.CompareResults(Path.Combine(LocalizationTests.TestDataDirectory, @"he-il\he-il.msi"), light.OutputFile);
        }

        [TestMethod]
        [Description("Verify that variable Ids containing dots are accepted.")]
        [Priority(2)]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/index.php?func=detail&aid=1711440&group_id=105970&atid=642714")]
        public void ValidIdentifier()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(LocalizationTests.TestDataDirectory, @"ValidIdentifier\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.LocFiles.Add(Path.Combine(LocalizationTests.TestDataDirectory, @"ValidIdentifier\en-us.wxl"));
            light.Cultures = "en-us";
            light.Run();
        }
    }
}