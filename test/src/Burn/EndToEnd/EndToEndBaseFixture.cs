//-----------------------------------------------------------------------
// <copyright file="EndToEndBaseFixture.cs" company="Microsoft">
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
//     - Test Fixture for Pri-0 end-to-end scenarios.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.EndToEnd
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;

    public class EndToEndBaseFixture : BurnCommonTestFixture
    {
        public EndToEndBaseFixture()
        {
            //this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.StdUx());
            // use the TestUX 
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());
        }

        /// <summary>
        /// uninstall/delete everything this test fixture may install or download
        /// </summary>
        public override void CleanUp()
        {
            base.CleanUp();

            MsiUtils.RemoveMSI(testMsiPerMachineFile);
            MsiUtils.RemoveMSI(testMsiPerUserExtCabMsiFile);
            MsiUtils.RemoveMSI(testMsiFile);
            MsiUtils.RemoveMSI(testMsiConditionalPass1File);
            MsiUtils.RemoveMSI(testMsiConditionalPass2File);
            MsiUtils.RemoveMSI(testMsiPerUserFile);

            // delete all testExe logs from all users %temp% folders
            foreach (string tempDir in Microsoft.Tools.WindowsInstallerXml.Test.Utilities.UserUtilities.GetAllUserTempPaths())
            {
                foreach (string file in System.IO.Directory.GetFiles(tempDir, "*-TestExeLog.txt", System.IO.SearchOption.TopDirectoryOnly))
                {
                    System.IO.File.SetAttributes(file, System.IO.FileAttributes.Normal);
                    System.IO.File.Delete(file);
                }
            }

        }

    }
}
