//-----------------------------------------------------------------------
// <copyright file="BurnArguments.cs" company="Microsoft">
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
