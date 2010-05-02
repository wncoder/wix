//-----------------------------------------------------------------------
// <copyright file="EndToEndBundleTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Tests for Pri-0 end-to-end scenarios.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.EndToEnd
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests that cover burn.exe pri-0 scenarios.  
    /// These are things that must work.  
    /// Any failure would be considered a recall class bug.
    /// </summary>
    [TestClass]
    public class EndToEndBundleTests : BurnTests
    {
        private EndToEndBundleFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new EndToEndBundleFixture();
            fixture.CleanUp();
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            fixture.CleanUp();
        }

        #endregion

        [TestMethod]
        [Description("verify the burnstub.exe will launch without blowing up because stuff is missing in the layout or the parameterinfo.xml isn't formed correctly")]
        [Timeout(1800000)] // 30 minutes 
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnInstallUninstallBigBundle()
        {
            BurnCommonTestFixture.InstallMode installMode = BurnCommonTestFixture.InstallMode.install;
            BurnCommonTestFixture.UiMode uiMode = BurnCommonTestFixture.UiMode.Passive;
            BurnCommonTestFixture.UserType userType = BurnCommonTestFixture.UserType.CurrentUser;

            fixture.RunTest(installMode,
                uiMode,
                userType);
            // BUGBUG put this back once Bundles can set Metrics data to be collected
            //Assert.IsTrue(fixture.TestPasses(), "Failed!");
            Assert.IsTrue(fixture.TestPasses(false), "Failed Install!");

            installMode = BurnCommonTestFixture.InstallMode.uninstall;
            fixture.RunUninstallFromArpTest(
                uiMode,
                userType);
            // BUGBUG put this back once Bundles can set Metrics data to be collected
            //Assert.IsTrue(fixture.TestPasses(), "Failed!");
            Assert.IsTrue(fixture.TestPasses(false), "Failed Uninstall from ARP!");
        }
    }
}
