//-----------------------------------------------------------------------
// <copyright file="Components.ReserveCostTests.cs" company="Microsoft">
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
//     Tests for the ReserveCost element
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
    /// Tests for the ReserveCost element
    /// </summary>
    [TestClass]
    public class ReserveCostTests : WixTests
    {
        [TestMethod]
        [Description("Verify that a simple use of the ReserveCost element adds the correct entry to the ReserveCost table")]
        [Priority(1)]
        [Ignore]
        public void SimpleReserveCost()
        {
        }

        [TestMethod]
        [Description("Verify that ReserveCost can reserve disk cost for a specified directory and not just the parent Component's directory")]
        [Priority(1)]
        [Ignore]
        public void ReserveCostDirectory()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the required attributes RunFromSource and RunLocal are missing")]
        [Priority(1)]
        [Ignore]
        public void InvalidReserveCost()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the value of RunFromSource is not an integer")]
        [Priority(1)]
        [Ignore]
        public void InvalidRunFromSourceType()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the value of RunLocal is not an integer")]
        [Priority(1)]
        [Ignore]
        public void InvalidRunLocalType()
        {
        }
    }
}
