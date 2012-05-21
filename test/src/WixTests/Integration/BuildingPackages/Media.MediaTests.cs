//-----------------------------------------------------------------------
// <copyright file="Media.MediaTests.cs" company="Microsoft">
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
//     Tests for the Media element
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Integration.BuildingPackages.Media
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using WixTest;

    /// <summary>
    /// Tests for the Media element
    /// </summary>
    [TestClass]
    public class MediaTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Media\MediaTests");

        [TestMethod]
        [Description("Verify that files can be assigned to different media")]
        [Priority(1)]
        public void SimpleMedia()
        {
            string sourceFile = Path.Combine(MediaTests.TestDataDirectory, @"SimpleMedia\product.wxs");
            string msi = Builder.BuildPackage(sourceFile);

            Verifier.VerifyResults(Path.Combine(MediaTests.TestDataDirectory, @"SimpleMedia\expected.msi"), msi, "File", "Media");
        }
    }
}