//-----------------------------------------------------------------------
// <copyright file="BurnLogUnitTests.cs" company="Microsoft">
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
//     - Tests for the BurnLog (part of the Burn test infrastructure)
// </summary>
//-----------------------------------------------------------------------

namespace BurnTestToolsUnitTests.UnitTests.BurnLogTests
{
    using System;
    using System.Collections.Generic;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class BurnLogUnitTests
    {
        [TestMethod]
        [Description("verify the burnstub.exe will launch without blowing up because stuff is missing in the layout or the parameterinfo.xml isn't formed correctly")]
        public void ITBurnLogGetLatestBurnLogFromEachTempFolder()
        {
            List<BurnLog> bls = BurnLog.GetLatestBurnLogFromEachTempFolder("*_*_*.html", DateTime.MinValue, DateTime.MaxValue);
            Assert.IsTrue(bls.Count > 0, "No logs found");
        }

        [TestMethod]
        [Description("verify the burnstub.exe will launch without blowing up because stuff is missing in the layout or the parameterinfo.xml isn't formed correctly")]
        public void ITBurnLogGetAllBurnLogsFromEachUsersTempFolder()
        {
            List<BurnLog> bls = BurnLog.GetAllBurnLogsFromEachUsersTempFolder("*_*_*.html", DateTime.MinValue, DateTime.MaxValue);
            Assert.IsTrue(bls.Count > 0, "No logs found");
        }
        
    }
}
