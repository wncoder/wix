//-----------------------------------------------------------------------
// <copyright file="Components.IsolateComponentTests.cs" company="Microsoft">
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
//     Tests for the IsoloateComponent element
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
    /// Tests for the IsoloateComponent element
    /// </summary>
    [TestClass]
    public class IsolateComponentTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Components\IsolateComponentTests");

        [TestMethod]
        [Description("Verify simple usage of the IsolageComponent element")]
        [Priority(1)]
        public void SimpleIsolateComponent()
        {
            string sourceFile = Path.Combine(IsolateComponentTests.TestDataDirectory, @"SimpleIsolateComponent\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");

            string query = "SELECT `Component_Application` FROM `IsolatedComponent` WHERE `Component_Shared` = 'Component1'";
            Verifier.VerifyQuery(msi, query, "IsolateTest");
        }

        [TestMethod]
        [Description("Verify that there is an error if the Shared attribute's value is an undefined component")]
        [Priority(1)]
        public void InvalidSharedComponent()
        {
            string sourceFile = Path.Combine(IsolateComponentTests.TestDataDirectory, @"InvalidSharedComponent\product.wxs");

            Candle candle = new Candle();
            candle.SourceFiles.Add(sourceFile);
            candle.Run();

            Light light = new Light(candle);
            light.ExpectedExitCode = 94;
            light.ExpectedWixMessages.Add (new WixMessage (94,"Unresolved reference to symbol 'Component:Component2' in section 'Product:*'.",Message.MessageTypeEnum.Error));
            light .Run();
        }
    }
}
