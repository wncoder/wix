//-----------------------------------------------------------------------
// <copyright file="Conditions.ControlConditionTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests for conditions as they apply to controls
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
    /// Tests for conditions as they apply to controls
    /// </summary>
    [TestClass]
    public class ControlConditionTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Conditions\ControlConditionTests");

        [TestMethod]
        [Description("Verify that a condition for a control can be specified")]
        [Priority(1)]
        [Ignore]
        public void SimpleCondition()
        {
        }
    }
}