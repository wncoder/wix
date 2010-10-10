//-----------------------------------------------------------------------
// <copyright file="FileSearchTests.cs" company="Microsoft">
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
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Searches;

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

            FileSearchElement.FileSearchResultType resultType = (FileSearchElement.FileSearchResultType)Enum.Parse
                (typeof(FileSearchElement.FileSearchResultType), (string)TestContext.DataRow[1], true);

            string path = Environment.ExpandEnvironmentVariables(TestContext.DataRow[2].ToString());

            string variable = (string)TestContext.DataRow[3];

            string ManifestAuthoringPath = TestContext.DataRow[4].ToString();

            fixture.CreateFileSearchElement(id, resultType, ManifestAuthoringPath, variable);

            bool result = fixture.SearchResultVerification(resultType, variable, path);

            Assert.IsTrue(result, string.Format("File search result does not matches with expected value for path: {0}. See trace for more detail"
                , path));
        }
    }
}
