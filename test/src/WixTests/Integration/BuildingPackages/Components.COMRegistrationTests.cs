//-----------------------------------------------------------------------
// <copyright file="Components.COMRegistrationTests.cs" company="Microsoft">
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
//     Tests for COM registration
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Integration.BuildingPackages.Components
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using WixTest;

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
