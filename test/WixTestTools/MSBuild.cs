//-----------------------------------------------------------------------
// <copyright file="MSBuild.cs" company="Microsoft">
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
// <summary>A class that wraps MSBuild</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// A class that wraps MSBuild.
    /// </summary>
    public partial class MSBuild : TestTool
    {
        /// <summary>
        /// The MSBuild root output directory
        /// </summary>
        private string outputRootDirectory = String.Empty;

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
    }
}
