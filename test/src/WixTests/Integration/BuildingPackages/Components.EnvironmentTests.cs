//-----------------------------------------------------------------------
// <copyright file="Components.EnvironmentTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests for defining Environment Variables for a component
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Components
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for defining Environment Variables for a component
    /// </summary>
    [TestClass]
    public class EnvironmentTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Components\EnvironmentTests");

        [TestMethod]
        [Description("Verify that an environment variable can be created")]
        [Priority(1)]
        public void CreateEnvironment()
        {
            QuickTest .BuildMsiTest (Path.Combine(EnvironmentTests.TestDataDirectory, @"CreateEnvironment\product.wxs"),Path.Combine(EnvironmentTests.TestDataDirectory, @"CreateEnvironment\expected.msi")); 
        }

        [TestMethod]
        [Description("Verify that an environment variable can be changed")]
        [Priority(1)]
        public void ChangeEnvironment()
        {
            QuickTest.BuildMsiTest(Path.Combine(EnvironmentTests.TestDataDirectory, @"ChangeEnvironment\product.wxs"), Path.Combine(EnvironmentTests.TestDataDirectory, @"ChangeEnvironment\expected.msi"));
        }

        [TestMethod]
        [Description("Verify that an environment variable can be removed")]
        [Priority(1)]
        public void RemoveEnvironment()
        {
            QuickTest.BuildMsiTest(Path.Combine(EnvironmentTests.TestDataDirectory, @"RemoveEnvironment\product.wxs"), Path.Combine(EnvironmentTests.TestDataDirectory, @"RemoveEnvironment\expected.msi"));
        }
    }
}