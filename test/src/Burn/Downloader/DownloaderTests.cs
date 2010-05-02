//-----------------------------------------------------------------------
// <copyright file="DownloaderTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Tests for Burn downloader feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Downloader
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests that cover burnstub.exe download scenarios.  
    /// These are things that must work.  
    /// Any failure would be considered a recall class bug.
    /// </summary>
    [TestClass]
    public class DownloaderTests : BurnTests
    {
        private DownloaderFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new DownloaderFixture();
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
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Downloader\DownloaderData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=DownloaderData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Install$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnDownloaderDataDriven()
        {
            DownloaderFixture.Scenario scenario = (DownloaderFixture.Scenario)Enum.Parse(typeof(DownloaderFixture.Scenario), (string)TestContext.DataRow[0], true);
            DownloaderFixture.DownloadProtocol protocol = (DownloaderFixture.DownloadProtocol)Enum.Parse(typeof(DownloaderFixture.DownloadProtocol), (string)TestContext.DataRow[1], true);
            DownloaderFixture.IgnoreDownloadFailure ignoreDlFailure = (DownloaderFixture.IgnoreDownloadFailure)Enum.Parse(typeof(DownloaderFixture.IgnoreDownloadFailure), (string)TestContext.DataRow[2], true);
            DownloaderFixture.SerialDownloadConfiguration serialDlConfiguration = (DownloaderFixture.SerialDownloadConfiguration)Enum.Parse(typeof(DownloaderFixture.SerialDownloadConfiguration), (string)TestContext.DataRow[3], true);
            DownloaderFixture.SerialDownloadSwitch serialDlSwitch = (DownloaderFixture.SerialDownloadSwitch)Enum.Parse(typeof(DownloaderFixture.SerialDownloadSwitch), (string)TestContext.DataRow[4], true);
            DownloaderFixture.DownloadCacheState dlCacheState = (DownloaderFixture.DownloadCacheState)Enum.Parse(typeof(DownloaderFixture.DownloadCacheState), (string)TestContext.DataRow[5], true);
            DownloaderFixture.ItemNames itemNames = (DownloaderFixture.ItemNames)Enum.Parse(typeof(DownloaderFixture.ItemNames), (string)TestContext.DataRow[6], true);
            BurnCommonTestFixture.UserType userType = (BurnCommonTestFixture.UserType)Enum.Parse(typeof(BurnCommonTestFixture.UserType), (string)TestContext.DataRow[7], true);

            fixture.RunTest(scenario,
                protocol,
                ignoreDlFailure,
                serialDlConfiguration,
                serialDlSwitch,
                dlCacheState,
                itemNames,
                userType);
            Assert.IsTrue(fixture.TestPasses(), "Failed!");
        }
    }
}
