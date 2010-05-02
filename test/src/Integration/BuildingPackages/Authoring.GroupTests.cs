//-----------------------------------------------------------------------
// <copyright file="Authoring.GroupTests.cs" company="Microsoft">
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
//     Tests for the grouping concept in WiX. Most grouping tests specific
//     to certain elements will be tested with that element. 
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Authoring
{
    using System;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for the grouping concept in WiX
    /// </summary>
    [TestClass]
    public class GroupTests :WixTests
    {
        [TestMethod]
        [Description("Verify that a group of one type cannot contain a sub-group of an invalid type")]
        [Priority(1)]
        [Ignore]
        public void InvalidNesting()
        {
        }

        [TestMethod]
        [Description("Verify that a group cannot reference the same group twice")]
        [Priority(1)]
        [Ignore]
        public void DuplicateReferences()
        {
        }

        [TestMethod]
        [Description("Verify that two groups of different types can have the same Id")]
        [Priority(1)]
        [Ignore]
        public void GroupIds()
        {
        }
    }
}
