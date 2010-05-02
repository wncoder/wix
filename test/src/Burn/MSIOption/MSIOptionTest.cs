//-----------------------------------------------------------------------
// <copyright file="MSIOptionFixture.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Tests for Burn MSIOption feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.MSIOption
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.MSIOption;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class MSIOptionTest : BurnTests
    {
        private MSIOptionFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new MSIOptionFixture();
            fixture.UninstallMsi();
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            fixture.UninstallMsi();
        }

        #endregion

        [TestMethod]
        [Description("Setting Msi property at install time, PASSME=1")]
        [Timeout(100000)] // 10 minutes
        [TestProperty("IsRuntimeTest", "true")]
        public void MsiProperty_Install_Success()
        {
            fixture.BuildLayout("PASSME", "1");
            fixture.LaunchBurn(Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture.BurnCommonTestFixture.InstallMode.install);
            bool result = fixture.Verify();

            Assert.IsTrue(result, string.Format("Msi failed to install. See trace for more detail"));
        }

        [TestMethod]
        [Description("Setting Msi property at uninstall time, PASSME=1")]
        [Timeout(100000)] // 10 minutes
        [TestProperty("IsRuntimeTest", "true")]
        public void MsiProperty_Uninstall_Success()
        {
            fixture.BuildLayout("PASSME", "1");
            fixture.InstallMsi();

            fixture.LaunchBurn(Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture.BurnCommonTestFixture.InstallMode.uninstall);
            bool result = fixture.Verify();

            Assert.IsFalse(result, string.Format("Msi failed to uninstall. See trace for more detail"));

        }

        [TestMethod]
        [Description("Setting Msi property at repair time, PASSME=1")]
        [Timeout(100000)] // 10 minutes
        [TestProperty("IsRuntimeTest", "true")]
        public void MsiProperty_Repair_Success()
        {
            fixture.BuildLayout("PASSME", "1");
            fixture.InstallMsi();
            fixture.LaunchBurn(Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture.BurnCommonTestFixture.InstallMode.repair);
            bool result = fixture.Verify();

            Assert.IsTrue(result, string.Format("Msi failed to repair. See trace for more detail"));
        }

        [TestMethod]
        [Description("Setting Msi property at install time, PASSME=2 (fail)")]
        [Timeout(100000)] // 10 minutes
        [TestProperty("IsRuntimeTest", "true")]
        public void MsiProperty_Install_Fail()
        {
            fixture.BuildLayout("PASSME", "2");
            fixture.LaunchBurn(Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture.BurnCommonTestFixture.InstallMode.install);
            bool result = fixture.Verify();

            Assert.IsFalse(result, string.Format("Msi install should fail. See trace for more detail"));
        }

        [TestMethod]
        [Description("Setting Msi property at uninstall time, PASSME=2 (fail)")]
        [Timeout(100000)] // 10 minutes
        [TestProperty("IsRuntimeTest", "true")]
        public void MsiProperty_Uninstall_Fail()
        {
            fixture.BuildLayout("PASSME", "2");
            fixture.InstallMsi();

            fixture.LaunchBurn(Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture.BurnCommonTestFixture.InstallMode.uninstall);
            bool result = fixture.Verify();

            Assert.IsTrue(result, string.Format("Msi uninstall should fail. See trace for more detail"));

        }

        [TestMethod]
        [Description("Setting Msi property at install time, PASSME=1")]
        [Timeout(100000)] // 10 minutes
        [TestProperty("IsRuntimeTest", "true")]
        public void MsiProperty_Install_VariableSuccess()
        {
            fixture.BuildLayout("PASSME", "[varMsiProperty]", "varMsiProperty", "1"
                , Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Variables.VariableElement.VariableDataType.String);
            fixture.LaunchBurn(Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture.BurnCommonTestFixture.InstallMode.install);
            bool result = fixture.Verify();

            Assert.IsTrue(result, string.Format("Msi failed to install. See trace for more detail"));
        }
    }
}
