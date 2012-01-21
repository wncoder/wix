//-----------------------------------------------------------------------
// <copyright file="Localization.LocalizationTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Test how Lit handles -loc switch
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Lit.Localization
{
    using System;
    using System.IO;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
     /// <summary>
    /// Test the different ways for giving input files to Candle.
    /// </summary>
    [TestClass]
    public class LocalizationTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Tools\Lit\Localization\LocalizationTests");

        [TestMethod]
        [Description("Verify that Lit accepts a single Windows Installer XML include (wxs) file as input")]
        [Priority(1)]
        public void SingleWxlFile()
        {
            Lit lit = new Lit();
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.LocFiles.Add(Path.Combine(LocalizationTests.TestDataDirectory, @"Shared\en-US.wxl"));
            lit.Run();

            Verifier.VerifyWixLibLocString(lit.OutputFile,"en-us","String1","String1(en-us)");
        }

        [TestMethod]
        [Description("Verify that Lit accepts multiple Windows Installer XML object (wxs) files as input")]
        [Priority(2)]
        public void MultipleWxlFiles()
        {
            Lit lit = new Lit();
            lit.ObjectFiles.Add(Candle.Compile(WixTests.PropertyFragmentWxs));
            lit.LocFiles.Add(Path.Combine(LocalizationTests.TestDataDirectory, @"Shared\en-US.wxl"));
            lit.LocFiles.Add(Path.Combine(LocalizationTests.TestDataDirectory, @"Shared\ja-JP.wxl"));
            lit.Run();

            Verifier.VerifyWixLibLocString(lit.OutputFile, "en-us", "String1", "String1(en-us)");
            Verifier.VerifyWixLibLocString(lit.OutputFile, "ja-jp", "String1", "String1(ja-jp)");
        }
   }
}