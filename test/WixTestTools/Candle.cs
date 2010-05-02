//-----------------------------------------------------------------------
// <copyright file="Candle.cs" company="Microsoft">
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
// <summary>A class that wraps Candle</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// A class that wraps Candle.
    /// </summary>
    public partial class Candle : WixTool
    {
        /// <summary>
        /// Set the output location to temp.
        /// </summary>
        private bool outputToTemp;

        /// <summary>
        /// Constructor that uses the current directory as the working directory.
        /// </summary>
        public Candle()
            : this(null)
        {
        }

        /// <summary>
        /// Constructor that uses the default WiX tool directory as the tools location.
        /// </summary>
        /// <param name="workingDirectory">The working directory of the tool.</param>
        public Candle(string workingDirectory)
            : base("candle.exe", workingDirectory)
        {
            this.outputToTemp = true;
        }

        /// <summary>
        /// Constructor that accepts a path to the tools location and a working directory.
        /// </summary>
        /// <param name="toolDirectory">The directory of the tool.</param>
        /// <param name="workingDirectory">The working directory of the tool.</param>
        public Candle(string toolDirectory, string workingDirectory)
            : base(toolDirectory, "candle.exe", workingDirectory)
        {
            this.outputToTemp = true;
        }

        /// <summary>
        /// Specifies whether the output from Candle should be saved to a temp directory. True by default
        /// </summary>
        public bool OutputToTemp
        {
            get { return this.outputToTemp; }
            set { this.outputToTemp = value; }
        }

        /// <summary>
        /// The directory of the output.
        /// </summary>
        public string OutputDirectory
        {
            get
            {
                return Path.GetDirectoryName(this.OutputFile);
            }
        }

        /// <summary>
        /// Based on  current arguments, returns a list of files that Candle is expected to generate.
        /// </summary>
        /// <remarks>
        /// This is an expected list and files are guaranteed to exist.
        /// </remarks>
        public List<string> ExpectedOutputFiles
        {
            get
            {
                List<string> expectedOutputFiles = new List<string>();

                if (this.outputFile.EndsWith(@"\") || this.outputFile.EndsWith(@"/") || String.IsNullOrEmpty(this.outputFile))
                {
                    // Create list of expected files based on how Candle would do it.
                    // Candle would change the extension of each .wxs file to .wixobj.

                    string outputDirectory = (this.outputFile ?? String.Empty);

                    foreach (string sourceFile in this.sourceFiles)
                    {
                        string outputFile = Path.Combine(outputDirectory, Path.GetFileNameWithoutExtension(sourceFile) + ".wixobj");
                        expectedOutputFiles.Add(outputFile);
                    }
                }
                else
                {
                    expectedOutputFiles.Add(this.outputFile);
                }

                return expectedOutputFiles;
            }
        }

        /// <summary>
        /// Functional name of the tool
        /// </summary>
        public override string ToolDescription
        {
            get { return "Compiler"; }
        }

        /// <summary>
        /// Compile a wxs file.
        /// </summary>
        /// <param name="sourceFile">Path to a source file.</param>
        /// <returns>Path to the output file that was created.</returns>
        public static string Compile(string sourceFile)
        {
            if (String.IsNullOrEmpty(sourceFile))
            {
                throw new ArgumentException("sourceFile cannot be null or empty");
            }

            Candle candle = new Candle();
            candle.SourceFiles.Add(sourceFile);
            candle.Run();
            return candle.ExpectedOutputFiles[0];
        }

        /// <summary>
        /// Checks that the result from a run matches the expected results.
        /// </summary>
        /// <param name="result">A result from a run.</param>
        /// <returns>A list of errors.</returns>
        public override List<string> CheckResult(Result result)
        {
            List<string> errors = new List<string>();
            errors.AddRange(base.CheckResult(result));

            // If candle returns success then verify that expected wixobj files are created
            if (result.ExitCode == 0)
            {
                foreach (string file in this.ExpectedOutputFiles)
                {
                    if (!File.Exists(file))
                    {
                        errors.Add(String.Format("Expected wixobj file {0} was not created", file));
                    }
                }
            }

            return errors;
        }

        /// <summary>
        /// Run the tool.
        /// </summary>
        /// <param name="exceptionOnError">If true, throw an exception when expected results don't match actual results.</param>
        /// <returns>The results of the run.</returns>
        public override Result Run(bool exceptionOnError)
        {
            this.SetOutputDirectoryToTemp();
            return base.Run(exceptionOnError);
        }

        /// <summary>
        /// Sets the output directory to a temp folder if specified.
        /// </summary>
        private void SetOutputDirectoryToTemp()
        {
            if (this.OutputToTemp && !Path.IsPathRooted(this.OutputFile))
            {
                string tempDirectory = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
                this.OutputFile = String.Concat(tempDirectory, @"\", this.OutputFile);
            }
        }
    }
}
