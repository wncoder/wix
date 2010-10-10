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
//     - Contains methods that are shared across Burn specific tests
//     - Performs some initialization before the tests are run
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class BurnTests : WixTests
    {
        // Links that describe how to create data-driven tests:
        //   http://blogs.msdn.com/vstsqualitytools/archive/2006/01/10/511030.aspx
        //   http://msdn.microsoft.com/en-us/library/ms182527.aspx
        //   http://msdn.microsoft.com/en-us/library/ms404700(VS.80).aspx
        // Excel Connection strings:
        //   http://www.connectionstrings.com/excel
        //   http://www.connectionstrings.com/excel-2007
        //
        // Test machines need to have Excel or this installed:
        // 2007 Office System Driver: Data Connectivity Components http://www.microsoft.com/downloads/details.aspx?FamilyID=7554F536-8C28-4598-9B72-EF94E038C891&displaylang=en
        // Bing "How to install Microsoft.ACE.OLEDB.12.0 provider" for more info.

        public static string BundleCacheFolder = "Package Cache";
        public static string PayloadCacheFolder = "Package Cache";
        public static string PerMachineBundleCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%ProgramData%\" + BundleCacheFolder);
        public static string PerUserBundleCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%LOCALAPPDATA%\" + BundleCacheFolder);
        public static string PerMachinePayloadCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%ProgramData%\" + PayloadCacheFolder);
        public static string PerUserPayloadCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%LOCALAPPDATA%\" + PayloadCacheFolder);

        public bool IsDataDrivenTestEnabled()
        {
            bool retVal;
            try
            {
                retVal = (bool)TestContext.DataRow[0];
            }
            catch
            {
                // if Row 0  isn't a bool, assume the test should be run
                retVal = true;
            }
            if (!retVal)
            {
                Console.WriteLine("Test is not enabled for this DataRow.  Skipping Test.");
            }
            return retVal;
        }

    }
}
