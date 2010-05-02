//-----------------------------------------------------------------------
// <copyright file="Components.RefAndGroupTests.cs" company="Microsoft">
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
//     Tests for ComponentRefs and ComponentGroups
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
    /// Tests for ComponentRefs and ComponentGroups
    /// </summary>
    [TestClass]
    public class RefAndGroupTests : WixTests
    {
        [TestMethod]
        [Description("Verify that Components can be referenced")]
        [Priority(1)]
        [Ignore]
        public void ComponentRef()
        {
        }

        [TestMethod]
        [Description("Verify that a ComponentGroups with child Components and ComponentRefs can be created and referenced")]
        [Priority(1)]
        [Ignore]
        public void ComponentGroups()
        {
        }

        [TestMethod]
        [Description("Verify that a ComponentGroups can be nested using the child element ComponentGroupRef")]
        [Priority(1)]
        [Ignore]
        public void NestedComponentGroups()
        {
        }
    }
}