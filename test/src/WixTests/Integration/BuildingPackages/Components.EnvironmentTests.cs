//-----------------------------------------------------------------------
// <copyright file="Components.EnvironmentTests.cs" company="Microsoft">
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
//     Tests for defining Environment Variables for a component
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Integration.BuildingPackages.Components
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using WixTest;

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