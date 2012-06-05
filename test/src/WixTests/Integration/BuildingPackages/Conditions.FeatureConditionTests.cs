//-----------------------------------------------------------------------
// <copyright file="Conditions.FeatureConditionTests.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>
//     Tests for conditions as they apply to features
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Integration.BuildingPackages.Conditions
{
    using System;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;

    /// <summary>
    /// Tests for conditions as they apply to features
    /// </summary>
    [TestClass]
    public class FeatureConditionTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Conditions\FeatureConditionTests");

        [TestMethod]
        [Description("Verify that a condition for a feature can be specified")]
        [Priority(1)]
        public void SimpleCondition()
        {
            string msi = Builder.BuildPackage(Path.Combine(FeatureConditionTests.TestDataDirectory, @"SimpleCondition\product.wxs"));

            string query = "SELECT `Condition` FROM `Condition` WHERE `Feature_` = 'Feature1'";
            Verifier.VerifyQuery(msi, query, "Property1=\"A\"");

            query = "SELECT `Condition` FROM `Condition` WHERE `Feature_` = 'Feature2'";
            Verifier.VerifyQuery(msi, query, "1=1");
        }
    }
}
