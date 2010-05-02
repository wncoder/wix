//-----------------------------------------------------------------------
// <copyright file="BurnConditionTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Tests for Burn Condition feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Condition
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Variables;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Condition;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class BurnConditionTests : BurnTests
    {
        private BurnConditionFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new BurnConditionFixture();
            fixture.CleanUp();
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            fixture.CleanUp();
        }

        #endregion

        [TestMethod]
        [Description("Condition evalulation verification")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Condition\BurnConditionData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=BurnConditionData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Install$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnConditionDataDriven()
        {
            string id = (string)TestContext.DataRow[0];

            string variableType = (string)TestContext.DataRow[1];

            if (variableType.ToLower().Trim() != "string")
            {
                variableType = variableType.ToLower();
            }

            VariableElement.VariableDataType type = (VariableElement.VariableDataType)Enum.Parse(typeof(VariableElement.VariableDataType)
              , variableType, true);

            string value = (string)TestContext.DataRow[2];

            fixture.AddVariableElement(id, value, type);

            string condStatement = (string)TestContext.DataRow[3];

            if (type == VariableElement.VariableDataType.String)
            {
                condStatement = condStatement.Replace("&qt;", "\"");              
            }

            string expectedReturnVal = TestContext.DataRow[4].ToString();
            string expectedEvalResult = TestContext.DataRow[5].ToString();

            bool result = fixture.EvaluateCondition(condStatement, expectedReturnVal, Convert.ToBoolean(expectedEvalResult.ToLower()));

            Assert.IsTrue(result, string.Format("Eval condition for statement: {0} failed. See the trace for more detail"
                , condStatement));
        }


        [TestMethod]
        [Description("Condition evalulation verification for compound statement")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Condition\BurnConditionData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=BurnConditionData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "CompoundStatement$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnCompoundConditionDataDriven()
        {
            AddVariable((string)TestContext.DataRow[0], TestContext.DataRow[1].ToString(), TestContext.DataRow[2].ToString());
            AddVariable((string)TestContext.DataRow[3], TestContext.DataRow[4].ToString(), TestContext.DataRow[5].ToString());

            string condStatement = (string)TestContext.DataRow[6];
            condStatement = condStatement.Replace("&qt;", "\"");

            string expectedReturnVal = TestContext.DataRow[7].ToString();
            string expectedEvalResult = TestContext.DataRow[8].ToString();

            bool result = fixture.EvaluateCondition(condStatement, expectedReturnVal, Convert.ToBoolean(expectedEvalResult.ToLower()));

            Assert.IsTrue(result, string.Format("Eval condition for statement: {0} failed. See the trace for more detail"
                , condStatement));
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
