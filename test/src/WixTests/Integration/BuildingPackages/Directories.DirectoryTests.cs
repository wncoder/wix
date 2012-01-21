//-----------------------------------------------------------------------
// <copyright file="Directories.DirectoryTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests for directories
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Directories
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for directories
    /// </summary>
    [TestClass]
    public class DirectoryTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Directories\DirectoryTests");

        [TestMethod]
        [Description("Verify that directories can be defined and referenced")]
        [Priority(1)]
        public void SimpleDirectory()
        {
        }
    }
}