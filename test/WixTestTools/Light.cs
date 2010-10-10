//-----------------------------------------------------------------------
// <copyright file="Light.cs" company="Microsoft">
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
        /// Constructor that uses the default WiX tool directory as the tools location
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
            : this()
        {
            this.XmlOutput = xmlOutput;

            // The output of Candle is the input for Light
            this.ObjectFiles = candle.ExpectedOutputFiles;
        }

        /// <summary>
        /// Constructor that uses data from a Lit object to create a Light object
        /// </summary>
        /// <param name="candle">A Lit object</param>
        public Light(Lit lit)
            : this()
        {
            // The output of Lit is the input for Light
            this.ObjectFiles.Add(lit.ExpectedOutputFile);
            string outputFileName = String.Concat(Path.GetFileNameWithoutExtension(lit.ExpectedOutputFile), this.OutputFileExtension);
            this.OutputFile = Path.Combine(Path.GetDirectoryName(lit.ExpectedOutputFile), outputFileName);
        }

        /// <summary>
        /// The default file extension of an output file based on the value of the XMLOutput property
        /// </summary>
        protected override string OutputFileExtension
        {
            get
            {
                string outputFileExtension = ".msi";

                if (this.XmlOutput)
                {
                    outputFileExtension = ".wixout";
                }

                return outputFileExtension;
            }
        }

        /// <summary>
        /// Functional name of the tool
        /// </summary>
        public override string ToolDescription
        {
            get { return "Linker"; }
        }

        /// <summary>
        /// Sets the OutputFile to a default value if it is not set 
        /// </summary>
        protected override void SetDefaultOutputFile()
        {
            if (String.IsNullOrEmpty(this.OutputFile))
            {
                string outputFileName;
                string outputDirectoryName = Builder.GetUniqueFileName();

                if (null != this.ObjectFiles && this.ObjectFiles.Count == 1)
                {
                    outputFileName = String.Concat(Path.GetFileNameWithoutExtension(this.ObjectFiles[0]), this.OutputFileExtension);
                }
                else
                {
                    outputFileName = String.Concat("test", this.OutputFileExtension);
                }

                this.OutputFile = Path.Combine(outputDirectoryName, outputFileName);
            }
        }
    }
}
