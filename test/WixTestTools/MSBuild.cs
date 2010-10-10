//-----------------------------------------------------------------------
// <copyright file="MSBuild.cs" company="Microsoft">
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
// <summary>A class that wraps MSBuild</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text.RegularExpressions;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// A class that wraps MSBuild.
    /// </summary>
    public partial class MSBuild : TestTool
    {
        /// <summary>
        /// The MSBuild root output directory.
        /// </summary>
        private string outputRootDirectory = String.Empty;

        /// <summary>
        /// The expected MSBuild messages.
        /// </summary>
        private List<MSBuildMessage> expectedMSBuildMessages = new List<MSBuildMessage>();

        /// <summary>
        /// Ignore MSBuildMessages that were not expected.
        /// </summary>
        private bool ignoreExtraMSBuildMessages;

        /// <summary>
        /// Constructor that uses the default location for MSBuild.
        /// </summary>
        public MSBuild()
            : this(Environment.ExpandEnvironmentVariables(Settings.MSBuildDirectory))
        {
        }

        /// <summary>
        /// Constructor that accepts a path to the MSBuild location.
        /// </summary>
        /// <param name="toolDirectory">The directory of MSBuild.exe.</param>
        public MSBuild(string toolDirectory)
            : base(Path.Combine(toolDirectory, "msbuild.exe"), null)
        {
            this.SetDefaultArguments();
        }

        /// <summary>
        /// The expected MSBuild messages
        /// </summary>
        public List<MSBuildMessage> ExpectedMSBuildMessages
        {
            get { return this.expectedMSBuildMessages; }
            set { this.expectedMSBuildMessages = value; }
        }

        /// <summary>
        /// Ignore MSBuildMessages that were not expected.
        /// </summary>
        private bool IgnoreExtraMSBuildMessages
        {
            get { return this.ignoreExtraMSBuildMessages; }
            set { this.ignoreExtraMSBuildMessages = value; }
        }

        /// <summary>
        /// The MSBuild IntermediateOutputPath
        /// </summary>
        public string IntermediateOutputPath
        {
            get
            {
                if (this.Properties.ContainsKey("IntermediateOutputPath"))
                {
                    return this.Properties["IntermediateOutputPath"];
                }
                else
                {
                    return null;
                }

            }

            set
            {
                if (this.Properties.ContainsKey("IntermediateOutputPath"))
                {
                    this.Properties["IntermediateOutputPath"] = value;
                }
                else
                {
                    this.Properties.Add("IntermediateOutputPath", value);
                }
            }
        }

        /// <summary>
        /// The MSBuild OutputPath
        /// </summary>
        public string OutputPath
        {
            get
            {
                if (this.Properties.ContainsKey("OutputPath"))
                {
                    return this.Properties["OutputPath"];
                }
                else
                {
                    return null;
                }

            }

            set
            {
                if (this.Properties.ContainsKey("OutputPath"))
                {
                    this.Properties["OutputPath"] = value;
                }
                else
                {
                    this.Properties.Add("OutputPath", value);
                }
            }
        }

        /// <summary>
        /// The MSBuild root output directory
        /// </summary>
        public string OutputRootDirectory
        {
            get { return this.outputRootDirectory; }

            set
            {
                this.outputRootDirectory = value;
                this.IntermediateOutputPath = Path.Combine(value, @"obj\\");
                this.OutputPath = Path.Combine(value, @"bin\\");
            }
        }

        /// <summary>
        /// Checks that the result from a run matches the expected results
        /// </summary>
        /// <param name="result">A result from a run</param>
        /// <returns>A list of errors</returns>
        public override List<string> CheckResult(Result result)
        {
            List<string> errors = new List<string>();
            errors.AddRange(base.CheckResult(result));

            // Verify that the expected messages are present
            errors.AddRange(this.MSBuildMessageVerification(result.StandardOutput));

            return errors;
        }

        /// <summary>
        /// Perform unordered verification of the list of MSBuildMessages
        /// </summary>
        /// <param name="output">The standard output</param>
        /// <returns>A list of errors encountered during verification</returns>
        private List<string> MSBuildMessageVerification(string output)
        {
            List<string> errors = new List<string>();

            if (null == this.ExpectedMSBuildMessages)
            {
                return errors;
            }

            List<MSBuildMessage> actualMSBuildMessages = this.FindActualMSBuildMessages(output);

            for (int i = 0; i < this.ExpectedMSBuildMessages.Count; i++)
            {
                // If the expectedMessage does not have any specified MessageText then ignore it in a comparison
                bool ignoreText = String.IsNullOrEmpty(this.ExpectedMSBuildMessages[i].MessageText);

                // Flip this bool to true if the expected message is in the list of actual message that were printed
                bool expectedMessageWasFound = false;

                for (int j = 0; j < actualMSBuildMessages.Count; j++)
                {
                    if (null != actualMSBuildMessages[j] && 0 == MSBuildMessage.Compare(actualMSBuildMessages[j], this.ExpectedMSBuildMessages[i], ignoreText))
                    {
                        // Invalidate the message from the list of found errors by setting it to null
                        actualMSBuildMessages[j] = null;

                        expectedMessageWasFound = true;
                    }
                }

                // Check if the expected message was found in the list of actual messages
                if (!expectedMessageWasFound)
                {
                    errors.Add(String.Format("Could not find the expected message: {0}", this.ExpectedMSBuildMessages[i].ToString()));
                }
            }

            if (!this.IgnoreExtraMSBuildMessages)
            {
                // Now go through the messages that were found but that weren't expected
                foreach (MSBuildMessage actualMSBuildMessage in actualMSBuildMessages)
                {
                    if (null != actualMSBuildMessage)
                    {
                        errors.Add(String.Format("Found an unexpected message: {0}", actualMSBuildMessage.ToString()));
                    }
                }
            }

            return errors;
        }

        /// <summary>
        /// Helper method for finding all the errors and all the warnings in the output
        /// </summary>
        /// <returns>A list of MSBuildMessage in the output</returns>
        private List<MSBuildMessage> FindActualMSBuildMessages(string output)
        {
            List<MSBuildMessage> actualMSBuildMessages = new List<MSBuildMessage>();

            foreach (string line in output.Split('\n', '\r'))
            {
                MSBuildMessage actualWixMessage = MSBuildMessage.FindMSBuildMessage(line);

                if (null != actualWixMessage)
                {
                    actualMSBuildMessages.Add(actualWixMessage);
                }
            }

            return actualMSBuildMessages;
        }
    }
}
