//-----------------------------------------------------------------------
// <copyright file="EndToEndFailureScenariosTests.cs" company="Microsoft">
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
//     - Test Fixture for Pri-0 end-to-end scenarios.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.EndToEnd
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests that cover burnstub.exe pri-0 scenarios.  
    /// These are things that must work.  
    /// Any failure would be considered a recall class bug.
    /// </summary>
    [TestClass]
    public class EndToEndFailureScenariosTests : BurnTests
    {
        private EndToEndFailureScenariosFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new EndToEndFailureScenariosFixture();
            fixture.CleanUp();
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            fixture.CleanUp();
        }

        #endregion

        [TestMethod]
        [Description("verify bundle failure scenarios")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\EndToEnd\EndToEndFailureScenariosData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=EndToEndFailureScenariosData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Install$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnInstallFailDataDriven()
        {
            if (IsDataDrivenTestEnabled())
            {
                BurnCommonTestFixture.PayloadType payload1Type = (BurnCommonTestFixture.PayloadType)Enum.Parse(typeof(BurnCommonTestFixture.PayloadType), (string)TestContext.DataRow[1], true);
                BurnCommonTestFixture.PayloadOutcome payload1Outcome = (BurnCommonTestFixture.PayloadOutcome)Enum.Parse(typeof(BurnCommonTestFixture.PayloadOutcome), (string)TestContext.DataRow[2], true);
                BurnCommonTestFixture.PayloadType payload2Type = (BurnCommonTestFixture.PayloadType)Enum.Parse(typeof(BurnCommonTestFixture.PayloadType), (string)TestContext.DataRow[3], true);
                BurnCommonTestFixture.PayloadOutcome payload2Outcome = (BurnCommonTestFixture.PayloadOutcome)Enum.Parse(typeof(BurnCommonTestFixture.PayloadOutcome), (string)TestContext.DataRow[4], true);
                BurnCommonTestFixture.InstallMode installMode = (BurnCommonTestFixture.InstallMode)Enum.Parse(typeof(BurnCommonTestFixture.InstallMode), (string)TestContext.DataRow[5], true);
                BurnCommonTestFixture.UiMode uiMode = (BurnCommonTestFixture.UiMode)Enum.Parse(typeof(BurnCommonTestFixture.UiMode), (string)TestContext.DataRow[6], true);
                BurnCommonTestFixture.UserType userType = (BurnCommonTestFixture.UserType)Enum.Parse(typeof(BurnCommonTestFixture.UserType), (string)TestContext.DataRow[7], true);

                fixture.RunTest(payload1Type,
                    payload1Outcome,
                    payload2Type,
                    payload2Outcome,
                    installMode,
                    uiMode,
                    userType);
                Assert.IsTrue(fixture.TestPasses(), "Failed!");
            }
        }

    }
}
