//-----------------------------------------------------------------------
// <copyright file="Components.COMRegistrationTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests for COM registration
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
    /// Tests for COM registration
    /// </summary>
    [TestClass]
    public class COMRegistrationTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Components\COMRegistrationTests");

        [TestMethod]
        [Description("Verify that unadvertised class registration is handled correctly.")]
        [Priority(2)]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/index.php?func=detail&aid=1660163&group_id=105970&atid=642714")]
        public void ValidUnadvertisedClass()
        {
            string msi = Builder.BuildPackage(Path.Combine(COMRegistrationTests.TestDataDirectory, @"ValidUnadvertisedClass\product.wxs"));

            string query = @"SELECT `Value` FROM `Registry` WHERE `Key` = 'CLSID\{00000000-0000-0000-0000-000000000001}\InprocServer32'";
            Verifier.VerifyQuery(msi, query, "[#test.txt]");
        }
    }
}
