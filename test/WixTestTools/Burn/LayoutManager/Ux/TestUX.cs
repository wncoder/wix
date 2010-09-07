//-----------------------------------------------------------------------
// <copyright file="SampleUx.cs" company="Microsoft">
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
// <summary>SampleUx to drive end-to-end tests</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX
{
    using System.IO;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.UX;

    public class TestUX : UxBase
    {
        private static string[] TestUxBinaries = new string[] { "BootstrapperCore.dll", "BootstrapperCore.config;TestUX.BootstrapperCore.config", "TestUX.dll" };

        public TestUX()
        {
            base.UxBinaryFilename = "mbahost.dll";
        }

        public override void CopyAndConfigureUx(string LayoutLocation, WixElement Wix)
        {
            string srcBinDir = Microsoft.Tools.WindowsInstallerXml.Test.Settings.WixToolDirectory;

            // Copy the TestUX binaries
            LayoutManager.CopyFile(Path.Combine(srcBinDir, base.UxBinaryFilename), Path.Combine(LayoutLocation, base.UxBinaryFilename));
            foreach (string uxFile in TestUX.TestUxBinaries)
            {
                string[] paths = uxFile.Split(new char[] { ';' });
                string srcUxFile = Path.Combine(srcBinDir, paths.Length == 2 ? paths[1] : paths[0]);
                string destUxFile = Path.Combine(LayoutLocation, paths[0]);
                LayoutManager.CopyFile(srcUxFile, destUxFile);
            }
        }

        public override UXElement GetWixBundleUXElement()
        {
            UXElement myUX = new UXElement();
            myUX.SourceFile = base.UxBinaryFilename;

            foreach (string resFile in TestUX.TestUxBinaries)
            {
                string[] paths = resFile.Split(new char[] { ';' });
                PayloadElement payloadElement = new PayloadElement();
                payloadElement.SourceFile = paths[0];
                myUX.Payloads.Add(payloadElement);
            }

            return myUX;
        }
    }
}
