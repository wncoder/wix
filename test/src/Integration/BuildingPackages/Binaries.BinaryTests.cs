//-----------------------------------------------------------------------
// <copyright file="Binaries.BinaryTests.cs" company="Microsoft">
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
//     Tests for embedded binaries
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Binaries
{
    using System;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for embedded binaries
    /// </summary>
    [TestClass]
    public class BinaryTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Integration\BuildingPackages\Binaries\BinaryTests");

        [TestMethod]
        [Description("Verify that binary data can be embedded in an MSI")]
        [Priority(1)]
        public void SimpleBinary()
        {
            string msi = Builder.BuildPackage(Path.Combine(BinaryTests.TestDataDirectory, @"SimpleBinary\product.wxs"));
            Verifier.VerifyResults(Path.Combine(BinaryTests.TestDataDirectory, @"SimpleBinary\expected.msi"), msi, "Binary");

            Dark dark = new Dark();
            dark.InputFile = msi;
            dark.OutputFile = "decompiled.wxs";
            dark.BinaryPath = dark.CreateUniqueDirectory(Path.GetTempPath());
            dark.Run();
        }

        [TestMethod]
        [Description("Verify that a large file (? bytes) can be embedded in an MSI")]
        [Priority(2)]
        [Ignore]
        public void LargeBinary()
        {
        }

        [TestMethod]
        [Description("Verify that a 0 byte file can be embedded in an MSI")]
        [Priority(3)]
        [Ignore]
        public void SmallBinary()
        {
        }

    }
}
