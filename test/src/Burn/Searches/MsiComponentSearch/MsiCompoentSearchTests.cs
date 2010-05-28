//-----------------------------------------------------------------------
// <copyright file="MsiCompoentSearchTests.cs" company="Microsoft">
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
//     - Tests for Burn Component Search feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Searches
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Searches;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches;

    [TestClass]
    public class MsiCompoentSearchTests : WixTests
    {
        private BurnMsiComponentSearchFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new BurnMsiComponentSearchFixture();
            fixture.CleanUp();
            fixture.UninstallProducts();

            fixture.InstallProducts();
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            fixture.CleanUp();
            fixture.UninstallProducts();
        }

        #endregion

        [TestMethod]
        [Description("MsiComponent search")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Searches\MsiComponentSearch\MsiComponentSearchData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=MsiComponentSearchData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Install$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnMsiComponentSearchDataDriven()
        {
            string id = (string)TestContext.DataRow[0];

            string condition = string.Empty;

            if (!string.IsNullOrEmpty(TestContext.DataRow[1].ToString()))
            {
                condition = (string)TestContext.DataRow[1];
            }

            string variable = (string)TestContext.DataRow[2];

            string componentId = (string)TestContext.DataRow[3];

            string productCode = string.Empty;

            if (! string.IsNullOrEmpty(TestContext.DataRow[4].ToString()))
            {
                productCode = (string)TestContext.DataRow[4];
            }

            MsiComponentSearch.MsiComponentSearchType type = 
                (MsiComponentSearch.MsiComponentSearchType)Enum.Parse(typeof
               (MsiComponentSearch.MsiComponentSearchType), (string)TestContext.DataRow[5], true);

            fixture.CreateMsiComponentSearchElement(id, type, variable, productCode, componentId, condition);

            string expectedValue = TestContext.DataRow[6].ToString();

            bool result = fixture.SearchResultVerification(expectedValue);

            Assert.IsTrue(result, string.Format("MSI Component search failed for comp id: {0}. See trace for more detail."
                , componentId));

        }
    }
}
