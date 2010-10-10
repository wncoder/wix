//-----------------------------------------------------------------------
// <copyright file="IISExtension.IISWebErrorTests.cs" company="Microsoft">
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
// <summary>IIS Extension WebError tests</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Extensions.IISExtension
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers.Extensions;


    /// <summary>
    /// IIS extension WebError element tests
    /// </summary>
    [TestClass]
    public class IISWebErrorTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Extensions\IISExtension\IISWebErrorTests");

        [TestMethod]
        [Description("Verify the msi table data in CustomAction table and WebError table.")]
        [Priority(1)]
        public void IISWebError_VerifyMSITableData()
        {
            string sourceFile = Path.Combine(IISWebErrorTests.TestDataDirectory, @"product.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixIIsExtension");

            Verifier.VerifyCustomActionTableData(msiFile,
                new CustomActionTableData("ConfigureIIs", 1, "IIsSchedule", "ConfigureIIs"),
                new CustomActionTableData("ConfigureIIsExec", 3073, "IIsSchedule", "ConfigureIIsExec"),
                new CustomActionTableData("StartMetabaseTransaction", 11265, "IIsExecute", "StartMetabaseTransaction"),
                new CustomActionTableData("RollbackMetabaseTransaction", 11521, "IIsExecute", "RollbackMetabaseTransaction"),
                new CustomActionTableData("CommitMetabaseTransaction", 11777, "IIsExecute", "CommitMetabaseTransaction"),
                new CustomActionTableData("WriteMetabaseChanges", 11265, "IIsExecute", "WriteMetabaseChanges"));

            Verifier.VerifyTableData(msiFile, MSITables.IIsWebError,
                new TableRow(IIsWebErrorColumns.ErrorCode.ToString(), "400", false),
                new TableRow(IIsWebErrorColumns.SubCode.ToString(), "0", false),
                new TableRow(IIsWebErrorColumns.ParentType.ToString(), "2", false),
                new TableRow(IIsWebErrorColumns.ParentValue.ToString(), "Test"),
                new TableRow(IIsWebErrorColumns.File.ToString(), "[#Error404]"),
                new TableRow(IIsWebErrorColumns.URL.ToString(), string.Empty));

            Verifier.VerifyTableData(msiFile, MSITables.IIsWebError,
               new TableRow(IIsWebErrorColumns.ErrorCode.ToString(), "401", false),
               new TableRow(IIsWebErrorColumns.SubCode.ToString(), "1", false),
               new TableRow(IIsWebErrorColumns.ParentType.ToString(), "2", false),
               new TableRow(IIsWebErrorColumns.ParentValue.ToString(), "Test"),
               new TableRow(IIsWebErrorColumns.File.ToString(), "[#Error404]"),
               new TableRow(IIsWebErrorColumns.URL.ToString(), string.Empty));

            Verifier.VerifyTableData(msiFile, MSITables.IIsWebError,
               new TableRow(IIsWebErrorColumns.ErrorCode.ToString(), "401", false),
               new TableRow(IIsWebErrorColumns.SubCode.ToString(), "7", false),
               new TableRow(IIsWebErrorColumns.ParentType.ToString(), "2", false),
               new TableRow(IIsWebErrorColumns.ParentValue.ToString(), "Test"),
               new TableRow(IIsWebErrorColumns.File.ToString(), "[#Error404]"),
               new TableRow(IIsWebErrorColumns.URL.ToString(), string.Empty));

            Verifier.VerifyTableData(msiFile, MSITables.IIsWebError,
               new TableRow(IIsWebErrorColumns.ErrorCode.ToString(), "401", false),
               new TableRow(IIsWebErrorColumns.SubCode.ToString(), "61", false),
               new TableRow(IIsWebErrorColumns.ParentType.ToString(), "2", false),
               new TableRow(IIsWebErrorColumns.ParentValue.ToString(), "Test"),
               new TableRow(IIsWebErrorColumns.File.ToString(), "[#Error404]"),
               new TableRow(IIsWebErrorColumns.URL.ToString(), string.Empty));

        }
    }
}
