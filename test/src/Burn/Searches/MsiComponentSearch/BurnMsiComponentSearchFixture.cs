//-----------------------------------------------------------------------
// <copyright file="BurnMsiComponentSearchFixture.cs" company="Microsoft">
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
//     - Test fixture for Burn Component Search feature.
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

    public class BurnMsiComponentSearchFixture : BurnCommonTestFixture
    {
        public BurnMsiComponentSearchFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager
                (new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux.TestProxyUx());
        }

        public void CreateMsiComponentSearchElement(string id, MsiComponentSearch.MsiComponentSearchType type,
            string variable, string productCode, string compId, string condition)
        {
            this.Layout.ParameterInfo.MsiComponentSearch = new MsiComponentSearch();
            this.Layout.ParameterInfo.MsiComponentSearch.Id = id;
            this.Layout.ParameterInfo.MsiComponentSearch.Type = type;
            this.Layout.ParameterInfo.MsiComponentSearch.Variable = variable;
            this.Layout.ParameterInfo.MsiComponentSearch.ProductCode = productCode;
            this.Layout.ParameterInfo.MsiComponentSearch.ComponentId = compId;
            this.Layout.ParameterInfo.MsiComponentSearch.Condition = condition;

            this.Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, true);

            this.CreateLayout();
        }

        public void InstallProducts()
        {
            this.InstallHelloWorldMsi(testMsiPerUserExtCabMsiFile);
            this.InstallHelloWorldMsi(testMsiPerUserFile);
            this.InstallHelloWorldMsi(testMsiPerMachineFile);
        }

        public void UninstallProducts()
        {
            this.UninstallHelloWorldMsi(testMsiPerUserExtCabMsiFile);
            this.UninstallHelloWorldMsi(testMsiPerUserFile);
            this.UninstallHelloWorldMsi(testMsiPerMachineFile);
        }

        private string GetMsiComponentInfo()
        {
            string msiComponent = string.Empty;
            if (string.IsNullOrEmpty(Layout.ParameterInfo.MsiComponentSearch.ProductCode))
            {
                msiComponent = MsiUtils.GetComponentInfo(Layout.ParameterInfo.MsiComponentSearch.ComponentId, string.Empty
                    , (MsiUtils.MsiComponentSearchType)Layout.ParameterInfo.MsiComponentSearch.Type);
            }
            else
            {
                msiComponent = MsiUtils.GetComponentInfo(Layout.ParameterInfo.MsiComponentSearch.ComponentId
                    , Layout.ParameterInfo.MsiComponentSearch.ProductCode, (MsiUtils.MsiComponentSearchType)Layout.ParameterInfo.MsiComponentSearch.Type);
            }

            return msiComponent;
        }

        public bool SearchResultVerification(string expectedValue)
        {
            RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);
            bool result = false;

            string message = string.Empty;

            switch(Layout.ParameterInfo.MsiComponentSearch.Type)
            {
                case MsiComponentSearch.MsiComponentSearchType.directory:
                case MsiComponentSearch.MsiComponentSearchType.keyPath:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableString", Layout.ParameterInfo.MsiComponentSearch.Variable, 1024);
                    break;
                case MsiComponentSearch.MsiComponentSearchType.state:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableNumeric", Layout.ParameterInfo.MsiComponentSearch.Variable);
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
                //string msiComponent = GetMsiComponentInfo();
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
