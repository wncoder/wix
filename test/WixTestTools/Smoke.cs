//-----------------------------------------------------------------------
// <copyright file="Smoke.cs" company="Microsoft">
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
        /// <param name="workingDirectory">The directory from where the tool will be run</param>
        public Smoke(string workingDirectory)
            : base("smoke.exe", workingDirectory)
        {
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="toolDirectory">The directory of the tool</param>
        /// <param name="workingDirectory">The working directory of the tool</param>
        public Smoke(string toolDirectory, string workingDirectory)
            : base(toolDirectory, "smoke.exe", workingDirectory)
        {
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
