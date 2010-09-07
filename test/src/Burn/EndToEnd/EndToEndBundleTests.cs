//-----------------------------------------------------------------------
// <copyright file="EndToEndBundleTests.cs" company="Microsoft">
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
//     - Tests for Pri-0 end-to-end scenarios.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.EndToEnd
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests that cover pri-0 bundle scenarios.  
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
        [Description("verify you can build a bundle containing a variety of payload types in an attached conainer, install the bundle, verify the payloads were downloaded, installed, cached, the bundle was cached, ARP entry created, then uninstall the bundle via the ARP entry, verify the payloads were removed, caches and ARP entry were deleted.")]
        [Timeout(1800000)] // 30 minutes 
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnInstallUninstallBigBundle()
        {
            fixture.RunTest(BurnCommonTestFixture.InstallMode.install,
                BurnCommonTestFixture.UiMode.Silent,
                BurnCommonTestFixture.UserType.CurrentUser,
                BurnCommonTestFixture.PayloadLocation.InLocalLayout, 
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());
            // BUGBUG put this back once Bundles can set Metrics data to be collected
            //Assert.IsTrue(fixture.TestPasses(), "Failed!");
            Assert.IsTrue(fixture.TestPasses(false), "Failed Install!");

            fixture.RunUninstallFromArpTest(
                BurnCommonTestFixture.UiMode.Silent,
                BurnCommonTestFixture.UserType.CurrentUser);
            // BUGBUG put this back once Bundles can set Metrics data to be collected
            //Assert.IsTrue(fixture.TestPasses(), "Failed!");
            Assert.IsTrue(fixture.TestPasses(false), "Failed Uninstall from ARP!");
        }

        [TestMethod]
        [Description("verify you can build a bundle containing a variety of payload types to be downloaded, install the bundle, verify the payloads were downloaded, installed, cached, the bundle was cached, ARP entry created, then uninstall the bundle via the ARP entry, verify the payloads were removed, caches and ARP entry were deleted.")]
        [Timeout(1800000)] // 30 minutes 
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnInstallUninstallDownloadBundle()
        {
            fixture.RunTest(BurnCommonTestFixture.InstallMode.install,
                BurnCommonTestFixture.UiMode.Passive,
                BurnCommonTestFixture.UserType.CurrentUser,
                BurnCommonTestFixture.PayloadLocation.ToBeDownloaded);
            // BUGBUG put this back once Bundles can set Metrics data to be collected
            //Assert.IsTrue(fixture.TestPasses(), "Failed!");
            Assert.IsTrue(fixture.TestPasses(false), "Failed Install!");

            fixture.RunUninstallFromArpTest(
                BurnCommonTestFixture.UiMode.Passive,
                BurnCommonTestFixture.UserType.CurrentUser);
            // BUGBUG put this back once Bundles can set Metrics data to be collected
            //Assert.IsTrue(fixture.TestPasses(), "Failed!");
            Assert.IsTrue(fixture.TestPasses(false), "Failed Uninstall from ARP!");
        }

        [TestMethod]
        [Description("To verify payloads are uninstalled as a part of rollback")]
        [Timeout(1800000)] // 30 minutes 
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnInstallRollback()
        {
            fixture.RunRollbackTest(
                BurnCommonTestFixture.InstallMode.install, 
                BurnCommonTestFixture.UiMode.Passive, 
                BurnCommonTestFixture.UserType.CurrentUser, 
                BurnCommonTestFixture.PayloadLocation.InLocalLayout, 
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());

            Assert.IsTrue(fixture.TestPassesRollbackScenario(), "Failed Rollback scenario");
        }

        [TestMethod]
        [Description("To verify payloads are not uninstalled if error occurs and rollback is disabled")]
        [Timeout(1800000)] // 30 minutes 
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnInstallStop()
        {
            fixture.RunStopTest(
                BurnCommonTestFixture.InstallMode.install,
                BurnCommonTestFixture.UiMode.Passive,
                BurnCommonTestFixture.UserType.CurrentUser,
                BurnCommonTestFixture.PayloadLocation.InLocalLayout,
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());

            Assert.IsTrue(fixture.TestPassesStopScenario(), "Failed Stop scenario");
        }
    }
}
