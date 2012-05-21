//-----------------------------------------------------------------------
// <copyright file="BindFiles.BindFilesTests.cs" company="Microsoft">
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
//     Tests for how Lit handles -bf switch
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Tools.Lit.BindFiles
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;

    /// <summary>
    /// Tests for how Lit handles -bf switch
    /// </summary>
    [TestClass]
    public class BindFilesTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Tools\Lit\BindFiles\BindFilesTests");

        [TestMethod]
        [Description("Verify that Lit can bind files into a wix library")]
        [Priority(1)]
        public void SimpleBindFiles()
        {
            // Create a temp text file to bind into the wix library
            DirectoryInfo tempDirectory = Directory.CreateDirectory(Utilities.FileUtilities.GetUniqueFileName());
            string testFileName = Path.Combine(tempDirectory.FullName, "TextFile1.txt");
            StreamWriter outputFile = File.CreateText(testFileName);
            outputFile.Write("abc");
            outputFile.Close();

            // Build the library
            Lit lit = new Lit();
            lit.WorkingDirectory = tempDirectory.FullName;
            lit.ObjectFiles.Add(Candle.Compile(Path.Combine(BindFilesTests.TestDataDirectory, @"SimpleBindFiles\Product.wxs")));
            lit.BindFiles = true;
            lit.Run();

            // Delete the source file
            File.Delete(testFileName);

            // Link the library and verify files are in the resulting msi layout
            Light light = new Light(lit);
            light.Run();

            string outputFileName = Path.Combine(Path.GetDirectoryName(lit.OutputFile), @"PFiles\WixTestFolder\TextFile1.txt");
            Assert.IsTrue(File.Exists(outputFileName), "File was not created in msi layout as expected.");
            Assert.IsTrue(File.ReadAllText(outputFileName).Equals("abc"), "File contents do not match expected.");
        }
    }
}