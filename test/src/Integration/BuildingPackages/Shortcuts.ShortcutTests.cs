//-----------------------------------------------------------------------
// <copyright file="Shortcuts.ShortcutTests.cs" company="Microsoft">
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
//     Tests for shortcuts
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Shortcuts
{
    using System;
    using System.IO;
    using System.Text;

    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Tests for shortcuts
    /// </summary>
    [TestClass]
    public class ShortcutTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Integration\BuildingPackages\Shortcuts\ShortcutTests");

        [TestMethod]
        [Description("Verify that a simple shortcut can be created")]
        [Priority(1)]
        public void SimpleShortcut()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(ShortcutTests.TestDataDirectory, @"SimpleShortcut\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.SuppressedICEs.Add("ICE66");
            light.Run();

            Verifier.VerifyResults(Path.Combine(ShortcutTests.TestDataDirectory, @"SimpleShortcut\expected.msi"), light.OutputFile, "Shortcut");
        }
    }
}
