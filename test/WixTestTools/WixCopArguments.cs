﻿//-----------------------------------------------------------------------
// <copyright file="WixCopArguments.cs" company="Microsoft">
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
//     Fields, properties and methods for working with WixCop arguments
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.Collections.Generic;
    using System.Collections.Specialized;
    using System.Text;

    /// <summary>
    /// Fields, properties and methods for working with WixCop arguments
    /// </summary>
    public partial class WixCop
    {
        #region Private Members

        /// <summary>
        /// -f
        /// </summary>
        private bool fixErrors;

        /// <summary>
        //  -indent:<n>
        /// </summary>
        private int? indentation;

        /// <summary>
        /// -s
        /// </summary>
        private bool searchInCurrentDirectory;

        /// <summary>
        /// -set1
        /// </summary>
        private string primarySettingsFile;

        /// <summary>
        /// -set2
        /// </summary>
        private string secondarySettingsFile;

        /// <summary>
        /// SourceFiles
        /// </summary>
        private List<string> sourceFiles;

        #endregion

        #region Public Properties

        /// <summary>
        /// The arguments as they would be passed on the command line
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

                // FixErrors
                if (this.FixErrors)
                {
                    arguments.Append(" -f");
                }

                // Indentation
                if (null != this.Indentation)
                {
                    arguments.AppendFormat(" -indent:{0}", this.Indentation);
                }

                // SearchInCurrentDirectory
                if (this.SearchInCurrentDirectory)
                {
                    arguments.Append(" -s");
                }

                // PrimarySettingsFile
                if (!String.IsNullOrEmpty(this.PrimarySettingsFile))
                {
                    arguments.AppendFormat(@" -set1""{0}""", this.PrimarySettingsFile);
                }

                // SecondarySettingsFile
                if (!String.IsNullOrEmpty(this.SecondarySettingsFile))
                {
                    arguments.AppendFormat(@" -set2""{0}""", this.SecondarySettingsFile);
                }

                // Source Files
                foreach (string sourceFile in this.SourceFiles)
                {
                    arguments.AppendFormat(@" ""{0}""", sourceFile);
                }

                return arguments.ToString();
            }
        }

        /// <summary>
        /// -f
        /// </summary>
        public bool FixErrors
        {
            get { return this.fixErrors; }
            set { this.fixErrors = value; }
        }

        /// <summary>
        //  -indent:<n>
        /// </summary>
        public int? Indentation
        {
            get { return this.indentation; }
            set { this.indentation = value; }
        }

        /// <summary>
        /// -s
        /// </summary>
        public bool SearchInCurrentDirectory
        {
            get { return this.searchInCurrentDirectory; }
            set { this.searchInCurrentDirectory = value; }
        }

        /// <summary>
        /// -set1
        /// </summary>
        public string PrimarySettingsFile
        {
            get { return this.primarySettingsFile; }
            set { this.primarySettingsFile = value; }
        }

        /// <summary>
        /// -set2
        /// </summary>
        public string SecondarySettingsFile
        {
            get { return this.secondarySettingsFile; }
            set { this.secondarySettingsFile = value; }
        }

        /// <summary>
        /// SourceFiles
        /// </summary>
        public List<string> SourceFiles
        {
            get { return this.sourceFiles; }
            set { this.sourceFiles = value; }
        }


        #endregion

        /// <summary>
        /// Clears all of the assigned arguments and resets them to the default values
        /// </summary>
        public override void SetDefaultArguments()
        {
            this.FixErrors = false;
            this.Indentation = 4;
            this.SearchInCurrentDirectory = false;
            this.PrimarySettingsFile = string.Empty;
            this.SecondarySettingsFile = string.Empty;
            this.SourceFiles = new List<string>();            
        }
    }
}
