//-----------------------------------------------------------------------
// <copyright file="WixTool.cs" company="Microsoft">
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
// <summary>Wraps a WiX excecutable</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text.RegularExpressions;

    /// <summary>
    /// A base class for a Wix tool, eg. candle.exe
    /// </summary>
    public abstract partial class WixTool : TestTool
    {
        /// <summary>
        /// The expected WiX messages
        /// </summary>
        private List<WixMessage> expectedWixMessages = new List<WixMessage>();

        /// <summary>
        /// Ignore WixMessages that were not expected
        /// </summary>
        private bool ignoreExtraWixMessages;

        /// <summary>
        /// Ignore the order in which WixMessages are printed
        /// </summary>
        private bool ignoreWixMessageOrder;

        /// <summary>
        /// Constructor for a WixTool. Uses the default tool directory.
        /// </summary>
        /// <param name="toolName">The name of the tool. Eg. candle.exe</param>
        /// <param name="workingDirectory">The working directory of the tool.</param>
        public WixTool(string toolName, string workingDirectory)
            : this(Environment.ExpandEnvironmentVariables(Settings.WixToolDirectory), toolName, workingDirectory)
        {
            if (String.IsNullOrEmpty(Settings.WixToolDirectory))
            {
                throw new ArgumentException(
                    "{0} must be initialized to the WiX tools directory. Use '.' to specify the current directory.",
                    "Microsoft.Tools.WindowsInstallerXml.Test.Settings.WixToolDirectory");
            }
        }

        /// <summary>
        /// Constructor for a WixTool.
        /// </summary>
        /// <param name="toolDirectory">The directory of the tool.</param>
        /// <param name="toolName">The name of the tool. Eg. candle.exe</param>
        /// <param name="workingDirectory">The working directory of the tool.</param>
        public WixTool(string toolDirectory, string toolName, string workingDirectory)
            : base(Path.Combine(toolDirectory, toolName), workingDirectory)
        {
            this.SetBaseDefaultArguments();
            this.SetDefaultArguments();
            this.ExpectedExitCode = 0;
            this.IgnoreExtraWixMessages = false;
            this.IgnoreWixMessageOrder = false;
        }

        /// <summary>
        /// The expected WiX messages
        /// </summary>
        public List<WixMessage> ExpectedWixMessages
        {
            get { return this.expectedWixMessages; }
            set { this.expectedWixMessages = value; }
        }

        /// <summary>
        /// Ignore WixMessages that were not expected
        /// </summary>
        public bool IgnoreExtraWixMessages
        {
            get { return this.ignoreExtraWixMessages; }
            set { this.ignoreExtraWixMessages = value; }
        }

        /// <summary>
        /// Ignore the order in which WixMessages are printed
        /// </summary>
        public bool IgnoreWixMessageOrder
        {
            get { return ignoreWixMessageOrder; }
            set { ignoreWixMessageOrder = value; }
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
            if (this.IgnoreWixMessageOrder)
            {
                errors.AddRange(this.UnorderedWixMessageVerification(result.StandardOutput));
            }
            else
            {
                errors.AddRange(this.OrderedWixMessageVerification(result.StandardOutput));
            }

            return errors;
        }

        /// <summary>
        /// Clears all of the expected results and resets them to the default values
        /// </summary>
        public override void SetDefaultExpectedResults()
        {
            base.SetDefaultExpectedResults();

            this.ExpectedWixMessages = new List<WixMessage>();
        }

        /// <summary>
        /// Helper method for finding all the errors and all the warnings in the output
        /// </summary>
        /// <returns>A list of WixMessages in the output</returns>
        private List<WixMessage> FindActualWixMessages(string output)
        {
            List<WixMessage> actualWixMessages = new List<WixMessage>();

            foreach (string line in output.Split('\n', '\r'))
            {
                WixMessage actualWixMessage = WixMessage.FindWixMessage(line);

                if (null != actualWixMessage)
                {
                    actualWixMessages.Add(actualWixMessage);
                }
            }

            return actualWixMessages;
        }

        /// <summary>
        /// Perform ordered verification of the list of WixMessages
        /// </summary>
        /// <param name="output">The standard output</param>
        /// <returns>A list of errors encountered during verification</returns>
        private List<string> OrderedWixMessageVerification(string output)
        {
            List<string> errors = new List<string>();

            if (null == this.ExpectedWixMessages)
            {
                return errors;
            }

            List<WixMessage> actualWixMessages = this.FindActualWixMessages(output);

            for (int i = 0; i < this.ExpectedWixMessages.Count; i++)
            {
                // If the expectedMessage does not have any specified MessageText then ignore it in a comparison
                bool ignoreText = String.IsNullOrEmpty(this.ExpectedWixMessages[i].MessageText);

                if (i >= this.ExpectedWixMessages.Count)
                {
                    // there are more actual WixMessages than expected
                    break;
                }
                else if (i >= actualWixMessages.Count || 0 != WixMessage.Compare(actualWixMessages[i], this.ExpectedWixMessages[i], ignoreText))
                {
                    errors.Add(String.Format("Ordered WixMessage verification failed when trying to find the expected message {0}", expectedWixMessages[i]));
                    break;
                }
                else
                {
                    // the expected WixMessage was found
                    actualWixMessages[i] = null;
                }
            }

            if (!this.IgnoreExtraWixMessages)
            {
                // Now go through the messages that were found but that weren't expected
                foreach (WixMessage actualWixMessage in actualWixMessages)
                {
                    if (null != actualWixMessage)
                    {
                        errors.Add(String.Format("Found an unexpected message: {0}", actualWixMessage.ToString()));
                    }
                }
            }

            return errors;
        }

        /// <summary>
        /// Perform unordered verification of the list of WixMessages
        /// </summary>
        /// <param name="output">The standard output</param>
        /// <returns>A list of errors encountered during verification</returns>
        private List<string> UnorderedWixMessageVerification(string output)
        {
            List<string> errors = new List<string>();

            if (null == this.ExpectedWixMessages)
            {
                return errors;
            }

            List<WixMessage> actualWixMessages = this.FindActualWixMessages(output);

            for (int i = 0; i < this.ExpectedWixMessages.Count; i++)
            {
                // Flip this bool to true if the expected message is in the list of actual message that were printed
                bool expectedMessageWasFound = false;

                for (int j = 0; j < actualWixMessages.Count; j++)
                {
                    if (null != actualWixMessages[j] && this.ExpectedWixMessages[i] == actualWixMessages[j])
                    {
                        // Invalidate the message from the list of found errors by setting it to null
                        actualWixMessages[j] = null;

                        expectedMessageWasFound = true;
                        break;
                    }
                }

                // Check if the expected message was found in the list of actual messages
                if (!expectedMessageWasFound)
                {
                    errors.Add(String.Format("Could not find the expected message: {0}", this.ExpectedWixMessages[i].ToString()));

                    if (String.IsNullOrEmpty(this.ExpectedWixMessages[i].MessageText))
                    {
                        errors.Add("  When unordered WixMessage verification is performed, WixMessage text is not ignored");
                    }
                }
            }

            if (!this.IgnoreExtraWixMessages)
            {
                // Now go through the messages that were found but that weren't expected
                foreach (WixMessage actualWixMessage in actualWixMessages)
                {
                    if (null != actualWixMessage)
                    {
                        errors.Add(String.Format("Found an unexpected message: {0}", actualWixMessage.ToString()));
                    }
                }
            }

            return errors;
        }
    }
}
