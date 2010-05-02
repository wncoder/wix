//-----------------------------------------------------------------------
// <copyright file="BurnArguments.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Fields, properties and methods for working with Burn arguments
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.Collections.Generic;
    using System.Text;

    /// <summary>
    /// Fields, properties and methods for working with Burn arguments.
    /// </summary>
    public partial class BurnExe
    {
        #region Private Members
        #endregion

        #region Public Properties

        #endregion


        /// <summary>
        /// Clears all of the assigned arguments and resets them to the default values.
        /// </summary>
        public override void SetDefaultArguments()
        {            
        }

        /// <summary>
        /// The default file extension of an output file
        /// </summary>
        protected override string OutputFileExtension
        {
            get { return string.Empty;}
        }
    }
}
