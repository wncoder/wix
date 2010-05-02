//-------------------------------------------------------------------------------------------------
// <copyright file="Lit.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// Build task to execute the lib tool of the Windows Installer Xml toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Build.Tasks
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Text;

    using Microsoft.Build.Framework;
    using Microsoft.Build.Utilities;

    /// <summary>
    /// An MSBuild task to run the WiX lib tool.
    /// </summary>
    public sealed class Lit : WixToolTask
    {
        private const string LitToolName = "lit.exe";

        private string[] baseInputPaths;
        private bool bindFiles;
        private ITaskItem[] extensions;
        private ITaskItem[] localizationFiles;
        private ITaskItem[] objectFiles;
        private ITaskItem outputFile;
        private bool pedantic;
        private bool suppressIntermediateFileVersionMatching;
        private bool suppressSchemaValidation;
        private string extensionDirectory;
        private string[] referencePaths;

        public string[] BaseInputPaths
        {
            get { return this.baseInputPaths; }
            set { this.baseInputPaths = value; }
        }

        public bool BindFiles
        {
            get { return this.bindFiles; }
            set { this.bindFiles = value; }
        }

        public ITaskItem[] Extensions
        {
            get { return this.extensions; }
            set { this.extensions = value; }
        }

        public ITaskItem[] LocalizationFiles
        {
            get { return this.localizationFiles; }
            set { this.localizationFiles = value; }
        }

        [Required]
        public ITaskItem[] ObjectFiles
        {
            get { return this.objectFiles; }
            set { this.objectFiles = value; }
        }

        [Required]
        [Output]
        public ITaskItem OutputFile
        {
            get { return this.outputFile; }
            set { this.outputFile = value; }
        }

        public bool Pedantic
        {
            get { return this.pedantic; }
            set { this.pedantic = value; }
        }

        public bool SuppressIntermediateFileVersionMatching
        {
            get { return this.suppressIntermediateFileVersionMatching; }
            set { this.suppressIntermediateFileVersionMatching = value; }
        }

        public bool SuppressSchemaValidation
        {
            get { return this.suppressSchemaValidation; }
            set { this.suppressSchemaValidation = value; }
        }

        public string ExtensionDirectory
        {
            get { return this.extensionDirectory; }
            set { this.extensionDirectory = value; }
        }

        public string[] ReferencePaths
        {
            get { return this.referencePaths; }
            set { this.referencePaths = value; }
        }

        /// <summary>
        /// Get the name of the executable.
        /// </summary>
        /// <remarks>The ToolName is used with the ToolPath to get the location of lit.exe</remarks>
        /// <value>The name of the executable.</value>
        protected override string ToolName
        {
            get { return LitToolName; }
        }

        /// <summary>
        /// Get the path to the executable. 
        /// </summary>
        /// <remarks>GetFullPathToTool is only called when the ToolPath property is not set (see the ToolName remarks above).</remarks>
        /// <returns>The full path to the executable or simply lit.exe if it's expected to be in the system path.</returns>
        protected override string GenerateFullPathToTool()
        {
            // If there's not a ToolPath specified, it has to be in the system path.
            if (String.IsNullOrEmpty(this.ToolPath))
            {
                return LitToolName;
            }

            return Path.Combine(Path.GetFullPath(this.ToolPath), LitToolName);
        }

        /// <summary>
        /// Builds a command line from options in this task.
        /// </summary>
        protected override void BuildCommandLine(WixCommandLineBuilder commandLineBuilder)
        {
            base.BuildCommandLine(commandLineBuilder);

            commandLineBuilder.AppendSwitchIfNotNull("-out ", this.OutputFile);
            commandLineBuilder.AppendArrayIfNotNull("-b ", this.BaseInputPaths);
            commandLineBuilder.AppendIfTrue("-bf", this.BindFiles);
            commandLineBuilder.AppendExtensions(this.extensions, this.ExtensionDirectory, this.referencePaths);
            commandLineBuilder.AppendArrayIfNotNull("-loc ", this.LocalizationFiles);
            commandLineBuilder.AppendIfTrue("-pedantic", this.Pedantic);
            commandLineBuilder.AppendIfTrue("-ss", this.SuppressSchemaValidation);
            commandLineBuilder.AppendIfTrue("-sv", this.SuppressIntermediateFileVersionMatching);
            commandLineBuilder.AppendTextIfNotNull(this.AdditionalOptions);

            List<string> objectFilePaths = AdjustFilePaths(this.objectFiles, this.ReferencePaths);
            commandLineBuilder.AppendFileNamesIfNotNull(objectFilePaths.ToArray(), " ");
        }
    }
}
