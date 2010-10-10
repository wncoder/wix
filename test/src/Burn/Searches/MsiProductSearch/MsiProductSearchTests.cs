//-----------------------------------------------------------------------
// <copyright file="MsiProductSearchTests.cs" company="Microsoft">
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
//     - Tests for Burn Product Search feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Searches
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Searches;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class MsiProductSearchTests : BurnTests
    {
        private BurnMsiProductSearchFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new BurnMsiProductSearchFixture();
            fixture.CleanUp();

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
        [Description("MsiProduct search")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Searches\MsiProductSearch\MsiProductSearchData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=MsiProductSearchData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Install$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnMsiProductSearchDataDriven()
        {
            string id = (string)TestContext.DataRow[0];

            string condition = string.Empty;

            if (!string.IsNullOrEmpty(TestContext.DataRow[1].ToString()))
            {
                condition = (string)TestContext.DataRow[1];
            }

            string variable = (string)TestContext.DataRow[2];

            string productCode = (string)TestContext.DataRow[3];

            ProductSearchElement.ProductSearchResultType type = (ProductSearchElement.ProductSearchResultType)Enum.Parse
                (typeof(ProductSearchElement.ProductSearchResultType), (string)TestContext.DataRow[4], true);

            fixture.CreateMsiProductSearchElement(id, type, variable, productCode, condition);

            string expectedValue = TestContext.DataRow[5].ToString();

            bool result = fixture.SearchResultVerification(expectedValue, type, variable);

            Assert.IsTrue(result, string.Format("MSIProduct Search failed for product code: {0}. See trace for more detail", productCode));

        }
    }
}
