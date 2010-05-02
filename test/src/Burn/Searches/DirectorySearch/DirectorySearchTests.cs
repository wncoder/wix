//-----------------------------------------------------------------------
// <copyright file="DirectorySearchTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Tests for Burn Directory Search feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Searches
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Searches;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches;

    [TestClass]
    public class DirectorySearchTests : BurnTests
    {
        private BurnDirectorySearchFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new BurnDirectorySearchFixture();
            fixture.CleanUp();
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            fixture.CleanUp();
        }

        #endregion

        [TestMethod]
        [Description("Directory search")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Searches\DirectorySearch\DirectorySearchData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=DirectorySearchData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Install$",
            DataAccessMethod.Sequential)]
        public void BurnDirectorySearchDataDriven()
        {
            string id = (string)TestContext.DataRow[0];

            DirectorySearch.DirectorySearchType type = (DirectorySearch.DirectorySearchType)Enum.Parse(typeof(DirectorySearch.DirectorySearchType), (string)TestContext.DataRow[1], true);

            string path = Environment.ExpandEnvironmentVariables((string)TestContext.DataRow[2]);

            string variable = (string)TestContext.DataRow[3];

            fixture.CreateDirectorySearchElement(id, type, path, variable);

            bool result = fixture.SearchResultVerification();

            Assert.IsTrue(result
                , string.Format("Directory search result does not matches with expected value for path: {0}. See trace for more detail"
                , path));

        }
    }
}
