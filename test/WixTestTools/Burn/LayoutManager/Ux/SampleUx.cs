//-----------------------------------------------------------------------
// <copyright file="SampleUx.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>SampleUx to drive end-to-end tests</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux
{
    using System.IO;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX;

    public class SampleUx : UxBase
    {
        public SampleUx()
        {
            UxBinaryFilename = "UXEntryPoint.dll";
        }

        public override void CopyAndConfigureUx(string LayoutLocation)
        {
            // Location to find SampleUx binaries
            string srcBinDir = Microsoft.Tools.WindowsInstallerXml.Test.Settings.WixToolDirectory;

            // Copy the ManagedUX.dll binaries
            string[] SampleUxBinaries = new string[] { "ManagedUX.dll", "UXEntryPoint.dll", "UXManagedProxy.dll" };
            foreach (string uxFile in SampleUxBinaries)
            {
                string srcUxFile = Path.Combine(srcBinDir, uxFile);
                string destUxFile = Path.Combine(LayoutLocation, uxFile);
                LayoutManager.CopyFile(srcUxFile, destUxFile);
            }

            // Copy the SampleUx resource files
            string[] SampleUxResourceFolders = new string[] { "1033" }; // currently only have test resources for 1033 (ENU).
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
            myUX.SourceFile = "UXEntryPoint.dll";

            string[] sampleUxBinaries = new string[] { "ManagedUX.dll", "UXManagedProxy.dll", "DHtmlHeader.html" };
            foreach (string resFile in sampleUxBinaries)
            {
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.ResourceElement re =
                    new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.ResourceElement();
                re.SourceFile = resFile;
                myUX.Resources.Add(re);
            }
            string[] sampleUxResources = new string[] { "1033\\eula.rtf" }; 
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
