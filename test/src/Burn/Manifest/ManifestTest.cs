//-----------------------------------------------------------------------
// <copyright file="ManifestTest.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Tests for Burn Manifest feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Manifest
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Variables;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Manifest;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class ManifestTest : BurnTests
    {
        private ManifestFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new ManifestFixture();
            fixture.UninstallMsi();
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            fixture.LaunchBurn(Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture.BurnCommonTestFixture.InstallMode.uninstall);
            fixture.UninstallMsi();
        }

        #endregion

        [TestMethod]
        [Description("Condition evalulation verification for compound statement")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Manifest\ManifestTestData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=ManifestTestData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Install$",
            DataAccessMethod.Sequential)]
        public void BurnInstallConditionInstallDataDriven()
        {
            AddVariable((string)TestContext.DataRow[0], TestContext.DataRow[1].ToString(), TestContext.DataRow[2].ToString());
            AddVariable((string)TestContext.DataRow[3], TestContext.DataRow[4].ToString(), TestContext.DataRow[5].ToString());

            string condStatement = (string)TestContext.DataRow[6];
            condStatement = condStatement.Replace("&qt;", "'").Replace("<", "&lt;").Replace(">", "&gt;");

            fixture.BuildLayout(condStatement, string.Empty);

            fixture.LaunchBurn(Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture.BurnCommonTestFixture.InstallMode.maintenanceMode);

            bool result = fixture.Verify();

            Assert.IsTrue(result, string.Format("Msi failed to install. See trace for more detail"));
        }

        [TestMethod]
        [Description("Condition evalulation verification for compound statement")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Manifest\ManifestTestData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=ManifestTestData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Uninstall$",
            DataAccessMethod.Sequential)]
        public void BurnInstallConditionUninstallDataDriven()
        {
            fixture.InstallMsi();

            AddVariable((string)TestContext.DataRow[0], TestContext.DataRow[1].ToString(), TestContext.DataRow[2].ToString());
            AddVariable((string)TestContext.DataRow[3], TestContext.DataRow[4].ToString(), TestContext.DataRow[5].ToString());

            string condStatement = (string)TestContext.DataRow[6];
            condStatement = condStatement.Replace("&qt;", "'").Replace("<", "&lt;").Replace(">", "&gt;");

            fixture.BuildLayout(condStatement, string.Empty);

            fixture.LaunchBurn(Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture.BurnCommonTestFixture.InstallMode.maintenanceMode);

            bool result = fixture.Verify();

            Assert.IsFalse(result, string.Format("Msi failed to uninstall. See trace for more detail"));
        }

        [TestMethod]
        [Description("Condition evalulation verification for compound statement")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Manifest\ManifestTestData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=ManifestTestData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Rollback$",
            DataAccessMethod.Sequential)]
        public void BurnRollbackInstallConditionUninstallDataDriven()
        {
            AddVariable((string)TestContext.DataRow[0], TestContext.DataRow[1].ToString(), TestContext.DataRow[2].ToString());
            AddVariable((string)TestContext.DataRow[3], TestContext.DataRow[4].ToString(), TestContext.DataRow[5].ToString());

            string condStatement = (string)TestContext.DataRow[6];
            condStatement = condStatement.Replace("&qt;", "'").Replace("<", "&lt;").Replace(">", "&gt;");

            fixture.BuildLayoutRollback(condStatement);

            fixture.LaunchBurn(Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture.BurnCommonTestFixture.InstallMode.maintenanceMode);

            bool result = fixture.Verify();

            Assert.IsFalse(result, string.Format("Msi failed to uninstall. See trace for more detail"));

        }

        [TestMethod]
        [Description("Condition evalulation verification for compound statement")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Manifest\ManifestTestData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=ManifestTestData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Rollback$",
            DataAccessMethod.Sequential)]
        public void BurnRollbackInstallConditionUnstallDataDriven()
        {
            AddVariable((string)TestContext.DataRow[0], TestContext.DataRow[1].ToString(), TestContext.DataRow[2].ToString());
            AddVariable((string)TestContext.DataRow[3], TestContext.DataRow[4].ToString(), TestContext.DataRow[5].ToString());

            string condStatement = (string)TestContext.DataRow[6];
            condStatement = condStatement.Replace("&qt;", "'").Replace("<", "&lt;").Replace(">", "&gt;");

            fixture.BuildLayoutRollback(condStatement);

            fixture.LaunchBurn(Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture.BurnCommonTestFixture.InstallMode.maintenanceMode);

            bool result = fixture.Verify();

            Assert.IsFalse(result, string.Format("Msi failed to uninstall. See trace for more detail"));

        }


        private void AddVariable(string id, string variableType, string value)
        {
            if (variableType.ToLower().Trim() != "string")
            {
                variableType = variableType.ToLower();
            }

            VariableElement.VariableDataType type = (VariableElement.VariableDataType)Enum.Parse(typeof(VariableElement.VariableDataType)
              , variableType, true);

            fixture.AddVariableElement(id, value, type);            
        }
    }
}
