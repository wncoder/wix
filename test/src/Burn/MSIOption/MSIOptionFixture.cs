//-----------------------------------------------------------------------
// <copyright file="MSIOptionFixture.cs" company="Microsoft">
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
//     - Test fixture for Burn MSIOption feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.MSIOption
{
    using System.Diagnostics;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Variables;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;

    public class MSIOptionFixture : BurnCommonTestFixture
    {
        public MSIOptionFixture() : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager
                (new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux.SampleUx());

            this.UseABundle = true;
        }

        public void BuildLayout(string msiPropertyId, string msiPropertyValue)
        {
            Layout.AddMsi(testMsiConditionalPass3File, null, testMsiConditionalPass3Url, true, msiPropertyId, msiPropertyValue);
            this.Layout.GenerateBundle();
        }

        public void BuildLayout(string msiPropertyId, string msiPropertyValue, string variableId, string variableValue
            , VariableElement.VariableDataType type)
        {
            Layout.AddMsi(testMsiConditionalPass3File, null, testMsiConditionalPass3Url, true, msiPropertyId, msiPropertyValue);
            Layout.AddVariable(variableId, variableValue, type);
            this.Layout.GenerateBundle();
        }

        public void LaunchBurn(InstallMode installMode)
        {
            RunScenario(installMode, UiMode.Passive, UserType.CurrentUser);
        }

        public void InstallMsi()
        {
            LaunchMsiExec(InstallMode.install);
        }

        public void UninstallMsi()
        {
            LaunchMsiExec(InstallMode.uninstall);
        }

        private void LaunchMsiExec(InstallMode installMode)
        {
            Process process = new Process();
            process.StartInfo.FileName = "msiexec";

            string installCommand = string.Empty;
            if (installMode == InstallMode.install)
            {
                installCommand = "/i";
            }
            else if (installMode == InstallMode.uninstall)
            {
                installCommand = "/x";
            }

            process.StartInfo.Arguments = string.Format("{0} {1} /q PASSME=1", installCommand, testMsiConditionalPass3File);
            process.StartInfo.WindowStyle = ProcessWindowStyle.Normal;
            process.Start();
            process.WaitForExit();
        }

        public bool Verify()
        {           
            string prodCode = MsiUtils.GetMSIProductCode(testMsiConditionalPass3File);
            return MsiUtils.IsProductInstalled(prodCode);
        }
    }
}
