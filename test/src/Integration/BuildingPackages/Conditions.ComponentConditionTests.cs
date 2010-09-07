//-----------------------------------------------------------------------
// <copyright file="Conditions.ComponentConditionTests.cs" company="Microsoft">
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
//     Tests for conditions as they apply to components
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Conditions
{
    using System;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for conditions as they apply to Components
    /// </summary>
    [TestClass]
    public class ComponentConditionTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Conditions\ComponentConditionTests");

        [TestMethod]
        [Description("Verify that a condition for a component can be specified")]
        [Priority(1)]
        public void SimpleCondition()
        {
            string msi = Builder.BuildPackage(Path.Combine(ComponentConditionTests.TestDataDirectory, @"SimpleCondition\product.wxs"));
            
            string query = "SELECT `Condition` FROM `Component` WHERE `Component` = 'Component1'";
            Verifier.VerifyQuery(msi, query, "1 < 2");

            query = "SELECT `Condition` FROM `Component` WHERE `Component` = 'Component2'";
            Verifier.VerifyQuery(msi, query, "%MyEnvironmentVariable~=\"A\"");
        }
    }
}
