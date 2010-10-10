//-----------------------------------------------------------------------
// <copyright file="ExternalFilesTests.cs" company="Microsoft">
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
//     - Tests for Burn External Files feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.ExternalFiles
{
    using System;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests that cover burnstub.exe sub File feature.  
    /// A File element that is a child of another item (i.e. an Msi)
    /// </summary>
    [TestClass]
    public class ExternalFilesTests : BurnTests
    {
        private ExternalFilesFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new ExternalFilesFixture();
            fixture.CleanUp();
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            fixture.CleanUp();
        }

        #endregion

        public void RunExternalFilesTest()
        {
            if (IsDataDrivenTestEnabled())
            {
                ExternalFilesFixture.PayloadType item1PayloadType = (ExternalFilesFixture.PayloadType)Enum.Parse(typeof(ExternalFilesFixture.PayloadType), (string)TestContext.DataRow[1], true);
                ExternalFilesFixture.ItemNames item1ItemName = (ExternalFilesFixture.ItemNames)Enum.Parse(typeof(ExternalFilesFixture.ItemNames), (string)TestContext.DataRow[2], true);
                ExternalFilesFixture.ExternalFileType item1FileType = (ExternalFilesFixture.ExternalFileType)Enum.Parse(typeof(ExternalFilesFixture.ExternalFileType), (string)TestContext.DataRow[3], true);
                ExternalFilesFixture.ItemNames item1FileName = (ExternalFilesFixture.ItemNames)Enum.Parse(typeof(ExternalFilesFixture.ItemNames), (string)TestContext.DataRow[4], true);
                ExternalFilesFixture.CacheState item1ItemCacheState = (ExternalFilesFixture.CacheState)Enum.Parse(typeof(ExternalFilesFixture.CacheState), (string)TestContext.DataRow[5], true);
                ExternalFilesFixture.CacheState item1FileCacheState = (ExternalFilesFixture.CacheState)Enum.Parse(typeof(ExternalFilesFixture.CacheState), (string)TestContext.DataRow[6], true);
                ExternalFilesFixture.PayloadType? item2PayloadType = null;
                ExternalFilesFixture.ItemNames? item2ItemName = null;
                ExternalFilesFixture.ExternalFileType? item2FileType = null;
                ExternalFilesFixture.ItemNames? item2FileName = null;
                ExternalFilesFixture.CacheState? item2ItemCacheState = null;
                ExternalFilesFixture.CacheState? item2FileCacheState = null;
                try
                {
                    item2PayloadType = (ExternalFilesFixture.PayloadType)Enum.Parse(typeof(ExternalFilesFixture.PayloadType), (string)TestContext.DataRow[7], true);
                    item2ItemName = (ExternalFilesFixture.ItemNames)Enum.Parse(typeof(ExternalFilesFixture.ItemNames), (string)TestContext.DataRow[8], true);
                    item2FileType = (ExternalFilesFixture.ExternalFileType)Enum.Parse(typeof(ExternalFilesFixture.ExternalFileType), (string)TestContext.DataRow[9], true);
                    item2FileName = (ExternalFilesFixture.ItemNames)Enum.Parse(typeof(ExternalFilesFixture.ItemNames), (string)TestContext.DataRow[10], true);
                    item2ItemCacheState = (ExternalFilesFixture.CacheState)Enum.Parse(typeof(ExternalFilesFixture.CacheState), (string)TestContext.DataRow[11], true);
                    item2FileCacheState = (ExternalFilesFixture.CacheState)Enum.Parse(typeof(ExternalFilesFixture.CacheState), (string)TestContext.DataRow[12], true);
                }
                catch
                {
                    // it's ok if the item2 stuff fails to read since that can be null.
                }
                ExternalFilesFixture.UserType userType = (ExternalFilesFixture.UserType)Enum.Parse(typeof(ExternalFilesFixture.UserType), (string)TestContext.DataRow[13], true);

                fixture.RunTest(item1PayloadType,
                    item1ItemName,
                    item1FileType,
                    item1FileName,
                    item1ItemCacheState,
                    item1FileCacheState,
                    item2PayloadType,
                    item2ItemName,
                    item2FileType,
                    item2FileName,
                    item2ItemCacheState,
                    item2FileCacheState,
                    userType);
                Assert.IsTrue(fixture.TestPasses(), "Failed!");
            }
        }

        [TestMethod]
        [Description("verify bundle packages with external files scenarios")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\ExternalFiles\ExternalFilesData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=ExternalFilesData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Install$",
            DataAccessMethod.Sequential)]
        public void BurnExternalFilesDataDriven()
        {
            RunExternalFilesTest();
        }

        [TestMethod]
        [Description("verify bundle packages with external files scenarios")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\ExternalFiles\ExternalFilesData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=ExternalFilesData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "InstallPerUser$",
            DataAccessMethod.Sequential)]
        public void BurnExternalFilesPerUserDataDriven()
        {
            RunExternalFilesTest();
        }        
    }
}
