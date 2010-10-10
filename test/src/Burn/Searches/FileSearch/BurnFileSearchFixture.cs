//-----------------------------------------------------------------------
// <copyright file="BurnFileSearchFixture.cs" company="Microsoft">
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
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Searches;

    public class BurnFileSearchFixture : BurnCommonTestFixture
    {
        public BurnFileSearchFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager
                (new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());
        }

        public void CreateFileSearchElement(string id, FileSearchElement.FileSearchResultType type, string path, string variable)
       {
           FileSearchElement fileSearch = new FileSearchElement();
           fileSearch.Id = id;
           fileSearch.Result = type;
           fileSearch.Path = path;
           fileSearch.Variable = variable;

           this.Layout.Wix.Bundle.FileSearches.Add(fileSearch);

            this.Layout.AddExe(testExeFile, true);

            this.Layout.BuildBundle(false);
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

        public bool SearchResultVerification(FileSearchElement.FileSearchResultType resultType, string variable, string path)
        {
            RunScenario(InstallMode.install, UiMode.Passive, UserType.CurrentUser, false);
            bool result = false;

            string message = string.Empty;

            switch (resultType)
            {
                case FileSearchElement.FileSearchResultType.Exists:
                    message = ConstructBurnCoreMethodCallMessage("GetVariableNumeric", variable);
                    break;
                case FileSearchElement.FileSearchResultType.Version:
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
                if (resultType == FileSearchElement.FileSearchResultType.Exists)
                {
                    if (File.Exists(path))
                    {
                        if (Convert.ToInt32(variableValue) == 1)
                        {
                            Trace.WriteLine(string.Format("File path: {0} found in system as expected",
                                path));
                            return true;
                        }
                        else
                        {
                            Trace.WriteLine(string.Format("File path: {0} not found in system",
                               path));
                            return false;
                        }

                    }
                    else
                    {
                        if (Convert.ToInt32(variableValue) == 0)
                        {
                            Trace.WriteLine(string.Format("File path: {0} not found in system as expected",
                               path));
                            return true;
                        }
                        else
                        {
                            Trace.WriteLine(string.Format("File search returned value indicating file exists in system even though it was not found in system",
                               path));
                            return false;
                        }

                    }
                }
                else if (resultType == FileSearchElement.FileSearchResultType.Version)
                {
                    if (File.Exists(path))
                    {
                        string fileVersion = GetFileVersion(path);

                        long value = Convert.ToInt64(variableValue);

                        int major = (int)(value & (0xffff << 48) >> 48);
                        int minor = (int)(value & (0xffff << 32) >> 32);
                        int build = (int)(value & (0xffff << 16) >> 16);
                        int revision = (int)(value & 0xffff);

                       

                    }
                    else
                    {
                        Trace.WriteLine(string.Format("File path: {0} not found in system. For version verification make sure file exists in target system"
                            , path));

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
