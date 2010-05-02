//-----------------------------------------------------------------------
// <copyright file="Dark.cs" company="Microsoft">
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
// <summary>A class that wraps Dark</summary>
//-----------------------------------------------------------------------
namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.Collections.Generic;
    using System.Text;
    using System.IO;
    using System.Xml;

    /// <summary>
    /// Dark tool class.
    /// </summary>
    public partial class Dark : WixTool
    {
        #region Fields

        /// <summary>
        /// Set the output location to temp.
        /// </summary>
        private bool outputToTemp;
             
        #endregion

        #region Public Properties

        /// <summary>
        /// Gets the expected output file.
        /// </summary>
        public string ExpectedOutputFile
        {
            get
            { 
                // If the output file is not provided then the expected output file will have
                // the same name as input file with wxs extension.
                if (string.IsNullOrEmpty(this.OutputFile))
                {
                    return Path.GetFileNameWithoutExtension(this.InputFile) + ".wxs";
                }
                else
                {
                   return this.OutputFile;
                }  
            }
        }

        /// <summary>
        /// Functional name of the tool
        /// </summary>
        public override string ToolDescription
        {
            get { return "Decompiler"; }
        }

        #endregion

        #region Constructors
        /// <summary>
        /// Constructor that uses the current directory as the working directory.
        /// </summary>
        public Dark() : this(null)
        {
        }

        /// <summary>
        /// Constructor that uses the default WiX tool directory as the tools location.
        /// </summary>
        /// <param name="workingDirectory">The working directory of the tool.</param>
        public Dark(string workingDirectory)
            : base("dark.exe", workingDirectory)
        {
            this.OutputToTemp = true;
        }

        /// <summary>
        /// The constructor.
        /// </summary>
        /// <param name="toolDirectory">The directory of the tool.</param>
        /// <param name="workingDirectory">The working directory of the tool.</param>
        public Dark(string toolDirectory, string workingDirectory)
            : base(toolDirectory, "dark.exe", workingDirectory)
        {
            this.OutputToTemp = true;
        }
        #endregion

        #region Properties

        /// <summary>
        /// Gets or sets a value indicating whether to output to temp dir or not.
        /// </summary>
        public bool OutputToTemp
        {
            get { return this.outputToTemp; }
            set { this.outputToTemp = value; }
        }

        #endregion

        #region Methods

        /// <summary>
        /// Run the tool.
        /// </summary>
        /// <param name="exceptionOnError">Bool to indicate whether to produce exception
        /// in case of error or not.</param>
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
            if (this.OutputToTemp && !string.IsNullOrEmpty(this.OutputFile) && !Path.IsPathRooted(this.OutputFile))
            {
                string tempDirectory = Path.Combine(Path.GetTempPath(),Path.GetRandomFileName());
                
                // To be removed when -out switch is added to Dark
                // Remove from here
                try
                {
                    Directory.CreateDirectory(tempDirectory);
                }
                catch (Exception ex)
                {
                    string s = ex.Message;
                }
                // to here
                this.OutputFile = String.Concat(tempDirectory, @"\", this.OutputFile);
            }
        }

        /// <summary>
        /// Create a new, unique subdirectory under the specified path.
        /// </summary>
        /// <param name="path"> The path to the parent of the new directory.</param>
        /// <returns> The full path to the new directory.</returns>
        public string CreateUniqueDirectory(string path)
        {
            string directoryPath = Path.Combine(path, Path.GetRandomFileName());

            // Keep looping till it finds a random directory name that does not exist
            while (Directory.Exists(directoryPath))
            {
                directoryPath = Path.Combine(path, Path.GetRandomFileName());
            }

            Directory.CreateDirectory(directoryPath);

            return directoryPath;
        }
        #endregion
    }
}
