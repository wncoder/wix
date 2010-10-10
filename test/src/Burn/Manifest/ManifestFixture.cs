//-----------------------------------------------------------------------
// <copyright file="ManifestFixture.cs" company="Microsoft">
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
//     - Test fixture for Burn Manifest feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.Manifest
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Variable;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;

    public class ManifestFixture : BurnCommonTestFixture
    {
        public ManifestFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager
                (new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX.TestUX());
        }

        public void UninstallMsi()
        {
            string prodCode = MsiUtils.GetMSIProductCode(testMsiPerMachineFile);

            if (MsiUtils.IsProductInstalled(prodCode))
            {
                MsiUtils.RemoveMSI(testMsiPerMachineFile);
            }
        }

        public void InstallMsi()
        {
            string prodCode = MsiUtils.GetMSIProductCode(testMsiPerMachineFile);

            if ( ! MsiUtils.IsProductInstalled(prodCode))
            {
                MsiUtils.InstallMSI(testMsiPerMachineFile);
            }
        }

        public void AddVariableElement(string variableId, string variableValue, VariableElement.VariableDataType type)
        {
            Layout.AddVariable(variableId, variableValue, type);
        }  

        public void BuildLayout(string installCondition, string rollbackCondition)
        {
            Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, true, string.Empty, string.Empty, installCondition
                , rollbackCondition);
            this.Layout.BuildBundle(false);
        }

        public void BuildLayoutRollback(string rollbackCondition)
        {
            Layout.AddMsi(testMsiPerMachineFile, null, testMsiPerMachineUrl, true, string.Empty, string.Empty, string.Empty
                , rollbackCondition);

            this.Layout.AddExe(testExeFile, "TestExe1.exe", testExeUrl, true, string.Empty, rollbackCondition, "/ec 1601");

            this.Layout.BuildBundle(false);
        }

        public void LaunchBurn(InstallMode installMode)
        {
            RunScenario(installMode, UiMode.Passive, UserType.CurrentUser);
        }

        public bool Verify()
        {
            string prodCode = MsiUtils.GetMSIProductCode(testMsiPerMachineFile);
            return MsiUtils.IsProductInstalled(prodCode);
        }
    }
}
