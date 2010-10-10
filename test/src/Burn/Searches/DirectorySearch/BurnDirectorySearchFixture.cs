//-----------------------------------------------------------------------
// <copyright file="BurnDirectorySearchFixture.cs" company="Microsoft">
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
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Searches;

    public class BurnDirectorySearchFixture : BurnCommonTestFixture
    {

        public BurnDirectorySearchFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager
                (new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());
        }

        public void CreateDirectorySearchElement(string id, DirectorySearchElement.DirectorySearchResultType type,
            string path, string variable)
        {
            DirectorySearchElement dirSearch = new DirectorySearchElement();
            dirSearch.Id = id;
            dirSearch.Result = type;
            dirSearch.Path = path;
            dirSearch.Variable = variable;

            this.Layout.Wix.Bundle.DirectorySearches.Add(dirSearch);
            this.Layout.AddExe(testExeFile, true);

            this.Layout.BuildBundle(false);
        }

        private bool IsDirectoryExists(string path)
        {
            return Directory.Exists(path);
        }

        public void CreateDirectory(string dirPath)
        {
            if (!Directory.Exists(dirPath))
            {
                DirectoryInfo di = Directory.CreateDirectory(dirPath);
            }
        }

        public bool SearchResultVerification(string path, string variable)
        {
            RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);
            bool result = false;


            try
            {
                string message = ConstructBurnCoreMethodCallMessage("GetVariableNumeric", variable);
                SendMessageServerPipe(message);
                Thread.Sleep(2000); // wait for 2 sec so that method call result is written in xml file

                BurnCoreMethodCallResult methodCallResult = GetVariableResult();

                string variableName = methodCallResult.variableName;
                string variableValue = methodCallResult.variableValue;
                string returnValue = methodCallResult.methodReturnValue;

                if (Convert.ToInt32(returnValue) == 0)
                {
                    if (Directory.Exists(path))
                    {
                        if (Convert.ToInt32(variableValue) == 1)
                        {
                            Trace.WriteLine(string.Format("Directory path: {0} found in system as expected",
                                path));
                            return true;
                        }
                        else
                        {
                            Trace.WriteLine(string.Format("Directory path: {0} not found in system",
                               path));
                            return false;
                        }
                    }
                    else
                    {
                        if (Convert.ToInt32(variableValue) == 0)
                        {
                            Trace.WriteLine(string.Format("Directory path: {0} not found in system as expected",
                               path));
                            return true;
                        }
                        else
                        {
                            Trace.WriteLine(string.Format("Directory search returned value indicating directory exists in system even though it was not found in system",
                               path));
                            return false;
                        }
                    }
                }
                else 
                {
                    if (path.Contains("*"))
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