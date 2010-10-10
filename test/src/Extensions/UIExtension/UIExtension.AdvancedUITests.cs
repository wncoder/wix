//-----------------------------------------------------------------------
// <copyright file="UIExtension.AdvancedUITests.cs" company="Microsoft">
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
// <summary>UI Extension AdvancedUI tests</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Extensions.UIExtension
{
    using System;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers;

    /// <summary>
    /// NetFX extension AdvancedUI element tests
    /// </summary>
    [TestClass]
    public class AdvancedUITests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Extensions\UIExtension\AdvancedUITests");

        [TestMethod]
        [Description("Verify that the CustomAction Table is created in the MSI and has the expected data.")]
        [Priority(1)]
        public void AdvancedUI_VerifyMSITableData()
        {
            string sourceFile = Path.Combine(AdvancedUITests.TestDataDirectory, @"InstallforAllUser.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixUIExtension");

            Verifier.VerifyCustomActionTableData(msiFile,
                new CustomActionTableData("WixUIValidatePath", 65, "WixUIWixca", "ValidatePath"),
                new CustomActionTableData("WixUIPrintEula", 65, "WixUIWixca", "PrintEula"),
                new CustomActionTableData("WixSetDefaultPerUserFolder", 51, "WixPerUserFolder", @"[LocalAppDataFolder]Apps\[ApplicationFolderName]"),
                new CustomActionTableData("WixSetDefaultPerMachineFolder", 51, "WixPerMachineFolder", "[ProgramFilesFolder][ApplicationFolderName]"),
                new CustomActionTableData("WixSetPerUserFolder", 51, "APPLICATIONFOLDER", "[WixPerUserFolder]"),
                new CustomActionTableData("WixSetPerMachineFolder", 51, "APPLICATIONFOLDER", "[WixPerMachineFolder]"));
        }

        [TestMethod]
        [Description("Verify using the msilog that the correct actions was executed.")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "false")]
        [Ignore]
        public void AdvancedUI_InstallforAllUser()
        {
        }
      
        [TestMethod]
        [Description("Verify using the msilog that the correct actions was executed.")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "false")]
        [Ignore]
        public void AdvancedUI_InstallJustForYou()
        {
        }
     }
}
