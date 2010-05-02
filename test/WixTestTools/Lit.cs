//-----------------------------------------------------------------------
// <copyright file="Lit.cs" company="Microsoft">
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
// <summary>A class that wraps Lit</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.IO;
    using System.Collections.Generic;

    /// <summary>
    /// A class that wraps Lit
    /// </summary>
    public partial class Lit : WixTool
    {
        /// <summary>
        /// Default output extension for Lit output
        /// </summary>
        private const string OutputFileExtension = ".wixlib";

        /// <summary>
        /// Constructor that uses the current directory as the working directory
        /// </summary>
        public Lit()
            : this(String.Empty)
        {
        }

        /// <summary>
        /// Constructor that uses the default WiX tool directory as the tools location
        /// </summary>
        /// <param name="workingDirectory">The working directory of the tool</param>
        public Lit(string workingDirectory)
            : base("lit.exe", workingDirectory)
        {
            SetDefaultOutputFile();
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="toolDirectory">The directory of the tool</param>
        /// <param name="workingDirectory">The working directory of the tool</param>
        public Lit(string toolDirectory, string workingDirectory)
            : base(toolDirectory, "lit.exe", workingDirectory)
        {
            SetDefaultOutputFile();
        }

        /// <summary>
        /// Constructor that uses data from a Candle object to create a Lit object
        /// </summary>
        /// <param name="candle">A Candle object</param>
        public Lit(Candle candle)
            : base(Path.GetDirectoryName(candle.ToolFile), "lit.exe", candle.WorkingDirectory)
        {
            // The output of Candle is the input for Lit
            this.ObjectFiles = candle.ExpectedOutputFiles;
            SetDefaultOutputFile();
        }
        /// <summary>
        /// Run the tool.
        /// </summary>
        /// <param name="exceptionOnError">If true, throw an exception when expected results don't match actual results.</param>
        /// <returns>The results of the run.</returns>
        public override Result Run(bool exceptionOnError)
        {
            return base.Run(exceptionOnError);
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

            // If Lit returns success then verify that expected wix file is created
            if (result.ExitCode == 0)
            {
                if (null != this.ObjectFiles && this.ObjectFiles.Count > 0)
                {
                    if (false == this.Help && !File.Exists(this.ExpectedOutputFile))
                    {
                        errors.Add(String.Format("Expected wix file {0} was not created", this.ExpectedOutputFile));
                    }
                }
            }
            return errors;
        }

        /// <summary>
        /// The expected output file of Lit that is guaranteed to exist
        /// </summary>
        public string ExpectedOutputFile
        {
            get
            {
                if (!string.IsNullOrEmpty(this.OutputFile))
                {
                    return this.OutputFile;
                }
                else
                {
                    if (null != this.ObjectFiles && 1 == this.ObjectFiles.Count)
                    {
                        return Path.Combine(this.WorkingDirectory, String.Concat(Path.GetFileNameWithoutExtension(this.ObjectFiles[0]), Lit.OutputFileExtension));
                    }
                    else
                    {
                        return Path.Combine(this.WorkingDirectory, String.Concat("product", Lit.OutputFileExtension));
                    }
                }
            }
        }

        /// <summary>
        /// Functional name of the tool
        /// </summary>
        public override string ToolDescription
        {
            get { return "Library Tool"; }
        }

        /// <summary>
        /// Sets the output file as Lit to a temporary directory
        /// </summary>
        private void SetDefaultOutputFile()
        {
            DirectoryInfo tempDirectory = Directory.CreateDirectory(Builder.GetUniqueFileName());

            if (null != this.ObjectFiles && 1 == this.ObjectFiles.Count)
            {
                this.OutputFile = Path.Combine(tempDirectory.FullName, String.Concat(Path.GetFileNameWithoutExtension(this.ObjectFiles[0]), Lit.OutputFileExtension));
            }
            else
            {
                this.OutputFile = Path.Combine(tempDirectory.FullName, String.Concat("product", Lit.OutputFileExtension));
            }
        }
    }
}
