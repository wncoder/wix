//-----------------------------------------------------------------------
// <copyright file="BurnLogUnitTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
