//-----------------------------------------------------------------------
// <copyright file="Cabinets.CabinetTests.cs" company="Microsoft">
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
//     Tests for cabinets
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.CustomActions
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for cabinets
    /// </summary>
    [TestClass]
    public class CabinetTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Cabinets\CabinetTests");

        [TestMethod]
        [Description("Verify that binding fails with duplicate cabinet names")]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/index.php?func=detail&aid=1793251&group_id=105970&atid=642714")]
        [Priority(2)]
        public void DuplicateCabinetNames()
        {
            // Run Candle
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(CabinetTests.TestDataDirectory, @"DuplicateCabinetNames\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.ExpectedWixMessages.Add(new WixMessage(290, "Duplicate cabinet name '#dupe.cab' found.", WixMessage.MessageTypeEnum.Error));
            light.ExpectedWixMessages.Add(new WixMessage(291, "Duplicate cabinet name '#dupe.cab' error related to previous error.", WixMessage.MessageTypeEnum.Error));
            light.ExpectedExitCode = 291;
            light.Run();
        }
    }
}