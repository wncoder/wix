//-----------------------------------------------------------------------
// <copyright file="BurnVariableTests.cs" company="Microsoft">
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
//     - Tests for Burn Variables feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Variables
    
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Variable;

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
            string name = (string)TestContext.DataRow[0];

            string variableType = (string)TestContext.DataRow[1];

            if (variableType.ToLower().Trim() != "string")
            {
                variableType = variableType.ToLower();
            }

            VariableElement.VariableDataType type = (VariableElement.VariableDataType)Enum.Parse(typeof(VariableElement.VariableDataType)
              , variableType, true);

            string value = (string)TestContext.DataRow[2];

            fixture.CreateVariableElement(name, type, value);

            fixture.PrepareLayout();

            fixture.GetSetVariable(type, name, value);

            bool result = fixture.CompareVariableValue(type, name, value);

            Assert.IsTrue(result, string.Format("Get and Set for variable: {0} does not matches. Variable Type:{1} and Variable Value:{2} Please see the trace for more detail"
                , name, type.ToString(), value));
        }

        [TestMethod]
        [Description("To verify variable values are persistent after setup is re-launched")]
        [Timeout(600000)] // 10 minutes
        public void BurnPersistentVariable()
        {
            // Add string variable
            string varStrName = "stringVar";
            string varStrValue = "test test 123";
            fixture.CreateVariableElement(varStrName, VariableElement.VariableDataType.String, varStrValue);

            // Add numeric variable
            string varNumName = "numericVar";
            string varNumValue = "12456";
            fixture.CreateVariableElement(varNumName, VariableElement.VariableDataType.numeric, varNumValue);

            // Add version variable
            string varVerName = "versionVar";
            string varVerValue = "1.2.0.4";
            fixture.CreateVariableElement(varVerName, VariableElement.VariableDataType.version, varVerValue);

            fixture.PrepareLayout();

            // Get string variable
            fixture.GetVariable(VariableElement.VariableDataType.String, varStrName, varStrValue);
            // Compare the value
            bool result = fixture.CompareVariableValue(VariableElement.VariableDataType.String, varStrName, varStrValue);

            Assert.IsTrue(result, string.Format("String value for variable: {0} does not matches. See trace for more detail.", varStrName));

            // Get numeric variable
            fixture.GetVariable(VariableElement.VariableDataType.numeric, varNumName, varNumValue);
            // compare the numeric variable value
            result = fixture.CompareVariableValue(VariableElement.VariableDataType.numeric, varNumName, varNumValue);

            Assert.IsTrue(result, string.Format("Numeric value for variable: {0} does not matches. See trace for more detail.", varNumValue));

            // Get version variable
            fixture.GetVariable(VariableElement.VariableDataType.version, varVerName, varVerValue);
            // compare the version variable value
            result = fixture.CompareVariableValue(VariableElement.VariableDataType.version, varVerName, "281483566645252");

            Assert.IsTrue(result, string.Format("Version value for variable: {0} does not matches. See trace for more detail.", varVerName));
        }

        [TestMethod]
        [Description("To verify variable values are persistent after setup is relaunched")]
        [Timeout(600000)] // 10 minutes
        public void BurnChangePersistentVariable()
        {
            # region Prepare layout containing String, Numeric and Version variable

            // Add string variable
            string varStrName = "stringVar";
            string varStrValue = "test test 123";
            fixture.CreateVariableElement(varStrName, VariableElement.VariableDataType.String, varStrValue);

            // Add numeric variable
            string varNumName = "numericVar";
            string varNumValue = "12456";
            fixture.CreateVariableElement(varNumName, VariableElement.VariableDataType.numeric, varNumValue);

            // Add version variable
            string varVerName = "versionVar";
            string varVerValue = "1.2.0.4";
            fixture.CreateVariableElement(varVerName, VariableElement.VariableDataType.version, varVerValue);

            fixture.PrepareLayout();

            # endregion

            # region Set variable
            // set new value to string variable
            varStrValue = "updated test value 987";
            fixture.SetVariable(VariableElement.VariableDataType.String, varStrName, varStrValue);

            // set new value to numeric variable
            varNumValue = "98765";
            fixture.SetVariable(VariableElement.VariableDataType.numeric, varNumName, varNumValue);

            // set new value to version variable
            varVerValue = "281483566645300";
            fixture.SetVariable(VariableElement.VariableDataType.version, varVerName, varVerValue);

            # endregion

            # region Compare updated variable value

            // Get string variable
            fixture.GetVariable(VariableElement.VariableDataType.String, varStrName, varStrValue);

            // Compare the value
            bool result = fixture.CompareVariableValue(VariableElement.VariableDataType.String, varStrName, varStrValue);

            Assert.IsTrue(result, string.Format("String value for variable: {0} does not matches. See trace for more detail.", varStrName));

            // Get numeric variable
            fixture.GetVariable(VariableElement.VariableDataType.numeric, varNumName, varNumValue);

            // compare the numeric variable value
            result = fixture.CompareVariableValue(VariableElement.VariableDataType.numeric, varNumName, varNumValue);

            Assert.IsTrue(result, string.Format("Numeric value for variable: {0} does not matches. See trace for more detail.", varNumValue));

            // Get version variable
            fixture.GetVariable(VariableElement.VariableDataType.version, varVerName, varVerValue);

            // compare the version variable value
            result = fixture.CompareVariableValue(VariableElement.VariableDataType.version, varVerName, "281483566645300");

            Assert.IsTrue(result, string.Format("Version value for variable: {0} does not matches. See trace for more detail.", varVerName));

            # endregion
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
