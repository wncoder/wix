//-----------------------------------------------------------------------
// <copyright file="Burn.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>A class that wraps Burn.exe</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    public partial class BurnExe : WixTool
    {
        /// <summary>
        /// Set the output location to temp.
        /// </summary>
        private bool outputToTemp;

        /// <summary>
        /// Constructor that uses the current directory as the working directory.
        /// </summary>
        public BurnExe()
            : this(null)
        {
        }

        /// <summary>
        /// Constructor that uses the default WiX tool directory as the tools location.
        /// </summary>
        /// <param name="workingDirectory">The working directory of the tool.</param>
        public BurnExe(string workingDirectory)
            : base("burn.exe", workingDirectory)
        {
            this.outputToTemp = true;
        }

        /// <summary>
        /// Constructor that accepts a path to the tools location and a working directory.
        /// </summary>
        /// <param name="toolDirectory">The directory of the tool.</param>
        /// <param name="workingDirectory">The working directory of the tool.</param>
        public BurnExe(string toolDirectory, string workingDirectory)
            : base(toolDirectory, "burn.exe", workingDirectory)
        {
            this.outputToTemp = true;
        }

        /// <summary>
        /// Specifies whether the output from Burn should be saved to a temp directory. True by default
        /// </summary>
        public bool OutputToTemp
        {
            get { return this.outputToTemp; }
            set { this.outputToTemp = value; }
        }

        public override string ToolDescription
        {
            get { return "Chainer Builder"; }
        }

    }
}
