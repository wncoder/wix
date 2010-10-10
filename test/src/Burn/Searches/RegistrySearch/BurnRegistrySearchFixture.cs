//-----------------------------------------------------------------------
// <copyright file="BurnRegistrySearchFixture.cs" company="Microsoft">
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
//     - Test fixture for Burn Registry Search feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Searches
{
    using System;
    using System.Diagnostics;
    using System.Threading;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.Win32;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Searches;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Variable;

    public class BurnRegistrySearchFixture : BurnCommonTestFixture
    {
        private RegistryKey m_regKey;
        private string m_subKey;
        private string m_registryValue;

        public enum RegDataType
        {
            String,
            DWord,
            QWord,
            MultiString,
            ExpandString
        }

        public BurnRegistrySearchFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager
                (new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());
        }

        public void CreateRegistrySearchElement(string id, string subKey, RegistrySearchElement.RegRoot root,
            RegistrySearchElement.RegistrySearchResultType type, string value
            , string variable, RegistrySearchElement.YesNoType expendEnironmentVar
            , string registryKey, string registryValue)
        {
            RegistrySearchElement regSearchElement = new RegistrySearchElement();
            regSearchElement.Id = id;
            regSearchElement.Key = subKey;
            regSearchElement.Root = root;
            regSearchElement.Result = type;
            regSearchElement.Value = registryKey;
            regSearchElement.Variable = variable;
            regSearchElement.ExpandEnvironmentVariables = expendEnironmentVar;

            this.Layout.Wix.Bundle.RegistrySearches.Add(regSearchElement);

            this.Layout.AddExe(testExeFile, true);

            this.Layout.BuildBundle(false);

            if (!subKey.Contains("DoNotExists"))
            {
                CreateRegistryEntry(root, value, subKey, registryKey, registryValue);
            }

            m_registryValue = registryValue;
        }

        private void CreateRegistryEntry(RegistrySearchElement.RegRoot root, string value
            , string subKey, string registryKey, string registryValue)
        {
            m_subKey = subKey;
            switch (root)
            {
                case RegistrySearchElement.RegRoot.HKCU:
                    m_regKey = Registry.CurrentUser.CreateSubKey(subKey);
                    break;
                case RegistrySearchElement.RegRoot.HKLM:
                    m_regKey = Registry.LocalMachine.CreateSubKey(subKey);
                    break;
                case RegistrySearchElement.RegRoot.HKU:
                    m_regKey = Registry.Users.CreateSubKey(subKey);
                    break;
                case RegistrySearchElement.RegRoot.HKCR:
                    m_regKey = Registry.ClassesRoot.CreateSubKey(subKey);
                    break;
            }

            if (!string.IsNullOrEmpty(registryKey))
            {
                BurnRegistrySearchFixture.RegDataType val = (BurnRegistrySearchFixture.RegDataType)Enum.Parse
                    (typeof(BurnRegistrySearchFixture.RegDataType), value, true);

                switch (val)
                {
                    case RegDataType.DWord:
                        m_regKey.SetValue(registryKey, registryValue, RegistryValueKind.DWord);
                        break;
                    case RegDataType.QWord:
                        m_regKey.SetValue(registryKey, registryValue, RegistryValueKind.QWord);
                        break;
                    case RegDataType.String:
                        m_regKey.SetValue(registryKey, registryValue, RegistryValueKind.String);
                        break;
                    case RegDataType.ExpandString:
                        m_regKey.SetValue(registryKey, registryValue, RegistryValueKind.ExpandString);
                        break;
                    case RegDataType.MultiString:
                        m_regKey.SetValue(registryKey, registryValue.Split(new string[] {"."}, StringSplitOptions.RemoveEmptyEntries), RegistryValueKind.MultiString);
                        break;
                }
            }
        }

        public bool SearchResultVerification(string varType, RegistrySearchElement.RegRoot root, string variable, string key, string subKey
            , RegistrySearchElement.RegistrySearchResultType resultType)
        {
            RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);
            bool result = false;

            string message = string.Empty;
            VariableElement.VariableDataType variableType = VariableElement.VariableDataType.String;

            if (!string.IsNullOrEmpty(varType))
            {
                variableType = (VariableElement.VariableDataType)Enum.Parse
                    (typeof(VariableElement.VariableDataType), varType, true);
            }

            string expectedValue = Environment.ExpandEnvironmentVariables(m_registryValue);

            switch (variableType)
            {
                case VariableElement.VariableDataType.numeric:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableNumeric", variable);
                    break;
                case VariableElement.VariableDataType.String:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableString", variable, expectedValue.Length + 1);
                    break;
                case VariableElement.VariableDataType.version:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableVersion", variable);
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
                if (resultType == RegistrySearchElement.RegistrySearchResultType.Exists)
                {
                    if (root == RegistrySearchElement.RegRoot.HKLM || root == RegistrySearchElement.RegRoot.HKCU)
                    {
                        if (variableValue == "1")
                        {
                            result = true;
                            Trace.WriteLine(string.Format("Registry key: {0} as expected was found on the system"
                                , key));
                        }
                        else
                        {
                            if ((key.Contains("DoNotExists") || subKey.Contains("DoNotExists")) && variableValue == "0")
                            {
                                result = true;
                            }
                            else
                            {
                                result = false;
                                Trace.WriteLine(string.Format("Registry key: {0} was not found on the system"
                                    , key));
                            }
                        }
                    }
                }

                if (resultType == RegistrySearchElement.RegistrySearchResultType.Value)
                {
                    if (expectedValue == variableValue)
                    {
                        result = true;
                        Trace.WriteLine(string.Format("Registry search for key: {0} PASSED. Expected value matches with actual value: {1}"
                            , key, expectedValue));

                    }
                    else
                    {
                        result = false;
                        Trace.WriteLine(string.Format("Registry search for key: {0} FAILED. Expected value {1} whereas actual value: {2}"
                            , key, expectedValue, variableValue));

                    }
                }
            }
            else
            {
                if (root != RegistrySearchElement.RegRoot.HKLM || root != RegistrySearchElement.RegRoot.HKCU)
                {
                    result = true;
                    Trace.WriteLine("Burn engine current supports registry search for keys under HKLM and HKCU only");
                }
                else
                {
                    Trace.WriteLine(string.Format("Expected return value is 0 whereas actual return value is: {0}", returnValue));
                    result = false;
                }
            }

            // Delete the key
            DeleteKey(root);

            return result;
        }

        private void DeleteKey(RegistrySearchElement.RegRoot root)
        {
            if (!string.IsNullOrEmpty(m_subKey))
            {
                switch (root)
                {
                    case RegistrySearchElement.RegRoot.HKCU:
                        Registry.CurrentUser.DeleteSubKey(m_subKey);
                        break;
                    case RegistrySearchElement.RegRoot.HKLM:
                        Registry.LocalMachine.DeleteSubKey(m_subKey);
                        break;
                    case RegistrySearchElement.RegRoot.HKU:
                        Registry.Users.DeleteSubKey(m_subKey);
                        break;
                    case RegistrySearchElement.RegRoot.HKCR:
                        Registry.ClassesRoot.DeleteSubKey(m_subKey);
                        break;
                }
            }
        }
    }
}
