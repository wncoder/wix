//-----------------------------------------------------------------------
// <copyright file="WixprojMSBuild.cs" company="Microsoft">
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
// <summary>A class that uses MSBuild to build a .wixproj</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text.RegularExpressions;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// A class that uses MSBuild to build a .wixproj
    /// </summary>
    public partial class WixprojMSBuild : MSBuild
    {
        /// <summary>
        /// Constructor that uses the default location for MSBuild.
        /// </summary>
        public WixprojMSBuild()
            : base()
        {
            this.Initialize();
        }

        /// <summary>
        /// Constructor that uses the default location for MSBuild.
        /// </summary>
        /// <param name="outputRootDirectory">The MSBuild output location</param>
        public WixprojMSBuild(string outputRootDirectory)
            : this()
        {
            this.OutputRootDirectory = outputRootDirectory;
        }

        /// <summary>
        /// Constructor that accepts a path to the MSBuild location.
        /// </summary>
        /// <param name="toolDirectory">The directory of MSBuild.exe.</param>
        /// <param name="outputRootDirectory">The MSBuild output location</param>
        public WixprojMSBuild(string toolDirectory, string outputRootDirectory)
            : base(toolDirectory)
        {
            this.Initialize();
            this.OutputRootDirectory = outputRootDirectory;
        }

        /// <summary>
        /// Asserts that a task's output contains the specified substring
        /// </summary>
        /// <param name="task">The task to verify</param>
        /// <param name="substring">The substring to search for</param>
        public void AssertTaskSubstring(string task, string substring)
        {
            this.AssertTaskSubstring(task, substring, false);
        }

        /// <summary>
        /// Asserts that a task's output contains the specified substring
        /// </summary>
        /// <param name="task">The task to verify</param>
        /// <param name="substring">The substring to search for</param>
        /// <param name="ignoreCase">A Boolean indicating a case-sensitive or insensitive assertion. (true indicates a case-insensitive assertion.)</param>
        public void AssertTaskSubstring(string task, string substring, bool ignoreCase)
        {
            bool containsSubstring;
            Match match = this.SearchTask(task);
            string taskOutput = match.Groups["taskOutput"].Value;

            if (ignoreCase)
            {
                containsSubstring = taskOutput.ToLower().Contains(substring.ToLower());

            }
            else
            {
                containsSubstring = taskOutput.Contains(substring);
            }

            // Assert that the substring is contained in the task output
            Assert.IsTrue(containsSubstring, "The substring '{0}' is not contained in the {1} task's output '{2}'", substring, task, taskOutput); 
        }

        /// <summary>
        /// Asserts that a task's output doesn't contain the specified substring
        /// </summary>
        /// <param name="task">The task to verify</param>
        /// <param name="substring">The substring to search for</param>
        /// <param name="ignoreCase">A Boolean indicating a case-sensitive or insensitive assertion. (true indicates a case-insensitive assertion.)</param>
        public void AssertNotExistsTaskSubstring(string task, string substring, bool ignoreCase)
        {
            bool containsSubstring;
            Match match = this.SearchTask(task);
            string taskOutput = match.Groups["taskOutput"].Value;
            
            if (ignoreCase)
            {
                containsSubstring = taskOutput.ToLower().Contains(substring.ToLower());
                
            }
            else
            {
                containsSubstring = taskOutput.Contains(substring);
            }
            
            // Assert that the substring is contained in the task output
            Assert.IsFalse(containsSubstring, "The substring '{0}' is contained in the {1} task's output '{2}'", substring, task, taskOutput);
        }

        /// <summary>
        /// Asserts that the WixprojMSBuild's output contains the task specified
        /// </summary>
        /// <param name="task">The task to search for</param>
        public void AssertTaskExists(string task)
        {
            Match match = this.SearchTask(task);
            string taskOutput = match.Groups["taskOutput"].Value;


            // Assert that the task is contained in the build output
            Assert.IsTrue(match.Success, "The task '{0}' is not contained in the WixprojMSBuild's output '{1}'", task, this.Result.StandardOutput);
        }

        /// <summary>
        /// Searches for a task in the build output
        /// </summary>
        /// <returns>Match object holding representing the result from searching for task</returns>
        private Match SearchTask(string task)
        {
            Regex regex = new Regex(String.Format("Task \"{0}\"\\s*Command:(?<taskOutput>.*)Done executing task \"{0}\"\\.", task), RegexOptions.Singleline);
            return regex.Match(this.Result.StandardOutput);
        }

        /// <summary>
        /// Perform some initialization for this class
        /// </summary>
        private void Initialize()
        {
            this.ExpectedExitCode = 0;
            this.Properties.Add("DefineSolutionProperties", "false");
            this.Properties.Add("WixToolPath", Settings.WixToolDirectory);

            if (!String.IsNullOrEmpty(Settings.WixTargetsPath))
            {
                this.Properties.Add("WixTargetsPath", Settings.WixTargetsPath);
            }

            if (!String.IsNullOrEmpty(Settings.WixTasksPath))
            {
                this.Properties.Add("WixTasksPath", Settings.WixTasksPath);
                // this.Properties.Add("WixTasksPath", Path.Combine(Settings.WixToolDirectory, "WixTasks.dll"));
            }
        }
    }
}
