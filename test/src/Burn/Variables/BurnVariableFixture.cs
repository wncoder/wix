//-----------------------------------------------------------------------
// <copyright file="BurnVariableFixture.cs" company="Microsoft">
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
//     - Test fixture for Burn Variables feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Variables
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Threading;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnVariable;

    public class BurnVariableFixture : BurnCommonTestFixture
    {
        private BurnCoreMethodCallResult m_methodCallResult;
        
        private string[] mBuiltInVariableName = new string[] { 
            "AdminToolsFolder", "string",
            "AppDataFolder", "string",
            "CommonAppDataFolder", "string",
            "CommonFiles64Folder", "string",
            "CommonFilesFolder", "string",
            "DesktopFolder", "string",
            "FavoritesFolder", "string",
            "FontsFolder", "string",
            "LocalAppDataFolder", "string",
            "MyPicturesFolder", "string",
            "NTProductType", "numeric",
            "NTSuiteBackOffice", "numeric",
            "NTSuiteDataCenter", "numeric",
            "NTSuiteEnterprise", "numeric",
            "NTSuitePersonal", "numeric",
            "NTSuiteSmallBusiness", "numeric",
            "NTSuiteSmallBusinessRestricted", "numeric",
            "NTSuiteWebServer", "numeric",
            "PersonalFolder", "string",
            "ProgramFiles64Folder", "string",
            "ProgramFilesFolder", "string",
            "ProgramMenuFolder", "string",
            "SendToFolder", "string",
            "StartMenuFolder", "string",
            "StartupFolder", "string",
            "SystemFolder", "string",
            "TempFolder", "string",
            "TemplateFolder", "string",
            "VersionMsi", "version",
            "VersionNT", "version",
            "VersionNT64", "version",
            "WindowsFolder", "string",
            "WindowsVolume", "string"
        };

        public BurnVariableFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager
                (new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux.TestProxyUx());
        }

        public void CreateVariableElement(string id, VariableDataType type, string value)
        {
            Layout.ParameterInfo.Variable = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnVariable.Variable();

            this.Layout.ParameterInfo.Variable.Id = id;
            this.Layout.ParameterInfo.Variable.Type = type;
            this.Layout.ParameterInfo.Variable.Value = value;

            PrepareLayout();
        }

        public void PrepareLayout()
        {
            this.Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, true);

            this.CreateLayout();
        }

        private void SetVariable()
        {
            if (MsiUtils.IsProductInstalled(testMsiPerMachineFile))
            {
                MsiUtils.RemoveMSI(testMsiPerMachineFile);
            }

            RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);
            string message = string.Empty;

            switch(Layout.ParameterInfo.Variable.Type)
            {
                case VariableDataType.numeric:
                    message = ConstructBurnCoreMethodCallMessage("SetVariableNumeric", Layout.ParameterInfo.Variable.Id
                        , Layout.ParameterInfo.Variable.Value);
                    break;
                case VariableDataType.String:
                    message = ConstructBurnCoreMethodCallMessage("SetVariableString", Layout.ParameterInfo.Variable.Id
                        , Layout.ParameterInfo.Variable.Value);
                    break;
                case VariableDataType.version:
                    message = ConstructBurnCoreMethodCallMessage("SetVariableVersion", Layout.ParameterInfo.Variable.Id
                        , Layout.ParameterInfo.Variable.Value);
                    break;
            }

            SendMessageServerPipe(message);
            Thread.Sleep(2000);
        }

        private void GetVariable()
        {
            if (MsiUtils.IsProductInstalled(testMsiPerMachineFile))
            {
                MsiUtils.RemoveMSI(testMsiPerMachineFile);
            }

            RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);
            string message = string.Empty;

            switch(Layout.ParameterInfo.Variable.Type)
            {
                case VariableDataType.numeric:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableNumeric", Layout.ParameterInfo.Variable.Id);
                    break;
                case VariableDataType.String:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableString", Layout.ParameterInfo.Variable.Id
                        , Layout.ParameterInfo.Variable.Value.Length + 1);
                    break;
                case VariableDataType.version:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableVersion", Layout.ParameterInfo.Variable.Id);
                    break;
            }

            SendMessageServerPipe(message);
            Thread.Sleep(2000); // wait for 2 sec so that method call result is written in xml file

            m_methodCallResult = GetVariableResult();            
        }

        public bool CompareVariableValue()
        {
            SetVariable();
            GetVariable();

            bool result = false;

            string variableName = m_methodCallResult.variableName;
            string variableValue = m_methodCallResult.variableValue;
            string returnValue = m_methodCallResult.methodReturnValue;

            if (Convert.ToInt32(returnValue) == 0)
            {
                if (variableValue == Layout.ParameterInfo.Variable.Value)
                {
                    Trace.WriteLine(string.Format("Set and Get variable value does matches. Expected value: {0} and Actual value: {1}"
                        , Layout.ParameterInfo.Variable.Value, variableValue));
                    return true;
                }
                else
                {
                    Trace.WriteLine(string.Format("Set and Get variable value does not matches. Expected value: {0} but Actual value: {1}"
                       , Layout.ParameterInfo.Variable.Value, variableValue));
                    return false;
                }
            }
            else
            {
                Trace.WriteLine(string.Format("Expected return value is 0 whereas actual return value is: {0}", returnValue));
                result = false;
            }

            return result;
        }

        public bool VerifyBuiltInVariables()
        {
            bool result = true;
            for (int i = 0; i < mBuiltInVariableName.Count(); i+=2)
            {
                string expectedValue = GetBuiltInVariableExpectedValue(mBuiltInVariableName[i]);
                string variableDataType = mBuiltInVariableName[i+1];
                string message = string.Empty;
                switch(variableDataType)
                {
                    case "string":
                        message = ConstructBurnCoreMethodCallMessage("GetVariableString", mBuiltInVariableName[i], 10000);
                        break;
                    case "numeric":
                        message = ConstructBurnCoreMethodCallMessage("GetVariableNumeric", mBuiltInVariableName[i]);
                        break;
                    case "version":
                        message = ConstructBurnCoreMethodCallMessage("GetVariableVersion", mBuiltInVariableName[i]);
                        break;
                }

                RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);

                SendMessageServerPipe(message);
                Thread.Sleep(2000); // wait for 2 sec so that method call result is written in xml file

                m_methodCallResult = GetVariableResult();

                string variableName = m_methodCallResult.variableName;
                string variableValue = m_methodCallResult.variableValue;
                string returnValue = m_methodCallResult.methodReturnValue;

                if (variableValue != expectedValue)
                {
                    result = false;
                    Trace.WriteLine(string.Format("Built variable: {0} value is INCORRECT, Expected value: {1} -- Actual value: {2}"
                        , mBuiltInVariableName[i], expectedValue, variableValue));

                }
                else
                {
                    Trace.WriteLine(string.Format("Built variable: {0} value is correct, Expected value: {1} -- Actual value: {2}"
                        , mBuiltInVariableName[i], expectedValue, variableValue));
                }
            }

            return result;
        }

        private string GetBuiltInVariableExpectedValue(string builtInVariableName)
        {
            string value = string.Empty;
            switch(builtInVariableName)
            {
                case "AdminToolsFolder":
                    // TODO:
                    break;
                case "AppDataFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
                    break;
                case "CommonAppDataFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData);
                    break;
                case "CommonFiles64Folder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.CommonProgramFiles);
                    break;
                case "CommonFilesFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.CommonProgramFiles);
                    break;
                case "DesktopFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory);
                    break;
                case "FavoritesFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.Favorites);
                    break;
                case "FontsFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.System) + "\\Fonts";
                    break;
                case "LocalAppDataFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
                    break;
                case "MyPicturesFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.MyPictures);
                    break;
                case "NTProductType":
                    // TODO:
                    break;
                case "NTSuiteBackOffice":
                    // TODO:
                    break;
                case "NTSuiteDataCenter":
                    // TODO:
                    break;
                case "NTSuiteEnterprise":
                    // TODO:
                    break;
                case "NTSuitePersonal":
                    // TODO:
                    break;
                case "NTSuiteSmallBusiness":
                    // TODO:
                    break;
                case "NTSuiteSmallBusinessRestricted":
                    // TODO:
                    break;
                case "NTSuiteWebServer":
                    // TODO:
                    break;
                case "PersonalFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
                    break;
                case "ProgramFiles64Folder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles);
                    break;
                case "ProgramFilesFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles);
                    break;
                case "ProgramMenuFolder":
                    value = value = Environment.GetFolderPath(Environment.SpecialFolder.Programs);
                    break;
                case "SendToFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.SendTo);
                    break;
                case "StartMenuFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.StartMenu);
                    break;
                case "StartupFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.Startup);
                    break;
                case "SystemFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.System);
                    break;
                case "TempFolder":
                    value = Path.GetTempPath();
                    break;
                case "TemplateFolder":
                    value = Environment.GetFolderPath(Environment.SpecialFolder.Templates);
                    break;
                case "VersionMsi":
                    // TODO:
                    break;
                case "VersionNT":
                    // TODO:
                    break;
                case "VersionNT64":
                    // TODO:
                    break;
                case "WindowsFolder":
                    // TODO:
                    break;
                case "WindowsVolume":
                    // TODO:
                    break;
            }

            return value;
        }
    }
}
