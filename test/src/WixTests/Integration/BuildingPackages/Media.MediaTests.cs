//-----------------------------------------------------------------------
// <copyright file="Media.MediaTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests for the Media element
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Media
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

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