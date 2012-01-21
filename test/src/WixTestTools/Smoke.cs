//-----------------------------------------------------------------------
// <copyright file="Smoke.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Wraps the WiX Smoke tool</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    /// <summary>
    /// Wraps the WiX Smoke tool
    /// </summary>
    public partial class Smoke : WixTool
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public Smoke()
            : this(null)
        {
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="workingDirectory">The working directory of the tool</param>
        public Smoke(string workingDirectory)
            : base("smoke.exe", workingDirectory)
        {
        }

        /// <summary>
        /// The default file extension of an output file
        /// </summary>
        protected override string OutputFileExtension
        {
            get { return string.Empty; }
        }

        /// <summary>
        /// Functional name of the tool
        /// </summary>
        public override string ToolDescription
        {
            get { return "Validator"; }
        }
    }
}
