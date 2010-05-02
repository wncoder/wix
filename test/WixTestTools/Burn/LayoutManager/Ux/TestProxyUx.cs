//-----------------------------------------------------------------------
// <copyright file="TestProxyUx.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>To make API call to IBurnCore methods from mstest</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux
{
    public class TestProxyUx : UxBase
    {
        public TestProxyUx()
        {
            UxBinaryFilename = "TestProxyEntryPoint.dll";
        }


        public override void CopyAndConfigureUx(string LayoutLocation)
        {
            // Location to find SampleUx binaries
            string srcBinDir = Microsoft.Tools.WindowsInstallerXml.Test.Settings.WixToolDirectory;
                                 
            // Copy the ManagedUX.dll binaries
            string[] SampleUxBinaries = new string[] { "TestProxyEntryPoint.dll", "ManagedBurnProxy.dll", "BurnTestTools.dll"
                , "Interop.IWshRuntimeLibrary.dll", "WixTests.dll", "WixTestTools.dll" };
            foreach (string uxFile in SampleUxBinaries)
            {
                string srcUxFile = Path.Combine(srcBinDir, uxFile);
                string destUxFile = Path.Combine(LayoutLocation, uxFile);
                LayoutManager.CopyFile(srcUxFile, destUxFile);
            }

            // Copy the SampleUx resource files
            string[] SampleUxResourceFolders = new string[] { "1033" }; // "UIData", isn't needed, currently only have resources for 1033 (ENU).
            foreach (string folder in SampleUxResourceFolders)
            {
                string srcResDir = Path.Combine(srcBinDir, folder);
                string destResDir = Path.Combine(LayoutLocation, folder);
                foreach (string srcResFile in Directory.GetFiles(srcResDir, "*", SearchOption.AllDirectories))
                {
                    string destResFile = srcResFile.Replace(srcResDir, destResDir);
                    LayoutManager.CopyFile(srcResFile, destResFile);
                }
            }

        }

        public override Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.UxElement GetBurnManifestUxElement()
        {
            UxElement myUX = new UxElement();
            myUX.SourceFile = "TestProxyEntryPoint.dll";
            string[] sampleUxBinaries = new string[] { "ManagedBurnProxy.dll", "Interop.IWshRuntimeLibrary.dll", "WixTests.dll", "WixTestTools.dll"};
            foreach (string resFile in sampleUxBinaries)
            {
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.ResourceElement re =
                    new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.ResourceElement();
                re.SourceFile = resFile;
                myUX.Resources.Add(re);
            }
            string[] sampleUxResources = new string[] {"1033\\eula.rtf"};
            foreach (string resFile in sampleUxResources)
            {
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.ResourceElement re =
                    new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.ResourceElement();
                re.SourceFile = resFile;
                re.FileName = resFile;
                myUX.Resources.Add(re);
            }
            return myUX;
        }
    }
}
