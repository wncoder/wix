//-----------------------------------------------------------------------
// <copyright file="CustomActions.CustomActionTests.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>
//     Tests for custom actions
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Integration.BuildingPackages.CustomActions
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;

    /// <summary>
    /// Tests for custom actions
    /// </summary>
    [TestClass]
    public class CustomActionTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\CustomActions\CustomActionTests");

        [TestMethod]
        [Description("Verify that a custom action missing required attributes fails")]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/index.php?func=detail&aid=1983810&group_id=105970&atid=642714")]
        [Priority(2)]
        public void MissingRequiredAttributes()
        {
            // Run Candle
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(CustomActionTests.TestDataDirectory, @"MissingRequiredAttributes\product.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(37, "The CustomAction/@ExeCommand attribute cannot be specified without attribute BinaryKey, Directory, FileKey, or Property present.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 37;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that a custom action can be created")]
        [Priority(1)]
        public void SimpleCustomAction()
        {
            string msi = Builder.BuildPackage(Path.Combine(CustomActionTests.TestDataDirectory, @"SimpleCustomAction\product.wxs"));

            Verifier.VerifyResults(Path.Combine(CustomActionTests.TestDataDirectory, @"SimpleCustomAction\expected.msi"), msi, "CustomAction", "InstallExecuteSequence");
        }
    }
}