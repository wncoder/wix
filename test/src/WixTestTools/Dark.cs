//-----------------------------------------------------------------------
// <copyright file="Dark.cs" company="Microsoft">
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
// <summary>A class that wraps Dark</summary>
//-----------------------------------------------------------------------
namespace WixTest
{
    using System;
    using System.Collections.Generic;
    using System.Text;
    using System.IO;
    using System.Xml;
    using WixTest.Utilities;

    /// <summary>
    /// Dark tool class.
    /// </summary>
    public partial class Dark : WixTool
    {
        /// <summary>
        /// Constructor that uses the current directory as the working directory.
        /// </summary>
        public Dark()
            : this(null)
        {
        }

        /// <summary>
        /// Constructor that uses the default WiX tool directory as the tools location.
        /// </summary>
        /// <param name="workingDirectory">The working directory of the tool.</param>
        public Dark(string workingDirectory)
            : base("dark.exe", workingDirectory)
        {
        }

        /// <summary>
        /// The default file extension of an output file
        /// </summary>
        protected override string OutputFileExtension
        {
            get { return ".wxs"; }
        }

        /// <summary>
        /// Functional name of the tool
        /// </summary>
        public override string ToolDescription
        {
            get { return "Decompiler"; }
        }

        /// <summary>
        /// Sets the OutputFile to a default value if it is not set 
        /// </summary>
        protected override void SetDefaultOutputFile()
        {
            if (String.IsNullOrEmpty(this.OutputFile))
            {
                string outputFileName;
                string outputDirectoryName = FileUtilities.GetUniqueFileName();

                if (!string.IsNullOrEmpty(this.InputFile))
                {
                    outputFileName = String.Concat(Path.GetFileNameWithoutExtension(this.InputFile), this.OutputFileExtension);
                    this.OutputFile = Path.Combine(outputDirectoryName, outputFileName);
                }
            }
        }
    }
}
