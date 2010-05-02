//-----------------------------------------------------------------------
// <copyright file="WixUnit.cs" company="Microsoft">
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
// <summary>A class that wraps WixUnit</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text.RegularExpressions;

    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// A class that wraps WixUnit.
    /// </summary>
    public partial class WixUnit : WixTool
    {
        /// <summary>
        /// Constructor that uses the current directory as the working directory.
        /// </summary>
        public WixUnit()
            : this((string)null)
        {
        }

        /// <summary>
        /// Constructor that uses the default WiX tool directory as the tools location.
        /// </summary>
        /// <param name="workingDirectory">The working directory of the tool.</param>
        public WixUnit(string workingDirectory)
            : base("wixunit.exe", workingDirectory)
        {
            this.IgnoreExtraWixMessages = true;
        }

        /// <summary>
        /// Constructor that accepts a path to the tools location and a working directory.
        /// </summary>
        /// <param name="toolDirectory">The directory of the tool.</param>
        /// <param name="workingDirectory">The working directory of the tool.</param>
        public WixUnit(string toolDirectory, string workingDirectory)
            : base(toolDirectory, "wixunit.exe", workingDirectory)
        {
            this.IgnoreExtraWixMessages = true;
        }

        /// <summary>
        /// Copy constructor
        /// </summary>
        /// <param name="wixUnit">The object to copy</param>
        public WixUnit(WixUnit wixUnit)
            : this(Path.GetDirectoryName(wixUnit.ToolFile), wixUnit.WorkingDirectory)
        {
            this.EnvironmentVariables = wixUnit.EnvironmentVariables;
            this.Help = wixUnit.Help;
            this.NoTidy = wixUnit.NoTidy;
            this.OtherArguments = wixUnit.OtherArguments;
            this.RunFailedTests = wixUnit.RunFailedTests;
            this.SingleThreaded = wixUnit.SingleThreaded;
            this.TestFile = wixUnit.TestFile;
            this.Tests = wixUnit.Tests;
            this.Update = wixUnit.Update;
            this.Validate = wixUnit.Validate;
            this.VerboseOutput = wixUnit.VerboseOutput;
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

            // Explicitly look for WixUnit error messages. We ignore all other tool error messages because
            // they are handled by WixUnit.
            errors.AddRange(this.FindWixUnitWixMessages(result.StandardOutput));

            Regex failedTestRun = new Regex(@"Failed (?<numFailedTests>\d*) out of (?<numTests>\d*) unit test");
            Match messageMatch = failedTestRun.Match(result.StandardOutput);

            if (messageMatch.Success)
            {
                int numFailedTests = Convert.ToInt32(messageMatch.Groups["numFailedTests"].Value);
                int numTests = Convert.ToInt32(messageMatch.Groups["numTests"].Value);
                errors.Add(String.Format("{0} test(s) failed", numFailedTests));
            }

            return errors;
        }

        /// <summary>
        /// Run WixUnit on a particular test
        /// </summary>
        /// <param name="test">The test to run</param>
        /// <returns>The results of the run</returns>
        public Result RunTest(string test)
        {
            // Create a copy of the this object with a new list of tests
            WixUnit wixUnit = new WixUnit(this);
            wixUnit.Tests = new List<string>();
            wixUnit.Tests.Add(test);

            return wixUnit.Run();
        }

        /// <summary>
        /// Functional name of the tool
        /// </summary>
        public override string ToolDescription
        {
            get { return "WixUnit"; }
        }

        /// <summary>
        /// Helper method for finding WixUnit errors and warnings in the output
        /// </summary>
        /// <param name="output">The text to search</param>
        /// <returns>A list of WixMessages in the output</returns>
        private List<string> FindWixUnitWixMessages(string output)
        {
            List<string> wixUnitWixMessages = new List<string>();

            foreach (string line in output.Split('\n', '\r'))
            {
                WixMessage wixUnitWixMessage = WixMessage.FindWixMessage(line, WixTools.Wixunit);

                if (null != wixUnitWixMessage)
                {
                    wixUnitWixMessages.Add(wixUnitWixMessage.ToString());
                }
            }

            return wixUnitWixMessages;
        }
    }
}
