//-----------------------------------------------------------------------
// <copyright file="Features.RefAndGroupTests.cs" company="Microsoft">
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
//     Tests for FeatureGroups and FeatureRefs
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Features
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for FeatureGroups and FeatureRefs
    /// </summary>
    [TestClass]
    public class RefAndGroupTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Features\RefAndGroupTests");

        [TestMethod]
        [Description("Verify that features can be referenced")]
        [Priority(1)]
        [Ignore]
        public void FeatureRefs()
        {
        }

        [TestMethod]
        [Description("Verify that feature group can be created in Fragments/FeatureRefs and referenced")]
        [Priority(1)]
        [Ignore]
        public void FeatureGroups()
        {
        }

        [TestMethod]
        [Description("Verify that features can be nested")]
        [Priority(1)]
        [Ignore]
        public void NestedFeatures()
        {
        }

        [TestMethod]
        [Description("Verify that feature groups can be nested and referenced")]
        [Priority(1)]
        [Ignore]
        public void NestedFeatureGroups()
        {
        }

        [TestMethod]
        [Description("Verify that the Product element can contain Features, FeatureGroups, FeatureRefs and FeatureGroupRefs")]
        [Priority(1)]
        [Ignore]
        public void ComplexFeatureUsage()
        {
        }

        [TestMethod]
        [Description("Verify that Merge Module References are handled correctly within FeatureGroups")]
        [Priority(2)]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/download.php?group_id=105970&atid=642714&file_id=238466&aid=1760155")]
        public void FeatureGroupContainingMergeRef()
        {
            string msi = Builder.BuildPackage(Path.Combine(RefAndGroupTests.TestDataDirectory, @"FeatureGroupContainingMergeRef\Product.wxs"));

            // verify only one row is added for the merge module and it has the correct value
            string query = "SELECT `Component_` FROM `FeatureComponents`";
            Verifier.VerifyQuery(msi, query, "ModuleComponent.D75D42C7_6B72_46FE_8EB1_83D02B9341D2");
        }
    }
}
