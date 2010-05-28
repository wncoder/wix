//-----------------------------------------------------------------------
// <copyright file="BurnDirectorySearchFixture.cs" company="Microsoft">
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
//     - Test fixture for Burn Directory Search feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Searches
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Threading;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches;

    public class BurnDirectorySearchFixture : BurnCommonTestFixture
    {

        public BurnDirectorySearchFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager
                (new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux.TestProxyUx());
        }

        public void CreateDirectorySearchElement(string id, DirectorySearch.DirectorySearchType type,
            string path, string variable)
        {
            this.Layout.ParameterInfo.DirectorySearch = new DirectorySearch();
            this.Layout.ParameterInfo.DirectorySearch.Id = id;
            this.Layout.ParameterInfo.DirectorySearch.Type = type;
            this.Layout.ParameterInfo.DirectorySearch.Path = path;
            this.Layout.ParameterInfo.DirectorySearch.Variable = variable;

            this.Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, true);

            this.CreateLayout();
        }

        private bool IsDirectoryExists()
        {
            return Directory.Exists(this.Layout.ParameterInfo.DirectorySearch.Path);
        }

        public void CreateDirectory(string dirPath)
        {
            if (!Directory.Exists(dirPath))
            {
                DirectoryInfo di = Directory.CreateDirectory(dirPath);
            }
        }

        public bool SearchResultVerification()
        {
            RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);
            bool result = false;


            try
            {
                string message = ConstructBurnCoreMethodCallMessage("GetVariableNumeric", Layout.ParameterInfo.DirectorySearch.Variable);
                SendMessageServerPipe(message);
                Thread.Sleep(2000); // wait for 2 sec so that method call result is written in xml file

                BurnCoreMethodCallResult methodCallResult = GetVariableResult();

                string variableName = methodCallResult.variableName;
                string variableValue = methodCallResult.variableValue;
                string returnValue = methodCallResult.methodReturnValue;

                if (Convert.ToInt32(returnValue) == 0)
                {
                    if (Directory.Exists(Layout.ParameterInfo.DirectorySearch.Path))
                    {
                        if (Convert.ToInt32(variableValue) == 1)
                        {
                            Trace.WriteLine(string.Format("Directory path: {0} found in system as expected",
                                Layout.ParameterInfo.DirectorySearch.Path));
                            return true;
                        }
                        else
                        {
                            Trace.WriteLine(string.Format("Directory path: {0} not found in system",
                               Layout.ParameterInfo.DirectorySearch.Path));
                            return false;
                        }
                    }
                    else
                    {
                        if (Convert.ToInt32(variableValue) == 0)
                        {
                            Trace.WriteLine(string.Format("Directory path: {0} not found in system as expected",
                               Layout.ParameterInfo.DirectorySearch.Path));
                            return true;
                        }
                        else
                        {
                            Trace.WriteLine(string.Format("Directory search returned value indicating directory exists in system even though it was not found in system",
                               Layout.ParameterInfo.DirectorySearch.Path));
                            return false;
                        }
                    }
                }
                else 
                {
                    if (Layout.ParameterInfo.DirectorySearch.Path.Contains("*"))
                    {
                        result = true;
                    }
                    else
                    {
                        Trace.WriteLine(string.Format("Expected return value is 0 whereas actual return value is: {0}", returnValue));
                        result = false;
                    }
                }


            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex.Message);
                result = false;
            }

            return result;
        }
    }
}