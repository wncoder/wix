//-------------------------------------------------------------------------------------------------
// <copyright file="CommandLineOption.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Command line option used by console tools.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    /// <summary>
    /// A command line option.
    /// </summary>
    public struct CommandLineOption
    {
        public string Option;
        public string Description;
        public int AdditionalArguments;

        /// <summary>
        /// Instantiates a new BuilderCommandLineOption.
        /// </summary>
        /// <param name="option">The option name.</param>
        /// <param name="description">The description of the option.</param>
        /// <param name="additionalArguments">Count of additional arguments to require after this switch.</param>
        public CommandLineOption(string option, string description, int additionalArguments)
        {
            this.Option = option;
            this.Description = description;
            this.AdditionalArguments = additionalArguments;
        }
    }
}
