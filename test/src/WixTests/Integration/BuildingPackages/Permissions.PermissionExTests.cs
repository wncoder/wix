//-----------------------------------------------------------------------
// <copyright file="Permissions.PermissionExTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests for PermissionEx (setting ACLs on File, Registry, CreateFolder
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Permissions
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using DTF = Microsoft.Deployment.WindowsInstaller;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests for PermissionEx (setting ACLs on File, Registry, CreateFolder
    /// </summary>
    /// <remarks>
    /// PermissionEx is new in Windows Installer 5.0
    /// </remarks>
    [TestClass]
    public class PermissionExTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Permissions\PermissionExTests");

        [TestMethod]
        [Description("Verify PermissionEx can be used on Files")]
        [Priority(2)]
        public void FilePermissionEx()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PermissionExTests.TestDataDirectory, @"FilePermissionEx\product.wxs"));
            candle.Run();

            Light light = new Light(candle);

            // Only run validation if the current version of Windows Installer is 5.0 or above
            if (DTF.Installer.Version < MSIVersions.GetVersion(MSIVersions.Versions.MSI50))
            {
                light.SuppressMSIAndMSMValidation = true;
            }

            light.Run();

            Verifier.VerifyResults(Path.Combine(PermissionExTests.TestDataDirectory, @"FilePermissionEx\expected.msi"), light.OutputFile, "MsiLockPermissionsEx");
        }

        [TestMethod]
        [Description("Verify PermissionEx can be used on Registry")]
        [Priority(2)]
        public void RegistryPermissionEx()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PermissionExTests.TestDataDirectory, @"RegistryPermissionEx\product.wxs"));
            candle.Run();

            Light light = new Light(candle);

            // Only run validation if the current version of Windows Installer is 5.0 or above
            if (DTF.Installer.Version < MSIVersions.GetVersion(MSIVersions.Versions.MSI50))
            {
                light.SuppressMSIAndMSMValidation = true;
            }

            light.Run();

            Verifier.VerifyResults(Path.Combine(PermissionExTests.TestDataDirectory, @"RegistryPermissionEx\expected.msi"), light.OutputFile, "MsiLockPermissionsEx");
        }

        [TestMethod]
        [Description("Verify PermissionEx can be used on CreateFolder")]
        [Priority(2)]
        public void CreateFolderPermissionEx()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PermissionExTests.TestDataDirectory, @"CreateFolderPermissionEx\product.wxs"));
            candle.Run();

            Light light = new Light(candle);

            // Only run validation if the current version of Windows Installer is 5.0 or above
            if (DTF.Installer.Version < MSIVersions.GetVersion(MSIVersions.Versions.MSI50))
            {
                light.SuppressMSIAndMSMValidation = true;
            }

            light.Run();

            Verifier.VerifyResults(Path.Combine(PermissionExTests.TestDataDirectory, @"CreateFolderPermissionEx\expected.msi"), light.OutputFile, "MsiLockPermissionsEx");
        }

        [TestMethod]
        [Description("Verify PermissionEx can be used twice on one File")]
        [Priority(2)]
        public void PermissionExTwiceOnOneFile()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PermissionExTests.TestDataDirectory, @"PermissionExTwiceOnOneFile\product.wxs"));
            candle.Run();

            Light light = new Light(candle);

            // Only run validation if the current version of Windows Installer is 5.0 or above
            if (DTF.Installer.Version < MSIVersions.GetVersion(MSIVersions.Versions.MSI50))
            {
                light.SuppressMSIAndMSMValidation = true;
            }

            light.Run();

            Verifier.VerifyResults(Path.Combine(PermissionExTests.TestDataDirectory, @"PermissionExTwiceOnOneFile\expected.msi"), light.OutputFile, "MsiLockPermissionsEx");
        }
    }
}