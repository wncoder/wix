//-----------------------------------------------------------------------
// <copyright file="BurnMsiComponentSearchFixture.cs" company="Microsoft">
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
//     - Test fixture for Burn Component Search feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Searches
{
    using System;
    using System.Diagnostics;
    using System.Threading;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Searches;

    public class BurnMsiComponentSearchFixture : BurnCommonTestFixture
    {
        public BurnMsiComponentSearchFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager
                (new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());
        }

        public void CreateMsiComponentSearchElement(string id, ComponentSearchElement.ComponentSearchResultType type,
            string variable, string productCode, string compId, string condition)
        {
            ComponentSearchElement componentSearch = new ComponentSearchElement();
            componentSearch.Id = id;
            componentSearch.Result = type;
            componentSearch.Variable = variable;
            componentSearch.ProductCode = productCode;
            componentSearch.Guid = compId;
            componentSearch.Condition = condition;

            this.Layout.Wix.Bundle.ComponentSearches.Add(componentSearch);

            this.Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, true);

            this.Layout.BuildBundle(false);
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

        private string GetMsiComponentInfo(string componentId, string productCode, ComponentSearchElement.ComponentSearchResultType resultType)
        {
            string msiComponent = string.Empty;
            if (string.IsNullOrEmpty(productCode))
            {
                msiComponent = MsiUtils.GetComponentInfo(componentId, string.Empty
                    , (MsiUtils.MsiComponentSearchType)resultType);
            }
            else
            {
                msiComponent = MsiUtils.GetComponentInfo(componentId
                    , productCode, (MsiUtils.MsiComponentSearchType)resultType);
            }

            return msiComponent;
        }

        public bool SearchResultVerification(string expectedValue, ComponentSearchElement.ComponentSearchResultType resultType, string variable)
        {
            RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);
            bool result = false;

            string message = string.Empty;

            switch (resultType)
            {
                case ComponentSearchElement.ComponentSearchResultType.Directory:
                case ComponentSearchElement.ComponentSearchResultType.KeyPath:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableString", variable, 1024);
                    break;
                case ComponentSearchElement.ComponentSearchResultType.State:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableNumeric", variable);
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
                        , resultType.ToString(), variableValue, expectedValue));
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
