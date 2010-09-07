//-----------------------------------------------------------------------
// <copyright file="RegistrySearchTests.cs" company="Microsoft">
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
//     - Tests for Burn Registry Search feature.
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
    public class RegistrySearchTests : BurnTests
    {
        private BurnRegistrySearchFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new BurnRegistrySearchFixture();
            fixture.CleanUp();
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            fixture.CleanUp();
        }

        #endregion

        [TestMethod]
        [Description("Registry search")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Searches\RegistrySearch\RegistrySearchData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=RegistrySearchData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Install$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnRegistrySearchDataDriven()
        {
            string id = (string)TestContext.DataRow[0];

            RegistrySearchElement.RegistrySearchResultType type = (RegistrySearchElement.RegistrySearchResultType)Enum.Parse
                (typeof(RegistrySearchElement.RegistrySearchResultType), (string)TestContext.DataRow[1], true);

            RegistrySearchElement.RegRoot root = (RegistrySearchElement.RegRoot)Enum.Parse(typeof(RegistrySearchElement.RegRoot), (string)TestContext.DataRow[2], true);

            string subKey = @TestContext.DataRow[3].ToString();

            string registryKey = TestContext.DataRow[4].ToString();

            string registryValue = TestContext.DataRow[5].ToString();

            string value = TestContext.DataRow[6].ToString();

            string variableType = TestContext.DataRow[7].ToString();

            RegistrySearchElement.YesNoType expandEnvironmentVar = (RegistrySearchElement.YesNoType)Enum.Parse(typeof(RegistrySearchElement.YesNoType), (string)TestContext.DataRow[8], true);

            string variable = (string)TestContext.DataRow[9];

            fixture.CreateRegistrySearchElement(id, subKey, root, type, value, variable, expandEnvironmentVar, registryKey, registryValue);

            bool result = fixture.SearchResultVerification(variableType, root, variable, registryKey, subKey, type);

            Assert.IsTrue(result, string.Format("Registry search returned incorrect result for: {0}. Please see trace for more detail."
                , subKey));
        }
    }
}
