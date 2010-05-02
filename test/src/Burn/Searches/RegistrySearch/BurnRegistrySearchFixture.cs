//-----------------------------------------------------------------------
// <copyright file="BurnRegistrySearchFixture.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnVariable;

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
                (new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux.TestProxyUx());
        }

        public void CreateRegistrySearchElement(string id, string subKey, RegistrySearch.RegRoot root,
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.Type type, string value
            , string variable, string varType, RegistrySearch.YesNoType expendEnironmentVar
            , string registryKey, string registryValue)
        {
            this.Layout.ParameterInfo.RegistrySearch = new RegistrySearch();
            this.Layout.ParameterInfo.RegistrySearch.Id = id;
            this.Layout.ParameterInfo.RegistrySearch.Key = subKey;
            this.Layout.ParameterInfo.RegistrySearch.Root = root;
            this.Layout.ParameterInfo.RegistrySearch.Type = type;           
            this.Layout.ParameterInfo.RegistrySearch.Value = registryKey;
            this.Layout.ParameterInfo.RegistrySearch.Variable = variable;

            if (!string.IsNullOrEmpty(varType))
            {
                VariableDataType variableType = (VariableDataType)Enum.Parse (typeof(VariableDataType), varType, true);
                this.Layout.ParameterInfo.RegistrySearch.VariableType = variableType;
            }

            this.Layout.ParameterInfo.RegistrySearch.ExpandEnvironment = expendEnironmentVar;

            this.Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, true);

            this.CreateLayout();

            if (!subKey.Contains("DoNotExists"))
            {
                CreateRegistryEntry(root, value, subKey, registryKey, registryValue);
            }

            m_registryValue = registryValue;
        }

        private void CreateRegistryEntry(RegistrySearch.RegRoot root, string value
            , string subKey, string registryKey, string registryValue)
        {
            m_subKey = subKey;
            switch (Layout.ParameterInfo.RegistrySearch.Root)
            {
                case RegistrySearch.RegRoot.HKCU:
                    m_regKey = Registry.CurrentUser.CreateSubKey(subKey);
                    break;
                case RegistrySearch.RegRoot.HKLM:
                    m_regKey = Registry.LocalMachine.CreateSubKey(subKey);
                    break;
                case RegistrySearch.RegRoot.HKU:
                    m_regKey = Registry.Users.CreateSubKey(subKey);
                    break;
                case RegistrySearch.RegRoot.HKCR:
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

        public bool SearchResultVerification()
        {
            RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);
            bool result = false;

            string message = string.Empty;

            switch (Layout.ParameterInfo.RegistrySearch.VariableType)
            {
                case VariableDataType.numeric:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableNumeric", Layout.ParameterInfo.RegistrySearch.Variable);
                    break;
                case VariableDataType.String:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableString", Layout.ParameterInfo.RegistrySearch.Variable, 1024);
                    break;
                case VariableDataType.version:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableVersion", Layout.ParameterInfo.RegistrySearch.Variable);
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
                if (Layout.ParameterInfo.RegistrySearch.Type == Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.Type.exists)
                {
                    if (Layout.ParameterInfo.RegistrySearch.Root == RegistrySearch.RegRoot.HKLM ||
                        Layout.ParameterInfo.RegistrySearch.Root == RegistrySearch.RegRoot.HKCU)
                    {
                        if (variableValue == "1")
                        {
                            result = true;
                            Trace.WriteLine(string.Format("Registry key: {0} as expected was found on the system"
                                , Layout.ParameterInfo.RegistrySearch.Key));
                        }
                        else
                        {
                            if (Layout.ParameterInfo.RegistrySearch.Key.Contains("DoNotExists"))
                            {
                                result = true;
                            }
                            else
                            {
                                result = false;
                                Trace.WriteLine(string.Format("Registry key: {0} was not found on the system"
                                    , Layout.ParameterInfo.RegistrySearch.Key));
                            }
                        }
                    }                   
                }

                if (Layout.ParameterInfo.RegistrySearch.Type == Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.Type.value)
                {
                    string expectedValue = Environment.ExpandEnvironmentVariables(m_registryValue);

                    if (expectedValue == variableValue)
                    {
                        result = true;
                        Trace.WriteLine(string.Format("Registry search for key: {0} PASSED. Expected value matches with actual value: {1}"
                            , Layout.ParameterInfo.RegistrySearch.Key, expectedValue));

                    }
                    else
                    {
                        result = false;
                        Trace.WriteLine(string.Format("Registry search for key: {0} FAILED. Expected value {1} whereas actual value: {2}"
                            , Layout.ParameterInfo.RegistrySearch.Key, expectedValue, variableValue));

                    }
                }
            }
            else
            {
                if (Layout.ParameterInfo.RegistrySearch.Root != RegistrySearch.RegRoot.HKLM ||
                        Layout.ParameterInfo.RegistrySearch.Root != RegistrySearch.RegRoot.HKCU)
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
            DeleteKey();

            return result;
        }

        private void DeleteKey()
        {
            if (!string.IsNullOrEmpty(m_subKey))
            {
                switch (Layout.ParameterInfo.RegistrySearch.Root)
                {
                    case RegistrySearch.RegRoot.HKCU:
                        Registry.CurrentUser.DeleteSubKey(m_subKey);
                        break;
                    case RegistrySearch.RegRoot.HKLM:
                        Registry.LocalMachine.DeleteSubKey(m_subKey);
                        break;
                    case RegistrySearch.RegRoot.HKU:
                        Registry.Users.DeleteSubKey(m_subKey);
                        break;
                    case RegistrySearch.RegRoot.HKCR:
                        Registry.ClassesRoot.DeleteSubKey(m_subKey);
                        break;
                }
            }
        }
    }
}
