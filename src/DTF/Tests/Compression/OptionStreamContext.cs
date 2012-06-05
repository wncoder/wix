//-------------------------------------------------------------------------------------------------
// <copyright file="OptionStreamContext.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using Microsoft.Deployment.Compression;

namespace Microsoft.Deployment.Test
{
    public class OptionStreamContext : ArchiveFileStreamContext
    {
        private PackOptionHandler packOptionHandler;

        public OptionStreamContext(IList<string> archiveFiles, string directory, IDictionary<string, string> files)
            : base(archiveFiles, directory, files)
        {
        }

        public delegate object PackOptionHandler(string optionName, object[] parameters);

        public PackOptionHandler OptionHandler
        {
            get
            {
                return this.packOptionHandler;
            }
            set
            {
                this.packOptionHandler = value;
            }
        }

        public override object GetOption(string optionName, object[] parameters)
        {
            if (this.OptionHandler == null)
            {
                return null;
            }

            return this.OptionHandler(optionName, parameters);
        }
    }
}
