//-----------------------------------------------------------------------
// <copyright file="SymbolPaths.SymbolPathTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests for SymbolPath elements.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.SymbolPaths
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for SymbolPath elements.
    /// </summary>
    [TestClass]
    public class SymbolPathTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\SymbolPaths\SymbolPathTests");

        [TestMethod]
        [Description("Verify that a SymbolPath element can exist under a component")]
        [Priority(2)]
        public void ComponentSymbolPath()
        {
            string sourceFile = Path.Combine(SymbolPathTests.TestDataDirectory, @"ComponentSymbolPath\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");
            Verifier.VerifyResults(Path.Combine(SymbolPathTests.TestDataDirectory, @"ComponentSymbolPath\expected.msi"), msi);
        }
    }
}