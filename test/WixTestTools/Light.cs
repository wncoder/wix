//-----------------------------------------------------------------------
// <copyright file="Light.cs" company="Microsoft">
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
// <summary>A class that wraps Light</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.IO;

    /// <summary>
    /// A class that wraps Light
    /// </summary>
    public partial class Light : WixTool
    {
        /// <summary>
        /// Constructor that uses the current directory as the working directory
        /// </summary>
        public Light()
            : this(String.Empty)
        {
        }

        /// <summary>
        /// Constructor that uses the default WiX tool directory as the tools location
        /// </summary>
        /// <param name="workingDirectory">The working directory of the tool</param>
        public Light(string workingDirectory)
            : base("light.exe", workingDirectory)
        {
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="toolDirectory">The directory of the tool</param>
        /// <param name="workingDirectory">The working directory of the tool</param>
        public Light(string toolDirectory, string workingDirectory)
            : base(toolDirectory, "light.exe", workingDirectory)
        {
        }

        /// <summary>
        /// Constructor that uses data from a Candle object to create a Light object
        /// </summary>
        /// <param name="candle">A Candle object</param>
        public Light(Candle candle)
            : this(candle, false)
        {
        }

        /// <summary>
        /// Constructor that uses data from a Candle object to create a Light object
        /// </summary>
        /// <param name="candle">A Candle object</param>
        /// <param name="xmlOutput">False if Light should build an MSI. True if Light should build a wixout.</param>
        public Light(Candle candle, bool xmlOutput)
            : this(Path.GetDirectoryName(candle.ToolFile), candle.WorkingDirectory)
        {
            // The output of Candle is the input for Light
            this.ObjectFiles = candle.ExpectedOutputFiles;

            string outputFileExtension = ".msi";
            if (xmlOutput)
            {
                this.XmlOutput = true;
                outputFileExtension = ".wixout";
            }

            // If Candle has only one input file, use that file name for Light's output file
            if (null != candle.ExpectedOutputFiles && 1 == candle.ExpectedOutputFiles.Count)
            {
                string outputFileName = String.Concat(Path.GetFileNameWithoutExtension(candle.ExpectedOutputFiles[0]), outputFileExtension);
                this.OutputFile = Path.Combine(candle.OutputDirectory, outputFileName);
            }
            else
            {
                string outputFileName = String.Concat("product", outputFileExtension);
                this.OutputFile = Path.Combine(candle.OutputDirectory, outputFileName);
            }
        }

        /// <summary>
        /// Constructor that uses data from a Lit object to create a Light object
        /// </summary>
        /// <param name="candle">A Lit object</param>
        public Light(Lit lit)
            : this(Path.GetDirectoryName(lit.ToolFile), lit.WorkingDirectory)
        {
            // The output of Lit is the input for Light
            this.ObjectFiles.Add(lit.ExpectedOutputFile);
            string outputFileName = String.Concat(Path.GetFileNameWithoutExtension(lit.ExpectedOutputFile), ".msi");
            this.OutputFile = Path.Combine(Path.GetDirectoryName(lit.ExpectedOutputFile), outputFileName);
        }

        /// <summary>
        /// Functional name of the tool
        /// </summary>
        public override string ToolDescription
        {
            get { return "Linker"; }
        }
    }
}
