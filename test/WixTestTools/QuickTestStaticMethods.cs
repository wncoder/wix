//-----------------------------------------------------------------------
// <copyright file="QuickTestStaticMethods.cs" company="Microsoft">
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
//     Static methods for running QuickTests
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using System;
    using System.Collections.Generic;
    using System.Text;

    /// <summary>
    /// Static methods for running QuickTests
    /// </summary>
    public partial class QuickTest
    {
        /// <summary>
        /// Build an MSI with Candle and Light then compare it to an expected MSI
        /// </summary>
        /// <param name="sourceFile">The wxs source file</param>
        /// <param name="expectedMsi">The path to the expected MSI</param>
        public static void BuildMsiTest(string sourceFile, string expectedMsi)
        {
            List<string> sourceFiles = new List<string>();
            sourceFiles.Add(sourceFile);

            QuickTest.BuildMsiTest(sourceFiles, expectedMsi);
        }

        /// <summary>
        /// Build an MSI with Candle and Light then compare it to an expected MSI
        /// </summary>
        /// <param name="sourceFiles">The wxs source files</param>
        /// <param name="expectedMsi">The path to the expected MSI</param>
        public static void BuildMsiTest(List<string> sourceFiles, string expectedMsi)
        {
            QuickTest quickTest = new QuickTest();
            quickTest.Action = QuickTest.Actions.BuildMsi;
            quickTest.SourceFiles = sourceFiles;
            quickTest.ExpectedMsi = expectedMsi;
            quickTest.Run();
        }
    }
}
