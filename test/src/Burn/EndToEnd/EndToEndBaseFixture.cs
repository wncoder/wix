﻿//-----------------------------------------------------------------------
// <copyright file="EndToEndBaseFixture.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Test Fixture for Pri-0 end-to-end scenarios.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.EndToEnd
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;

    public class EndToEndBaseFixture : BurnCommonTestFixture
    {
        public EndToEndBaseFixture()
        {
            //this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux.StdUx());
            // use the SampleUx 
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux.SampleUx());
        }

        /// <summary>
        /// uninstall/delete everything this test fixture may install or download
        /// </summary>
        public override void CleanUp()
        {
            base.CleanUp();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.RemoveMSI(testMsiPerMachineFile);
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.RemoveMSI(testMsiPerUserExtCabMsiFile);
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.RemoveMSI(testMsiFile);
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.RemoveMSI(testMsiConditionalPass1File);
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.RemoveMSI(testMsiConditionalPass2File);
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.RemoveMSI(testMsiPerUserFile);

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
