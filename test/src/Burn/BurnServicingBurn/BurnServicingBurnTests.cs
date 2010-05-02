//-----------------------------------------------------------------------
// <copyright file="BurnServicingBurnTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Tests for Burn servicing Burn scenarios.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.BurnServicingBurn
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests that cover scenarios that verify the 'Burn Servicing Burn' feature.  
    /// </summary>
    [TestClass]
    public class BurnServicingBurnTests : BurnTests
    {
        private BurnServicingBurnFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new BurnServicingBurnFixture();
            fixture.CleanUp();
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            fixture.CleanUp();
        }

        #endregion

        private BurnCommonTestFixture.BundleType bundleType;
        private BurnServicingBurnFixture.UpgradeUpdateType upgradeUpdateType;
        private BurnCommonTestFixture.PayloadOutcome payloadOutcome;
        private BurnCommonTestFixture.InstallMode installMode;
        private BurnCommonTestFixture.UiMode uiMode;
        private BurnCommonTestFixture.UserType userType;

        public void SetData()
        {
            bundleType = (BurnCommonTestFixture.BundleType)Enum.Parse(typeof(BurnCommonTestFixture.BundleType), (string)TestContext.DataRow[0], true);
            upgradeUpdateType = (BurnServicingBurnFixture.UpgradeUpdateType)Enum.Parse(typeof(BurnServicingBurnFixture.UpgradeUpdateType), (string)TestContext.DataRow[1], true);
            payloadOutcome = (BurnCommonTestFixture.PayloadOutcome)Enum.Parse(typeof(BurnCommonTestFixture.PayloadOutcome), (string)TestContext.DataRow[2], true);
            installMode = (BurnCommonTestFixture.InstallMode)Enum.Parse(typeof(BurnCommonTestFixture.InstallMode), (string)TestContext.DataRow[3], true);
            uiMode = (BurnCommonTestFixture.UiMode)Enum.Parse(typeof(BurnCommonTestFixture.UiMode), (string)TestContext.DataRow[4], true);
            userType = (BurnCommonTestFixture.UserType)Enum.Parse(typeof(BurnCommonTestFixture.UserType), (string)TestContext.DataRow[5], true);
        }

        public void BurnBurnServicingBurnExecuteTest()
        {
            SetData();
            fixture.RunTest(bundleType,
                payloadOutcome,
                upgradeUpdateType,
                installMode,
                uiMode,
                userType);
            Assert.IsTrue(fixture.TestPasses(), "Failed!");
        }

        [TestMethod]
        [Description("verify the burnstub.exe will launch without blowing up because stuff is missing in the layout or the parameterinfo.xml isn't formed correctly")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\BurnServicingBurn\BurnServicingBurnData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=BurnServicingBurnData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "InstallPerMachineAdmin$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnBurnServicingBurnInstallPerMachineAdminDataDriven()
        {
            BurnBurnServicingBurnExecuteTest();
        }

        [TestMethod]
        [Description("verify the burnstub.exe will launch without blowing up because stuff is missing in the layout or the parameterinfo.xml isn't formed correctly")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\BurnServicingBurn\BurnServicingBurnData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=BurnServicingBurnData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "InstallPerUserAdmin$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnBurnServicingBurnInstallPerUserAdminDataDriven()
        {
            BurnBurnServicingBurnExecuteTest();
        }

        [TestMethod]
        [Description("verify the burnstub.exe will launch without blowing up because stuff is missing in the layout or the parameterinfo.xml isn't formed correctly")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\BurnServicingBurn\BurnServicingBurnData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=BurnServicingBurnData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "InstallPerUserNormalUser$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnBurnServicingBurnInstallPerUserNormalUserDataDriven()
        {
            BurnBurnServicingBurnExecuteTest();
        }

        [TestMethod]
        [Description("verify the burnstub.exe will launch without blowing up because stuff is missing in the layout or the parameterinfo.xml isn't formed correctly")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\BurnServicingBurn\BurnServicingBurnData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=BurnServicingBurnData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "InstallFail$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnBurnServicingBurnInstallFailDataDriven()
        {
            BurnBurnServicingBurnExecuteTest();
        }

        [TestMethod]
        [Description("verify the burnstub.exe will launch without blowing up because stuff is missing in the layout or the parameterinfo.xml isn't formed correctly")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\BurnServicingBurn\BurnServicingBurnData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=BurnServicingBurnData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Uninstall$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnBurnServicingBurnUnnstallUpdateDataDriven()
        {
            BurnBurnServicingBurnExecuteTest();
        }

        [TestMethod]
        [Description("verify the burnstub.exe will launch without blowing up because stuff is missing in the layout or the parameterinfo.xml isn't formed correctly")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\BurnServicingBurn\BurnServicingBurnData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=BurnServicingBurnData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Uninstall$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnBurnServicingBurnUnnstallTargetDataDriven()
        {
            SetData();
            fixture.RunTest(bundleType,
                payloadOutcome,
                upgradeUpdateType,
                installMode,
                uiMode,
                userType, 
                BurnServicingBurnFixture.TestMode.UninstallUpdateTarget);
            Assert.IsTrue(fixture.TestPasses(), "Failed!");
        }

        [TestMethod]
        [Description("verify the burnstub.exe will launch without blowing up because stuff is missing in the layout or the parameterinfo.xml isn't formed correctly")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\BurnServicingBurn\BurnServicingBurnData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=BurnServicingBurnData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Repair$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnBurnServicingBurnRepairDataDriven()
        {
            SetData();
            fixture.RunTest(bundleType,
                payloadOutcome,
                upgradeUpdateType,
                installMode,
                uiMode,
                userType,
                BurnServicingBurnFixture.TestMode.Repair);
            Assert.IsTrue(fixture.TestPasses(), "Failed!");
        }
    }
}
