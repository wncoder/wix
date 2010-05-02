//-----------------------------------------------------------------------
// <copyright file="Files.CopyFileTests.cs" company="Microsoft">
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
//     Tests for the CopyFile element
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
    /// Tests for the CopyFile element
    /// </summary>
    [TestClass]
    public class CopyFileTests
    {
        [TestMethod]
        [Description("Verify that a file that is installed can be copied")]
        [Priority(1)]
        [Ignore]
        public void CopyInstalledFile()
        {
        }

        [TestMethod]
        [Description("Verify that a file that is installed can be moved")]
        [Priority(1)]
        [Ignore]
        public void MoveInstalledFile()
        {
        }

        [TestMethod]
        [Description("Verify that a file that is already on the machine can be copied")]
        [Priority(1)]
        [Ignore]
        public void CopyExistingFile()
        {
        }

        [TestMethod]
        [Description("Verify that a file that is already on the machine can be moved")]
        [Priority(1)]
        [Ignore]
        public void MoveExistingFile()
        {
        }


        [TestMethod]
        [Description("Verify that there is an error if FileId is not a defined file")]
        [Priority(1)]
        [Ignore]
        public void CopyNonExistingFile()
        {
        }
    }
}
