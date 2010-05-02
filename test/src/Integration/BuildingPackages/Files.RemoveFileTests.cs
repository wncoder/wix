//-----------------------------------------------------------------------
// <copyright file="Files.RemoveFileTests.cs" company="Microsoft">
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
//     Tests for the RemoveFile element
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Files
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for the RemoveFile element
    /// </summary>
    [TestClass]
    public class RemoveFilesTests
    {
        [TestMethod]
        [Description("Verify that a file that is installed can removed on install, uninstall or both")]
        [Priority(1)]
        [Ignore]
        public void SimpleRemoveFile()
        {
        }

        [TestMethod]
        [Description("Verify that multiple files can be removed with wildcard characters")]
        [Priority(1)]
        [Ignore]
        public void WildcardRemoveFile()
        {
        }

        [TestMethod]
        [Description("Verify that the file to be removed can be specified in a Property")]
        [Priority(1)]
        [Ignore]
        public void RemoveFileWithProperty()
        {
        }

        [TestMethod]
        [Description("Verify that the file to be removed can be specified in a Directory reference")]
        [Priority(1)]
        [Ignore]
        public void RemoveFileWithDirectory()
        {
        }
    }
}
