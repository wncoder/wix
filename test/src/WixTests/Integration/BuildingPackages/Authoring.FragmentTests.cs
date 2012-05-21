//-----------------------------------------------------------------------
// <copyright file="Authoring.FragmentTests.cs" company="Microsoft">
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
//     Tests for a Fragment
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Integration.BuildingPackages.Authoring
{
    using System;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;

    /// <summary>
    /// Tests for a Fragment
    /// </summary>
    [TestClass]
    public class FragmentTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Authoring\FragmentTests");

        [TestMethod]
        [Description("Verify that multiple fragments can be defined and referenced indirectly and that multiple levels of referencing is supported")]
        [Priority(1)]
        public void MultipleFragments()
        {
            string sourceFile = Path.Combine(FragmentTests.TestDataDirectory, @"MultipleFragments\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");
            Verifier.VerifyResults(Path.Combine(FragmentTests.TestDataDirectory, @"MultipleFragments\expected.msi"), msi, "Feature", "Component", "FeatureComponents");
        }

        [TestMethod]
        [Description("Verify that there is an error if FragmentRef is used")]
        [Priority(3)]
        public void FragmentRef()
        {
            string sourceFile = Path.Combine(FragmentTests.TestDataDirectory, @"FragmentRef\product.wxs");

            Candle candle = new Candle();
            candle.SourceFiles.Add(sourceFile);
            candle.ExpectedExitCode = 5;
            candle.ExpectedWixMessages.Add(new WixMessage(5, "The Product element contains an unexpected child element 'FragmentRef'.", WixMessage.MessageTypeEnum.Error));
            candle.Run();
        }
    }
}
