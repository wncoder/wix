//-----------------------------------------------------------------------
// <copyright file="Components.RemoveFolderTests.cs" company="Microsoft">
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
//     Tests for the RemoveFolder element
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
    /// Tests for the CreateFolder 
    /// </summary>
    [TestClass]
    public class RemoveFolderTests
    {
        [TestMethod]
        [Description("Verify that a simple use of the RemoveFolder element adds the correct entry to the RemoveFile table")]
        [Priority(1)]
        [Ignore]
        public void SimpleRemoveFolder()
        {
        }

        [TestMethod]
        [Description("Verify that multiple unique RemoveFolder elements can exist as children of Component")]
        [Priority(1)]
        [Ignore]
        public void MultipleRemoveFolders()
        {
        }

        [TestMethod]
        [Description("Verify that multiple duplicate RemoveFolder elements cannot exist as children of Component")]
        [Priority(1)]
        [Ignore]
        public void DuplicateRemoveFolders()
        {
        }

        [TestMethod]
        [Description("Verify that a folder can be specified to be removed on uninstall")]
        [Priority(1)]
        [Ignore]
        public void RemoveFolderOnUninstall()
        {
        }

        [TestMethod]
        [Description("Verify that a folder can be specified to be removed on install and uninstall")]
        [Priority(1)]
        [Ignore]
        public void RemoveFolderOnBoth()
        {
        }

        [TestMethod]
        [Description("Verify that a Property can be used to specify which folder to remove")]
        [Priority(1)]
        [Ignore]
        public void RemoveFolderUsingProperty()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the Directory attribute is used with the Property attribute")]
        [Priority(1)]
        [Ignore]
        public void DirectoryAttributeWithPropertyAttribute()
        {
        }
    }
}
