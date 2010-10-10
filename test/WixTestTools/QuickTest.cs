//-----------------------------------------------------------------------
// <copyright file="QuickTest.cs" company="Microsoft">
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
//     This class provides a quick and simple way of running a test
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using System;
    using System.Collections.Generic;
    using System.Text;

    /// <summary>
    /// This class provides a quick and simple way of running a test
    /// </summary>
    public partial class QuickTest
    {
        public enum Actions
        {
            None = 0, // do nothing
            BuildMsi // Run Candle and Light against the input files
        }

        /// <summary>
        /// The action of the test
        /// </summary>
        private Actions action = Actions.None;

        /// <summary>
        /// The expected MSI
        /// </summary>
        private string expectedMsi = String.Empty;

        /// <summary>
        /// A list of source files for this test
        /// </summary>
        private List<string> sourceFiles = new List<string>();

        /// <summary>
        /// The action of the test
        /// </summary>
        public Actions Action
        {
            get { return this.action; }
            set { this.action = value; }
        }

        /// <summary>
        /// The expected MSI
        /// </summary>
        public string ExpectedMsi
        {
            get { return this.expectedMsi; }
            set { this.expectedMsi = value; }
        }

        /// <summary>
        /// A list of source files for this test
        /// </summary>
        public List<string> SourceFiles
        {
            get { return this.sourceFiles; }
            set { this.sourceFiles = value; }
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public QuickTest()
        {
        }

        /// <summary>
        /// Run the test
        /// </summary>
        public void Run()
        {
            switch (this.Action)
            {
                case Actions.BuildMsi:
                    this.BuildMsi();
                    break;
                case Actions.None:
                default:
                    break;
            }
        }

        /// <summary>
        /// Build a package from the specifed sources and compare to an expected output
        /// </summary>
        private void BuildMsi()
        {
            // Check that certain pre-conditions are met

            if (this.SourceFiles.Count == 0)
            {
                Assert.Inconclusive("There were no SourceFiles specified. At least on source file must be specified for this test to run.");
            }
            
            if (String.IsNullOrEmpty(this.ExpectedMsi))
            {
                Assert.Inconclusive("The ExpectedMsi was not set. It must be set for this test to run.");
            }

            Candle candle = new Candle();
            candle.SourceFiles = this.SourceFiles;
            candle.Run();

            Light light = new Light(candle);
            light.Run();

            Verifier.VerifyResults(this.ExpectedMsi, light.OutputFile);
        }
    }
}
