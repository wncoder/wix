//-----------------------------------------------------------------------
// <copyright file="Components.EnvironmentTests.cs" company="Microsoft">
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
    public class EnvironmentTests
    {
        [TestMethod]
        [Description("Verify that an environment variable can be created")]
        [Priority(1)]
        [Ignore]
        public void CreateEnvironment()
        {
        }

        [TestMethod]
        [Description("Verify that an environment variable can be changed")]
        [Priority(1)]
        [Ignore]
        public void ChangeEnvironment()
        {
        }

        [TestMethod]
        [Description("Verify that an environment variable can be removed")]
        [Priority(1)]
        [Ignore]
        public void RemoveEnvironment()
        {
        }

    }
}