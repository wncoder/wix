//-----------------------------------------------------------------------
// <copyright file="Sequencing.SequencingTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     General tests for sequencing
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Sequencing
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// General tests for sequencing
    /// </summary>
    [TestClass]
    public class SequencingTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Sequencing\SequencingTests");

        [TestMethod]
        [Description("Verify that a custom action can be sequenced")]
        [Priority(1)]
        public void SimpleSequencing()
        {
            string msi = Builder.BuildPackage(Path.Combine(SequencingTests.TestDataDirectory, @"SimpleSequencing\product.wxs"));
            string expectedMsi = Path.Combine(SequencingTests.TestDataDirectory, @"SimpleSequencing\expected.msi");
            Verifier.VerifyResults(expectedMsi, msi, "AdminExecuteSequence", "InstallExecuteSequence");
        }
    }
}
