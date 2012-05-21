//-----------------------------------------------------------------------
// <copyright file="CustomActions.CustomActionsTests.cs" company="Microsoft">
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