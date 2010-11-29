//-------------------------------------------------------------------------------------------------
// <copyright file="Insignia.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// Build task to execute the transform generator of the Windows Installer Xml toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Build.Tasks
{
    using System;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Text;

    using Microsoft.Build.Framework;
    using Microsoft.Build.Utilities;

    /// <summary>
    /// An MSBuild task to run the WiX transform generator.
    /// </summary>
    public sealed class Insignia : WixToolTask
    {
        private const string InsigniaToolName = "insignia.exe";

        private ITaskItem[] databaseFiles;

        [Required]
        public ITaskItem[] DatabaseFiles
        {
            get { return this.databaseFiles; }
            set { this.databaseFiles = value; }
        }

        /// <summary>
        /// Get the name of the executable.
        /// </summary>
        /// <remarks>The ToolName is used with the ToolPath to get the location of Insignia.exe.</remarks>
        /// <value>The name of the executable.</value>
        protected override string ToolName
        {
            get { return InsigniaToolName; }
        }

        /// <summary>
        /// Get the path to the executable. 
        /// </summary>
        /// <remarks>GetFullPathToTool is only called when the ToolPath property is not set (see the ToolName remarks above).</remarks>
        /// <returns>The full path to the executable or simply Insignia.exe if it's expected to be in the system path.</returns>
        protected override string GenerateFullPathToTool()
        {
            // If there's not a ToolPath specified, it has to be in the system path.
            if (String.IsNullOrEmpty(this.ToolPath))
            {
                return InsigniaToolName;
            }

            return Path.Combine(Path.GetFullPath(this.ToolPath), InsigniaToolName);
        }

        /// <summary>
        /// Builds a command line from options in this task.
        /// </summary>
        protected override void BuildCommandLine(WixCommandLineBuilder commandLineBuilder)
        {
            base.BuildCommandLine(commandLineBuilder);

            commandLineBuilder.AppendFileNamesIfNotNull(this.DatabaseFiles, " ");
            commandLineBuilder.AppendTextIfNotNull(this.AdditionalOptions);
        }
    }
}
