//-----------------------------------------------------------------------
// <copyright file="FileSearchTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Tests for Burn File Search feature.
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
    public class FileSearchTests : BurnTests
    {
        private BurnFileSearchFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new BurnFileSearchFixture();
            fixture.CleanUp();
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            fixture.CleanUp();
        }

        #endregion

        [TestMethod]
        [Description("File search")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Searches\FileSearch\FileSearchData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=FileSearchData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Install$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnFileSearchDataDriven()
        {
            string id = (string)TestContext.DataRow[0];

            FileSearch.FileSearchType type = (FileSearch.FileSearchType)Enum.Parse(typeof(FileSearch.FileSearchType), (string)TestContext.DataRow[1], true);

            string path = Environment.ExpandEnvironmentVariables((string)TestContext.DataRow[2]);

            string variable = (string)TestContext.DataRow[3];

            fixture.CreateFileSearchElement(id, type, path, variable);

            bool result = fixture.SearchResultVerification();

            Assert.IsTrue(result, string.Format("File search result does not matches with expected value for path: {0}. See trace for more detail"
                , path));
        }
    }
}
