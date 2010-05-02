//-----------------------------------------------------------------------
// <copyright file="BurnVariableTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Tests for Burn Variables feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Variables
    
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnVariable;

    [TestClass]
    public class BurnVariableTests : BurnTests
    {
        private BurnVariableFixture fixture;

        #region Additional test attributes
        [TestInitialize()]
        public void MyTestInitialize()
        {
            WixTests.SetTraceToOutputToConsole();

            fixture = new BurnVariableFixture();
            fixture.CleanUp();
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            fixture.CleanUp();
        }

        #endregion

        [TestMethod]
        [Description("Variable Get and Set verification")]
        [Timeout(1800000)] // 30 minutes 
        [DeploymentItem(@"%WIX_ROOT%\test\src\Burn\Variables\VariableData.xlsx")]
        [DataSource("System.Data.OleDb",
            "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=VariableData.xlsx;Extended Properties=\"Excel 12.0 Xml;HDR=YES;IMEX=1\"",
            "Install$",
            DataAccessMethod.Sequential)]
        [TestProperty("IsRuntimeTest", "true")]
        public void BurnVariableGetSetDataDriven()
        {
            string id = (string)TestContext.DataRow[0];

            string variableType = (string)TestContext.DataRow[1];

            if (variableType.ToLower().Trim() != "string")
            {
                variableType = variableType.ToLower();
            }

            VariableDataType type = (VariableDataType)Enum.Parse(typeof(VariableDataType)
              , variableType, true);

            string value = (string)TestContext.DataRow[2];

            fixture.CreateVariableElement(id, type, value);

            bool result = fixture.CompareVariableValue();

            Assert.IsTrue(result, string.Format("Get and Set for variable: {0} does not matches. Variable Type:{1} and Variable Value:{2} Please see the trace for more detail"
                , id, type.ToString(), value));
        }

        [TestMethod]
        [Description("Built-in variables verification")]
        [Timeout(1800000)] // 30 minutes       
        public void BurnBuiltInVariables()
        {
            fixture.PrepareLayout();
            bool result = fixture.VerifyBuiltInVariables();

            Assert.IsTrue(result, "Built-in variables verification failed. See trace for more detail");

        }
    }
}
