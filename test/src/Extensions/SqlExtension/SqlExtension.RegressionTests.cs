//-----------------------------------------------------------------------
// <copyright file="SqlExtensionTests.cs" company="Microsoft">
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
// <summary>Sql extension tests</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Sql extension tests
    /// </summary>
    [TestClass]
    public class SqlExtensionTests
    {
        [TestMethod]
        [Priority(3)]
        public void SqlExtensionCustomActionTest01()
        {
            string testDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Extensions\SqlExtension\RegressionTests\CustomActions");
            string msi = Builder.BuildPackage(testDirectory, "product.wxs", "product.msi", " -ext WixSqlExtension", " -cultures:en-us -ext WixSqlExtension");

            string query = "SELECT `Source` FROM `CustomAction` WHERE `Action` = 'InstallSqlData'";
            Assert.AreEqual("ScaSchedule2", Verifier.Query(msi, query), @"Unexpected value in {0} returned by ""{1}""", msi, query);

            query = "SELECT `Source` FROM `CustomAction` WHERE `Action` = 'UninstallSqlData'";
            Assert.AreEqual("ScaSchedule2", Verifier.Query(msi, query), @"Unexpected value in {0} returned by ""{1}""", msi, query);
        }
    }
}
