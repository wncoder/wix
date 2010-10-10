//-----------------------------------------------------------------------
// <copyright file="BurnConditionFixture.cs" company="Microsoft">
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
//     - Test Fixture for Burn Condition feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Condition
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Threading;
    using System.Xml;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Variable;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;

    public class BurnConditionFixture : BurnCommonTestFixture
    {
        public struct EvalConditionMethodCallResult
        {
            public string evalResult;           
            public string methodReturnValue;
        }

        public BurnConditionFixture()
            : base()
        {
            this.Layout = new LayoutManager(new TestUX());
        }

        public void AddVariableElement(string variableId, string variableValue, VariableElement.VariableDataType type)
        {
            Layout.AddVariable(variableId, variableValue, type);
        }  

        public bool EvaluateCondition(string conditionStatement, string expectedReturnVal, bool expectedEvalResult)
        {
            string prodCode = MsiUtils.GetMSIProductCode(testMsiPerMachineFile);

            if (MsiUtils.IsProductInstalled(prodCode))
            {
                MsiUtils.RemoveMSI(testMsiPerMachineFile);
            }

            this.Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, true);
            this.Layout.BuildBundle(false);

            bool result = false;

            RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);

            string message = ConstructEvalulateConditionCallMessage("EvaluateCondition", conditionStatement);
            SendMessageServerPipe(message);
            Thread.Sleep(2000); // wait for 2 sec so that method call result is written in xml file

            EvalConditionMethodCallResult evalResult = GetEvalResult();

            if (evalResult.methodReturnValue == expectedReturnVal)
            {
                Trace.WriteLine(string.Format("Expected and Actual return value matches. Return value: {0}"
                    , evalResult.methodReturnValue));

                if (evalResult.evalResult.ToLower() == expectedEvalResult.ToString().ToLower())
                {
                    result = true;
                    Trace.WriteLine(string.Format("Expected and Actual eval result matches. Eval result:{0}", evalResult.evalResult));                    
                }
                else
                {
                    result = false;
                    Trace.WriteLine(string.Format("Expected and Actual Eval result does not matches. Expected: {0} and Actual:{1}"
                        , expectedEvalResult.ToString(), evalResult.evalResult));
                }
            }
            else
            {
                result = false;
                Trace.WriteLine(string.Format("Expected and Actual return value does not matches. Expected return value: {0} where Actual return value:{1}"
                    , expectedReturnVal, evalResult.methodReturnValue));
            }

            return result;
        }

        private EvalConditionMethodCallResult GetEvalResult()
        {
            EvalConditionMethodCallResult result = new EvalConditionMethodCallResult();
            string resultFilePath = Environment.ExpandEnvironmentVariables(@"%temp%\BurnEvalResultResult.xml");

            if (File.Exists(resultFilePath))
            {
                XmlDocument xmldoc = new XmlDocument();
                xmldoc.Load(resultFilePath);

                XmlNode node = xmldoc.SelectSingleNode("./BurnCoreMethodResult/Method");
                result.evalResult = node.Attributes["Evalresult"].Value;               
                result.methodReturnValue = node.Attributes["returnvalue"].Value;
            }
            else
            {
                Trace.WriteLine(string.Format("Result file: {0} not found.", resultFilePath));
            }

            return result;
        }
    }
}
