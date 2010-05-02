//-----------------------------------------------------------------------
// <copyright file="BurnMsiProductSearchFixture.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Test fixture for Burn Product Search feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Searches
{
    using System;
    using System.Diagnostics;
    using System.Threading;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches;

    public class BurnMsiProductSearchFixture : BurnCommonTestFixture
    {
        public BurnMsiProductSearchFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager
                (new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux.TestProxyUx());
        }

        public void CreateMsiProductSearchElement(string id, MsiProductSearch.MsiProductSearchType type,
            string variable, string productCode, string condition)
        {
            this.Layout.ParameterInfo.MsiProductSearch = new MsiProductSearch();
            this.Layout.ParameterInfo.MsiProductSearch.Id = id;
            this.Layout.ParameterInfo.MsiProductSearch.Type = type;
            this.Layout.ParameterInfo.MsiProductSearch.Variable = variable;
            this.Layout.ParameterInfo.MsiProductSearch.ProductCode = productCode;
            this.Layout.ParameterInfo.MsiProductSearch.Condition = condition;

            this.Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, true);

            this.CreateLayout();           
        }

        public void InstallProducts()
        {
            this.InstallHelloWorldMsi(testMsiPerUserExtCabMsiFile);           
        }

        public void UninstallProducts()
        {
            this.UninstallHelloWorldMsi(testMsiPerUserExtCabMsiFile);
            this.UninstallHelloWorldMsi(testMsiPerMachineFile);
        }

        private string GetMsiProductInfo()
        {
            string msiProductInfo = string.Empty;

            msiProductInfo = MsiUtils.GetProductInfo(Layout.ParameterInfo.MsiProductSearch.ProductCode
                , (MsiUtils.MsiProductSearchType)Layout.ParameterInfo.MsiProductSearch.Type);


            return msiProductInfo;
        }

        public bool SearchResultVerification(string expectedValue)
        {
            RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);
            bool result = false;

            string message = string.Empty;

            switch(Layout.ParameterInfo.MsiProductSearch.Type)
            {
                case MsiProductSearch.MsiProductSearchType.version:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableVersion", Layout.ParameterInfo.MsiProductSearch.Variable);
                    break;
                case MsiProductSearch.MsiProductSearchType.assignment:
                case MsiProductSearch.MsiProductSearchType.language:
                case MsiProductSearch.MsiProductSearchType.state:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableNumeric", Layout.ParameterInfo.MsiProductSearch.Variable);
                    break;
            }

            SendMessageServerPipe(message);
            Thread.Sleep(2000); // wait for 2 sec so that method call result is written in xml file

            BurnCoreMethodCallResult methodCallResult = GetVariableResult();

            string variableName = methodCallResult.variableName;
            string variableValue = methodCallResult.variableValue;
            string returnValue = methodCallResult.methodReturnValue;

            if (Convert.ToInt32(returnValue) == 0)
            {
                //string msiProperty = GetMsiProductInfo();
                if (expectedValue == variableValue)
                {
                    result = true;
                    Trace.WriteLine(string.Format("Expected value matches with Actual value. Value: {0}", expectedValue));
                }
                else
                {
                    result = false;
                    Trace.WriteLine(string.Format("Expected value does not matches with actual value found on the system for {0}. Actual value: {1} Expected Value: {2}"
                        , Layout.ParameterInfo.MsiProductSearch.Type.ToString(), variableValue, expectedValue));
                }
            }
            else
            {
                Trace.WriteLine(string.Format("Expected return value is 0 whereas actual return value is: {0}", returnValue));
                result = false;
            }

            return result;

        }
       
    }
}
