//-----------------------------------------------------------------------
// <copyright file="Sequencing.InstallExecuteSequenceTests.cs" company="Microsoft">
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
//     Tests for the InstallExecuteSequence table
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Sequencing
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;
    using System.Xml;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for the InstallExecuteSequence table
    /// </summary>
    [TestClass]
    public class InstallExecuteSequenceTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Sequencing\InstallExecuteSequenceTests");

        [TestMethod]
        [Description("Verify that all of the standard actions can be added to the InstallExecuteSequence table")]
        [Priority(1)]
        public void AllStandardActions()
        {
            string msi = Builder.BuildPackage(Path.Combine(InstallExecuteSequenceTests.TestDataDirectory, @"AllStandardActions\product.wxs"));

            string expectedMsi = Path.Combine(InstallExecuteSequenceTests.TestDataDirectory, @"AllStandardActions\expected.msi");
            Verifier.VerifyResults(expectedMsi, msi, "InstallExecuteSequence");
        }
    }
}
