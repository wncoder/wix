//-----------------------------------------------------------------------
// <copyright file="BurnTests.cs" company="Microsoft">
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
//     Contains methods to help test Burn.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn
{
    using System;
    using System.IO;
    using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Win32;

    [TestClass]
    public class BurnTests : WixTests
    {
        public static string PayloadCacheFolder = "Package Cache";
        public static string PerMachinePayloadCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%ProgramData%\" + PayloadCacheFolder);
        public static string PerUserPayloadCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%LOCALAPPDATA%\" + PayloadCacheFolder);

        protected static readonly string[] Extensions = new string[] { "WixBalExtension", "WixDependencyExtension", "WixUtilExtension" };

        protected string GetTestInstallFolder(string additionalPath = null)
        {
            return Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86), "~Test WiX", this.TestContext.TestName, additionalPath ?? String.Empty);
        }

        protected RegistryKey GetTestRegistryRoot(string additionalPath = null)
        {
            string key = String.Format(@"Software\WiX\Tests\{0}\{1}", this.TestContext.TestName, additionalPath ?? String.Empty);
            return Registry.LocalMachine.OpenSubKey(key);
        }

        /// <summary>
        /// Sets the requested state for a package that the TestBA will return to the engine during plan.
        /// </summary>
        /// <param name="packageId">Package identity.</param>
        /// <param name="state">State to request.</param>
        protected void SetPackageRequestedState(string packageId, RequestState state)
        {
            string key = String.Format(@"Software\WiX\Tests\{0}\{1}", this.TestContext.TestName, packageId);
            using (RegistryKey packageKey = Registry.LocalMachine.CreateSubKey(key))
            {
                packageKey.SetValue("Requested", state.ToString());
            }
        }
    }
}
