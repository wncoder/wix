//-----------------------------------------------------------------------
// <copyright file="Conditions.LaunchConditionTests.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>
//     Tests for conditions as they apply to controls
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
    /// Tests for defining launch conditions
    /// </summary>
    [TestClass]
    public class LaunchConditionTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Conditions\LaunchConditionTests");

        [TestMethod]
        [Description("Verify that a launch condition can be specified")]
        [Priority(1)]
        public void SimpleCondition()
        {
            string sourceFile = Path.Combine(LaunchConditionTests.TestDataDirectory, @"SimpleCondition\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");
            string query1 = "SELECT `Sequence` FROM `InstallUISequence` WHERE `Action` = 'AppSearch'";
            Verifier.VerifyQuery(msi, query1, "50");
        }
    }
}