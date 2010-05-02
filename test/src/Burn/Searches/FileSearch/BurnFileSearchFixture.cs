//-----------------------------------------------------------------------
// <copyright file="BurnFileSearchFixture.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Test fixture for Burn File Search feature.
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
    
    public class BurnFileSearchFixture : BurnCommonTestFixture
    {
        public BurnFileSearchFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager
                (new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux.TestProxyUx());
        }

        public void CreateFileSearchElement(string id, FileSearch.FileSearchType type, string path, string variable)
       {
           this.Layout.ParameterInfo.FileSearch = new FileSearch();
            this.Layout.ParameterInfo.FileSearch.Id = id;
            this.Layout.ParameterInfo.FileSearch.Type = type;
            this.Layout.ParameterInfo.FileSearch.Path = path;
            this.Layout.ParameterInfo.FileSearch.Variable = variable;

            this.Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, true);

            this.CreateLayout();
       }

        public void CreateTextFile(string filePath)
        {
            if ( ! File.Exists(filePath) )
            {
                using (System.IO.StreamWriter file = new StreamWriter
                    (filePath, true))
                {
                    file.WriteLine("Sample sample");
                }
            }
        }

        public string GetFileVersion(string filePath)
        {
            FileVersionInfo fvi = FileVersionInfo.GetVersionInfo(filePath);
            return fvi.ProductVersion;
        }

        public bool SearchResultVerification()
        {
            RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);
            bool result = false;

            string message = string.Empty;

            switch (Layout.ParameterInfo.FileSearch.Type)
            {
                case FileSearch.FileSearchType.exists:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableNumeric", Layout.ParameterInfo.FileSearch.Variable);
                    break;
                case FileSearch.FileSearchType.version:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableVersion", Layout.ParameterInfo.FileSearch.Variable);
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
                if (Layout.ParameterInfo.FileSearch.Type == FileSearch.FileSearchType.exists)
                {
                    if (File.Exists(Layout.ParameterInfo.FileSearch.Path))
                    {
                        if (Convert.ToInt32(variableValue) == 1)
                        {
                            Trace.WriteLine(string.Format("File path: {0} found in system as expected",
                                Layout.ParameterInfo.FileSearch.Path));
                            return true;
                        }
                        else
                        {
                            Trace.WriteLine(string.Format("File path: {0} not found in system",
                               Layout.ParameterInfo.FileSearch.Path));
                            return false;
                        }

                    }
                    else
                    {
                        if (Convert.ToInt32(variableValue) == 0)
                        {
                            Trace.WriteLine(string.Format("File path: {0} not found in system as expected",
                               Layout.ParameterInfo.FileSearch.Path));
                            return true;
                        }
                        else
                        {
                            Trace.WriteLine(string.Format("File search returned value indicating file exists in system even though it was not found in system",
                               Layout.ParameterInfo.FileSearch.Path));
                            return false;
                        }

                    }
                }
                else if (Layout.ParameterInfo.FileSearch.Type == FileSearch.FileSearchType.version)
                {
                    if (File.Exists(Layout.ParameterInfo.FileSearch.Path))
                    {
                        string fileVersion = GetFileVersion(Layout.ParameterInfo.FileSearch.Path);

                        long value = Convert.ToInt64(variableValue);

                        int major = (int)(value & (0xffff << 48) >> 48);
                        int minor = (int)(value & (0xffff << 32) >> 32);
                        int build = (int)(value & (0xffff << 16) >> 16);
                        int revision = (int)(value & 0xffff);

                       

                    }
                    else
                    {
                        Trace.WriteLine(string.Format("File path: {0} not found in system. For version verification make sure file exists in target system"
                            , Layout.ParameterInfo.FileSearch.Path));

                    }
                }
            }
            else
            {
                Trace.WriteLine(string.Format("Expected return value is 0 whereas actual return value is: {0}", returnValue));
                result = false;
            }

            return result;
        }

        public void DeleteFile(string filePath)
        {
            File.Delete(filePath);
        }
    }
}
