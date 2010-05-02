//-----------------------------------------------------------------------
// <copyright file="CandleArguments.cs" company="Microsoft">
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
// <summary>
//     Fields, properties and methods for working with Candle arguments
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.Collections.Generic;
    using System.Text;

    /// <summary>
    /// Fields, properties and methods for working with Candle arguments.
    /// </summary>
    public partial class Candle
    {
        #region Private Members

        /// <summary>
        /// -ext
        /// </summary>
        private List<string> extensions;

        /// <summary>
        /// -zs
        /// </summary>
        private bool onlyValidateDocuments;

        /// <summary>
        /// -out outputFile
        /// </summary>
        private string outputFile;

        /// <summary>
        /// -pedantic
        /// </summary>
        private bool pedantic;

        /// <summary>
        /// <![CDATA[-p<file>]]>
        /// </summary>
        private string preProcessFile;

        /// <summary>
        /// <![CDATA[-d<name>=<value>]]>
        /// </summary>
        private Dictionary<string, string> preProcessorParams;

        /// <summary>
        ///  <![CDATA[-I<dir>]]>
        /// </summary>
        private List<string> includeSearchPaths;

        /// <summary>
        /// sourceFile [sourceFile ...]
        /// </summary>
        private List<string> sourceFiles;

        /// <summary>
        ///  Suppress all warnings
        /// </summary>
        private bool suppressAllWarnings;

        /// <summary>
        /// -ss
        /// </summary>
        private bool suppressSchemaValidation;

        /// <summary>
        ///  -sw[N]
        /// </summary>
        private List<int> suppressWarnings;

        /// <summary>
        /// -trace
        /// </summary>
        private bool trace;

        /// <summary>
        /// -wx[N]
        /// </summary>
        private List<int> treatWarningsAsErrors;

        /// <summary>
        /// Treat all warnings as errors
        /// </summary>
        private bool treatAllWarningsAsErrors;

        /// <summary>
        /// -v
        /// </summary>
        private bool verboseOutput;

        #endregion

        #region Public Properties

        /// <summary>
        /// The arguments as they would be passed on the command line.
        /// </summary>
        /// <remarks>
        /// To allow for negative testing, checking for invalid combinations
        /// of arguments is not performed.
        /// </remarks>
        public override string Arguments
        {
            get
            {
                StringBuilder arguments = new StringBuilder(base.Arguments);

                // Extensions
                foreach (string extension in this.Extensions)
                {
                    arguments.AppendFormat(@" -ext ""{0}""", extension);
                }

                // OnlyValidateDocuments
                if (this.OnlyValidateDocuments)
                {
                    arguments.Append(" -zs");
                }

                // OutputFile
                if (!String.IsNullOrEmpty(this.OutputFile))
                {
                    // WiX requires that we add extra backslashes to the end of a directory path
                    if (this.OutputFile.EndsWith(@"\") && !this.OutputFile.EndsWith(@"\\"))
                    {
                        arguments.AppendFormat(@" -out ""{0}\""", this.OutputFile);
                    }
                    else
                    {
                        arguments.AppendFormat(@" -out ""{0}""", this.OutputFile);
                    }
                }

                // Pedantic
                if (this.Pedantic)
                {
                    arguments.Append(" -pedantic");
                }

                // PreProcessFile
                if (!String.IsNullOrEmpty(this.PreProcessFile))
                {
                    arguments.AppendFormat(@" -p""{0}""", this.PreProcessFile);
                }

                // PreProcessorParams
                foreach (string key in this.PreProcessorParams.Keys)
                {
                    arguments.AppendFormat(" -d{0}={1}", key, this.PreProcessorParams[key]);
                }

                // IncludeSearchPaths
                foreach (string searchPath in this.IncludeSearchPaths)
                {
                    arguments.AppendFormat(@" -I""{0}""", searchPath);
                }

                // SourceFiles
                foreach (string sourceFile in this.SourceFiles)
                {
                    arguments.AppendFormat(@" ""{0}""", sourceFile);
                }

                // SuppressAllWarnings
                if (this.SuppressAllWarnings)
                {
                    arguments.Append(" -sw");
                }

                // SuppressSchemaValidation
                if (true == this.suppressSchemaValidation)
                {
                    arguments.Append(" -ss");
                }

                // SuppressWarnings
                foreach (int warning in this.SuppressWarnings)
                {
                    arguments.AppendFormat(" -sw{0}", warning.ToString());
                }

                // Trace
                if (this.Trace)
                {
                    arguments.Append(" -trace");
                }

                // TreatAllWarningsAsErrors
                if (this.TreatAllWarningsAsErrors)
                {
                    arguments.Append(" -wx");
                }

                // Treat specific warnings as errors
                foreach (int warning in this.TreatWarningsAsErrors)
                {
                    arguments.AppendFormat(" -wx{0}", warning.ToString());
                }
                
                // VerboseOutput
                if (this.VerboseOutput)
                {
                    arguments.Append(" -v");
                }

                return arguments.ToString();
            }
        }

        /// <summary>
        /// -ext
        /// </summary>
        public List<string> Extensions
        {
            get { return this.extensions; }
            set { this.extensions = value; }
        }

        /// <summary>
        /// -zs
        /// </summary>
        public bool OnlyValidateDocuments
        {
            get { return this.onlyValidateDocuments; }
            set { this.onlyValidateDocuments = value; }
        }

        /// <summary>
        /// -out outputFile
        /// </summary>
        public string OutputFile
        {
            get { return this.outputFile; }
            set { this.outputFile = value; }
        }

        /// <summary>
        /// -pedantic
        /// </summary>
        public bool Pedantic
        {
            get { return this.pedantic; }
            set { this.pedantic = value; }
        }

        /// <summary>
        /// <![CDATA[-p<file>]]>
        /// </summary>
        public string PreProcessFile
        {
            get { return this.preProcessFile; }
            set { this.preProcessFile = value; }
        }

        /// <summary>
        /// <![CDATA[-d<name>=<value>]]>  
        /// </summary>
        public Dictionary<string, string> PreProcessorParams
        {
            get { return this.preProcessorParams; }
            set { this.preProcessorParams = value; }
        }

        /// <summary>
        /// <![CDATA[-I<dir>]]> 
        /// </summary>
        public List<string> IncludeSearchPaths
        {
            get { return this.includeSearchPaths; }
            set { this.includeSearchPaths = value; }
        }

        /// <summary>
        /// sourceFile [sourceFile ...]
        /// </summary>
        public List<string> SourceFiles
        {
            get { return this.sourceFiles; }
            set { this.sourceFiles = value; }
        }

        /// <summary>
        ///  Suppress all warnings.
        /// </summary>
        public bool SuppressAllWarnings
        {
            get { return this.suppressAllWarnings; }
            set { this.suppressAllWarnings = value; }
        }

        /// <summary>
        /// -ss
        /// </summary>
        public bool SuppressSchemaValidation
        {
            get { return this.suppressSchemaValidation; }
            set { this.suppressSchemaValidation = value; }
        }

        /// <summary>
        /// -sw[N]
        /// </summary>
        public List<int> SuppressWarnings
        {
            get { return this.suppressWarnings; }
            set { this.suppressWarnings = value; }
        }

        /// <summary>
        /// -trace
        /// </summary>
        public bool Trace
        {
            get { return this.trace; }
            set { this.trace = value; }
        }

        /// <summary>
        /// Treat all warnings as errors.
        /// </summary>
        public bool TreatAllWarningsAsErrors
        {
            get { return this.treatAllWarningsAsErrors; }
            set { this.treatAllWarningsAsErrors = value; }
        }

        /// <summary>
        /// -wx[N]
        /// </summary>
        public List<int> TreatWarningsAsErrors
        {
            get { return this.treatWarningsAsErrors; }
            set { this.treatWarningsAsErrors = value; }
        }

        /// <summary>
        /// -v
        /// </summary>
        public bool VerboseOutput
        {
            get { return this.verboseOutput; }
            set { this.verboseOutput = value; }
        }

        #endregion

        /// <summary>
        /// Clears all of the assigned arguments and resets them to the default values.
        /// </summary>
        public override void SetDefaultArguments()
        {
            this.Extensions = new List<string>();
            this.OnlyValidateDocuments = false;
            this.OutputFile = String.Empty;
            this.Pedantic = false;
            this.PreProcessFile = String.Empty;
            this.PreProcessorParams = new Dictionary<string, string>();
            this.IncludeSearchPaths = new List<string>();
            this.SourceFiles = new List<string>();
            this.SuppressAllWarnings = false;
            this.SuppressSchemaValidation = false;
            this.SuppressWarnings = new List<int>();
            this.Trace = false;
            this.TreatAllWarningsAsErrors = false;
            this.TreatWarningsAsErrors = new List<int>();
            this.VerboseOutput = false;
        }
    }
}
