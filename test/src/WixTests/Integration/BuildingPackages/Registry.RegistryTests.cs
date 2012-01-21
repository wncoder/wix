//-----------------------------------------------------------------------
// <copyright file="Registry.RegistryTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests for editing registry keys
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.CustomActions
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for editing registry keys
    /// </summary>
    [TestClass]
    public class RegistryTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Registry\RegistryTests");

        [TestMethod]
        [Description("Verify that registry keys can be added")]
        [Priority(1)]
        public void SimpleRegistry()
        {
            string msi = Builder.BuildPackage(Path.Combine(RegistryTests.TestDataDirectory, @"SimpleRegistry\product.wxs"));
            Verifier.VerifyResults(Path.Combine(RegistryTests.TestDataDirectory, @"SimpleRegistry\expected.msi"), msi, "Registry");
        }
    }
}
