//-----------------------------------------------------------------------
// <copyright file="Components.CreateFolderTests.cs" company="Microsoft">
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
//     Tests for the CreateFolder action
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
    public class CreateFolderTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Integration\BuildingPackages\Components\CreateFolderTests");

        [TestMethod]
        [Description("Verify that a simple use of the CreateFolder element adds the correct entries to the CreateFolder table")]
        [Priority(1)]
        public void SimpleCreateFolder()
        {
            string sourceFile = Path.Combine(CreateFolderTests.TestDataDirectory, @"SimpleCreateFolder\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");

            Verifier.VerifyResults(Path.Combine(CreateFolderTests.TestDataDirectory, @"SimpleCreateFolder\expected.msi"), msi, "CreateFolder");
        }

        [TestMethod]
        [Description("Verify that there is an error if two CreateFolder elements try to create the same folder")]
        [Priority(2)]
        [Ignore]
        public void CreateDuplicateFolders1()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if an undefined Directory is referenced")]
        [Priority(2)]
        [Ignore]
        public void InvalidCreateFolder()
        {
        }

        [TestMethod]
        [Description("Verify that a folder can be created with specific permissions")]
        [Priority(2)]
        [Ignore]
        public void CreateFolderWithPermissions()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if one CreateFolder uses its default parent folder and a second CreateFolder explicitly references its parent folder")]
        [Priority(3)]
        [Ignore]
        public void CreateDuplicateFolders2()
        {
        }

        [TestMethod]
        [Description("Verify that a CreateFolder defaults correctly when it is in a floating component")]
        [Priority(3)]
        [Ignore]
        public void CreateFolderInFloatingComponent()
        {
        }

        [TestMethod]
        [Description("Verify that a folder can be created with multiple overlapping permissions")]
        [Priority(3)]
        [Ignore]
        public void CreateFolderWithMultiplePermissions()
        {
        }

        [TestMethod]
        [Description("Verify that shortcuts can be added to created folders")]
        [Priority(3)]
        [Ignore]
        public void CreateFolderShortcut()
        {
        }
    }
}
